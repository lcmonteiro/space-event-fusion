#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>


#include <iostream>

namespace fusion {

/// ===============================================================================================
/// Types
/// @brief
///
/// ===============================================================================================
/// waiting types
/// @brief
template <int n>
struct type {
    constexpr static int id = n;
    explicit type()         = default;
};
using Output = type<0>;
using Input  = type<1>;
using Error  = type<3>;

/// extended wainting types
struct Connection : Input {};

/// process
/// @brief
using Process = std::function<void()>;


/// ===============================================================================================
/// Resources
/// @brief
///
/// ===============================================================================================
struct Handler {
    // deleted
    Handler(Handler&&) = delete;
    Handler(Handler&)  = delete;

    /// @brief
    ///
    template <typename Native>
    Handler(Native id);

    /// @brief
    ///
    ~Handler();

    /// @brief
    ///
    const int native;
};

/// Cluster
/// @brief
struct Element {
  protected:
    /// @brief
    ///
    template <typename Native>
    Element(Native id) : handler{id} {}

    /// wait
    /// @brief
    template <typename Type>
    friend Process wait(Type, Element& elem, Process proc) {
        std::cout << __FILE__ << __LINE__ << std::endl;
        return proc;
    }

    /// handler
    /// @brief
    const Handler handler;
};

/// Cluster
/// @brief
struct Cluster {
    using Shared = std::shared_ptr<Cluster>;
    /// @brief
    ///
    const Handler handler;

    /// wait
    /// @brief
    ///
    template <typename Type>
    friend Process wait(Type, Cluster& cluster, Process proc) {
        return proc;
    }

    /// wait
    /// @brief
    ///
    friend void wait(const Handler& handler, Shared& space, Process func);
};

/// Space
/// @brief
struct Space {
    using Shared = std::shared_ptr<Space>;

    /// constructor
    /// @brief
    Space();

    /// run
    /// @brief
    void run();

  protected:
    /// wait
    /// @brief
    friend void wait(Input, const Handler& handler, const Shared& space, Process func);
    friend void wait(Error, const Handler& handler, const Shared& space, Process func);

  private:
    /// @brief
    ///
    const Handler handler;

    /// cache
    /// @brief
    std::unordered_map<int, std::tuple<int, std::map<int, Process>>> cache_;
};

/// ===============================================================================================
/// Scope
/// @brief
///
/// ===============================================================================================
namespace {
    template <typename Source, typename Base>
    struct scope : Source {
        using Shared = std::shared_ptr<scope>;

        /// constructor
        /// @brief
        template <typename... Args>
        scope(Base base, Args&&... args) : Source(std::forward<Args>(args)...), base_(base) {}

      protected:
        /// wait
        /// @brief
        template <typename Type, typename... Args>
        friend void wait(Shared scope, std::function<void(Shared, Base)> func, Args&&... args) {
            wait(
              Type(),
              scope->handler,
              scope->base_,
              wait(
                Type(),
                *scope,
                [call = std::move(func), scope, next = scope->base_]() { call(scope, next); },
                std::forward<Args>(args)...));
        }

        /// wait
        /// @brief
        template <typename Type>
        friend void wait(Type, const Handler& handler, const Shared& scope, Process func) {
            wait(Type(), handler, scope->base_, std::move(func));
        }

      private:
        /// @brief
        /// base element
        const Base base_;
    };
    template <typename Source>
    struct scope<Source, void> : Source {
        using Shared = std::shared_ptr<scope>;
    };
    template <typename Source, typename Base>
    using Scope = std::shared_ptr<scope<Source, Base>>;

} // namespace
} // namespace fusion

/// ===============================================================================================
/// Build - Interfaces
/// @brief
///
/// ===============================================================================================
template <typename Source, typename Callable, typename... Args>
std::enable_if_t<std::is_invocable_r_v<void, Callable, fusion::Scope<Source, void>>, void>
build(Callable call, Args&&... args) {
    auto source = std::make_shared<Source>(std::forward<Args>(args)...);
    call(source);
    source->run();
}
template <typename Source, typename Base, typename Callable, typename... Args>
std::enable_if_t<std::is_invocable_r_v<void, Callable, fusion::Scope<Source, Base>, Base>, void>
build(Base base, Callable call, Args&&... args) {
    call(std::make_shared<fusion::scope<Source, Base>>(base, std::forward<Args>(args)...), base);
}

/// ===============================================================================================
/// Wait - Interfaces
/// @brief
///
/// ===============================================================================================
template <
  typename Type,
  typename Source,
  typename Base,
  typename Shared = fusion::Scope<Source, Base>,
  typename... Args>
void wait(Shared, std::function<void(Shared, Base)>, Args&&...);
