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

/// process
/// @brief
using Process = std::function<void()>;


/// ===============================================================================================
/// Resources
/// @brief
///
/// ===============================================================================================
struct Handler {
    // delete
    Handler(Handler&&) = delete;

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

    /// @brief
    ///
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
    friend void wait(const Handler& handler, Shared space, Process func);
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
    friend void wait(Input, const Handler& handler, Shared space, Process func);
    friend void wait(Error, const Handler& handler, Shared space, Process func);

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
        template <typename Type, typename Callable>
        friend std::enable_if_t<std::is_invocable_r_v<void, Callable, Shared, Base>, void>
        wait(Shared scope, Callable func) {
            wait(Type(), scope->handler, scope->base_, [func, scope, next = scope->base_]() {
                func(scope, next);
            });
        }
        template <typename Type, typename Callable>
        friend std::enable_if_t<std::is_invocable_r_v<void, Callable>, void>
        wait(Type, const Handler& handler, Shared scope, Callable func) {
            wait(Type(), handler, scope->base_, func);
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

template <typename Type, typename Source, typename Base, typename Callable>
std::enable_if_t<std::is_invocable_r_v<void, Callable, fusion::Scope<Source, Base>, Base>, void>
wait(fusion::Scope<Source, Base> scope, Callable func);
