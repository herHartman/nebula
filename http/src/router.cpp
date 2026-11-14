#include "nebula/http/router.hpp"
#include "nebula/http/detail/router_utils.hpp"
#include "nebula/http/http_methods.h"

#include <algorithm>
#include <expected>
#include <memory>
#include <oneapi/tbb/detail/_utils.h>
#include <ranges>
#include <stdexcept>
#include <string>

namespace nebula::http
{
namespace
{
struct ParsedPath
{
    std::vector<detail::router_utils::ParsedSegment> segments;
    std::vector<detail::Parameter> parameters;
};

auto not_empty()
{
    return std::views::filter([](const auto ctr) { return !ctr.empty(); });
}

ParsedPath parse_path(std::string_view path)
{
    if (path.empty())
    {
        throw std::runtime_error("path is empty");
    }

    std::vector<detail::router_utils::ParsedSegment> segments;
    std::vector<detail::Parameter> parameters;

    std::size_t idx = 0;
    auto segmentsView = std::views::split(path, '/') | not_empty();
    auto it = std::ranges::begin(segmentsView);
    for (; it != std::ranges::end(segmentsView); ++it, ++idx)
    {
        auto parsedSegment = detail::router_utils::parse_segment(std::string_view{*it});

        if (!parsedSegment.has_value())
        {
            throw std::runtime_error("failed to parse segment");
        }

        segments.push_back(*parsedSegment);

        if (parsedSegment->kind != detail::router_utils::SegmentKind::Literal)
        {
            detail::Parameter parameter{.name = std::string{parsedSegment->value}, .idx = idx};
            parameters.push_back(std::move(parameter));
        }

        if (parsedSegment->kind == detail::router_utils::SegmentKind::CatchAll)
        {
            if (std::ranges::next(it) != std::ranges::end(segmentsView))
            {
                throw std::runtime_error("catch all is not termiate node");
            }
            parameters.back().catchAll = true;
        }
    }

    return {.segments = std::move(segments), .parameters = std::move(parameters)};
}

struct MissingBranch
{
    std::unique_ptr<detail::Node> root;
    detail::Node *leaf;
};

MissingBranch buildMissingBranch(std::span<const detail::router_utils::ParsedSegment> segments)
{
    auto root = std::make_unique<detail::Node>();
    auto *leaf = root.get();
    for (const auto &segment : segments)
    {
        auto newNode = std::make_unique<detail::Node>();
        switch (segment.kind)
        {
        case detail::router_utils::SegmentKind::Literal: {
            auto [it, _] = leaf->staticChildren.try_emplace(std::string{segment.value}, std::move(newNode));
            leaf = it->second.get();
            break;
        }
        case detail::router_utils::SegmentKind::Parameter:
            leaf->parameterNode = std::move(newNode);
            leaf = leaf->parameterNode.get();
            break;
        case detail::router_utils::SegmentKind::CatchAll:
            leaf->catchAllNode = std::move(newNode);
            leaf = leaf->catchAllNode.get();
            break;
        }
    }

    return {.root = std::move(root), .leaf = leaf};
}
} // namespace

RouterBuilder::RouterBuilder() : mRoot(std::make_unique<detail::Node>())
{
}

void RouterBuilder::add(HttpMethods method, std::string path, Handler handler)
{
    ParsedPath parsedPath = parse_path(path);
    auto &segments = parsedPath.segments;

    std::size_t missingIdx = 0;

    detail::Node *currentNode = mRoot.get();

    for (; missingIdx < segments.size(); ++missingIdx)
    {
        const auto &segment = segments[missingIdx];
        if (segment.kind == detail::router_utils::SegmentKind::Literal)
        {
            auto &children = currentNode->staticChildren;
            auto it = children.find(segment.value);
            if (it == children.end())
            {
                break;
            }

            currentNode = it->second.get();
            continue;
        }

        detail::Node *nextNode = segment.kind == detail::router_utils::SegmentKind::Parameter
                                     ? currentNode->parameterNode.get()
                                     : currentNode->catchAllNode.get();
        if (!nextNode)
        {
            break;
        }

        currentNode = nextNode;
    }

    detail::Route route{
        .method = method, .handler = std::move(handler), .parameters = std::move(parsedPath.parameters)};
    if (missingIdx < segments.size())
    {
        auto branch = buildMissingBranch(std::span{segments}.subspan(missingIdx + 1));
        branch.leaf->routes.push_back(std::move(route));
        switch (segments[missingIdx].kind)
        {
        case detail::router_utils::SegmentKind::Literal:
            currentNode->staticChildren.try_emplace(std::string{segments[missingIdx].value}, std::move(branch.root));
            break;
        case detail::router_utils::SegmentKind::Parameter:
            currentNode->parameterNode = std::move(branch.root);
            break;
        case detail::router_utils::SegmentKind::CatchAll:
            currentNode->catchAllNode = std::move(branch.root);
            break;
        }
    }
    else if (std::ranges::contains(currentNode->routes, method, &detail::Route::method))
    {
        throw std::runtime_error{"method is already exists"};
    }
    else
    {
        currentNode->routes.push_back(std::move(route));
    }
}

Router RouterBuilder::build() &&
{
    return Router(std::move(mRoot));
}

auto Router::find(std::string_view path, HttpMethods method) const -> std::expected<MatchResult, FindError>
{
    if (!mRoot || path.empty() || path.front() != '/')
    {
        return std::unexpected(FindError{.kind = FindError::Kind::InvalidPath, .allowed = {}});
    }

    std::vector<std::string_view> segments;

    for (auto part : std::views::split(path, '/') | not_empty())
    {
        segments.emplace_back(part.begin(), part.end());
    }

    using SegmentIterator = std::ranges::iterator_t<decltype(segments)>;

    struct Frame
    {
        SegmentIterator it;
        const detail::Node *node;
    };

    // Методы всех узлов, до которых дошёл путь, но где нужного метода не оказалось
    pulsar::Flags<HttpMethods> allowed;

    std::vector<Frame> stack;
    stack.emplace_back(segments.begin(), mRoot.get());
    const auto end = segments.end();

    while (!stack.empty())
    {
        auto [it, node] = stack.back();
        stack.pop_back();

        if (it == end)
        {
            const auto &routes = node->routes;
            const auto routeIt = std::ranges::find(routes, method, &detail::Route::method);
            if (routeIt != routes.end())
            {
                PathParams pathParams{std::string{path}};
                for (const auto &parameter : routeIt->parameters)
                {
                    const auto value = segments[parameter.idx];

                    const auto offset = static_cast<std::size_t>(value.data() - path.data());

                    const auto length = parameter.catchAll ? path.size() - offset : value.size();

                    pathParams.addParam(PathParams::Item{.name = parameter.name,
                                                         .offset = static_cast<std::uint16_t>(offset),
                                                         .len = static_cast<std::uint16_t>(length)});
                }

                return MatchResult{.handler = routeIt->handler, .pathParams = std::move(pathParams)};
            }

            // Путь совпал, метод нет: запоминаем для 405 и откатываемся дальше
            for (const auto &route : routes)
            {
                allowed |= route.method;
            }

            continue;
        }

        const auto next = std::ranges::next(it);
        if (node->catchAllNode)
        {
            stack.emplace_back(end, node->catchAllNode.get());
        }

        if (node->parameterNode)
        {
            stack.emplace_back(next, node->parameterNode.get());
        }

        const auto &staticChildren = node->staticChildren;
        const auto childIt = staticChildren.find(std::string_view(*it));
        if (childIt != staticChildren.end())
        {
            stack.emplace_back(next, childIt->second.get());
        }
    }

    if (allowed)
    {
        return std::unexpected(FindError{.kind = FindError::Kind::MethodNotAllowed, .allowed = allowed});
    }

    return std::unexpected(FindError{.kind = FindError::Kind::NotFound, .allowed = {}});
}
} // namespace nebula::http
