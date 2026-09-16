#include "pulsar/io_context.hpp"
#include <cassert>
#include <cstdio>
#include <pthread.h>
#include <thread>

namespace pulsar
{
namespace
{
thread_local IoContext *tCurrentContext = nullptr;
}

IoContext::IoContext(unsigned queueDepth)
{
    mBackend.init(queueDepth);
    assert(tCurrentContext == nullptr && "на потоке уже есть IoContext");
    tCurrentContext = this;
}

IoContext::~IoContext()
{
    mBackend.shutdown();
    mBackgroundTasks.clear();

    if (tCurrentContext == this)
    {
        tCurrentContext = nullptr;
    }
}

IoContext &IoContext::current()
{
    assert(tCurrentContext != nullptr && "нет активного IoContext на этом потоке");
    return *tCurrentContext;
}

void IoContext::submit(const IoRequest &request, IoCompletion *completion)
{
    mBackend.submitRequest(request, completion);
}

void IoContext::spawn(Task<void> task)
{
    mBackgroundTasks.push_back(std::move(task));
    mBackgroundTasks.back().start();
}

void IoContext::reapFinishedTasks()
{
    std::erase_if(mBackgroundTasks, [this](const Task<void> &t) {
        if (!t.done())
        {
            return false;
        }
        // detached-задача: co_await её никто не делает, значит
        // исключение из неё иначе просто исчезнет без следа
        if (auto e = t.exception())
        {
            reportTaskException(e);
        }
        return true;
    });
}

void IoContext::reportTaskException(std::exception_ptr e) const noexcept
{
    if (mExceptionHandler)
    {
        mExceptionHandler(e);
        return;
    }
    try
    {
        std::rethrow_exception(e);
    }
    catch (const std::exception &ex)
    {
        std::fprintf(stderr, "[pulsar] необработанное исключение в spawn()-задаче: %s\n", ex.what());
    }
    catch (...)
    {
        std::fprintf(stderr, "[pulsar] необработанное не-std исключение в spawn()-задаче\n");
    }
}

void IoContext::setExceptionHandler(std::function<void(std::exception_ptr)> handler)
{
    mExceptionHandler = std::move(handler);
}

void IoContext::stop() noexcept
{
    mStopped = true;
}

void IoContext::restart() noexcept
{
    mStopped = false;
}

void IoContext::run()
{
    reapFinishedTasks();

    while (!mStopped && !mBackgroundTasks.empty())
    {
        mBackend.waitAndProcessCompletions();
        reapFinishedTasks();
    }
}

void runOnEachCore(unsigned coreCount, const std::function<Task<void>(unsigned)> &main)
{
    std::vector<std::thread> threads;
    threads.reserve(coreCount);

    for (unsigned i = 0; i < coreCount; ++i)
    {
        threads.emplace_back([i, &main] {
            IoContext ctx;

            ::cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(i, &cpuset);
            ::pthread_setaffinity_np(::pthread_self(), sizeof(cpuset), &cpuset);

            ctx.spawn(main(i));
            ctx.run();
        });
    }

    for (auto &t : threads)
    {
        t.join();
    }
}
} // namespace pulsar
