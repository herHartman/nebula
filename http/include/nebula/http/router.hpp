#pragma once

#include "nebula/http/handler.hpp"
#include "nebula/http/http_methods.h"
#include "pulsar/enum_flags.hpp"
#include <cstdint>
#include <expected>
#include <flat_map>
#include <memory>
#include <optional>

namespace nebula::http
{
namespace detail
{
struct Parameter
{
    std::string name;
    std::size_t idx = 0;
    bool catchAll = false;
};

struct Route
{
    HttpMethods method;
    Handler handler;
    std::vector<Parameter> parameters;
};

struct Node
{
    std::flat_map<std::string, std::unique_ptr<Node>, std::less<>> staticChildren;
    std::unique_ptr<Node> parameterNode;
    std::unique_ptr<Node> catchAllNode;
    std::vector<Route> routes;
};
} // namespace detail

struct FindError
{
    enum class Kind : std::uint8_t
    {
        InvalidPath,
        NotFound,
        MethodNotAllowed
    } kind;
    pulsar::Flags<HttpMethods> allowed;
};

class PathParams
{
  public:
    std::optional<std::string_view> getParam(std::string_view name);

  private:
    struct Item
    {
        std::string_view name;
        std::uint16_t offset;
        std::uint16_t len;
    };

    friend class Router;

    explicit PathParams(std::string url) noexcept;

    void addParam(Item item);

    static constexpr std::uint8_t kMaxSize = 16;
    std::array<Item, kMaxSize> items;
    std::string mUrl;
};

class Router
{
  public:
    struct MatchResult
    {
        const Handler &handler;
        PathParams pathParams;
    };

    std::expected<MatchResult, FindError> find(std::string_view path, HttpMethods method) const;

  private:
    Router() = delete;
    Router(const Router &) = delete;
    Router &operator=(const Router &) = delete;

    friend class RouterBuilder;

    Router(std::unique_ptr<detail::Node> root);

    std::unique_ptr<detail::Node> mRoot;
};

class RouterBuilder
{
  public:
    RouterBuilder();
    void add(HttpMethods method, std::string path, Handler handler);
    Router build() &&;

  private:
    std::unique_ptr<detail::Node> mRoot;
};
} // namespace nebula::http
