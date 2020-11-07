#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>


#include <iostream>

namespace fusion {

/// Fusion Elements
/// @brief
struct Element;
struct Cluster;
struct Space;

namespace shared {
    using Element = std::shared_ptr<Element>;
    using Cluster = std::shared_ptr<Cluster>;
    using Space   = std::shared_ptr<Space>;
} // namespace shared

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
    /// Constructor
    /// @brief
    /// Space();

    /// Destructor
    /// @brief
    /// virtual ~Space();
protected:
    /// wait
    /// @brief
    friend void wait(const Handler& handler, Shared space, Process func);
private:
    /// processes
    /// @brief
    std::map<int, Process> processes_;
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
        template <typename Callable>
        friend std::enable_if_t<std::is_invocable_r_v<void, Callable, Shared, Base>, void>
        wait(Shared scope, Callable func) {
            wait(scope->handler, scope->base_, [func, scope, next = scope->base_]() {
                func(scope, next);
            });
        }


        template <typename Callable>
        friend std::enable_if_t<std::is_invocable_r_v<void, Callable>, void>
        wait(const Handler& handler, Shared scope, Callable func) {
            wait(handler, scope->base_, func);
        }



        // friend class fusion::Waiter;
        /// @brief
        /// base element
        const Base base_;
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
std::enable_if_t<std::is_invocable_r_v<void, Callable, fusion::Space::Shared>, void>
build(Callable call, Args&&... args) {
    call(std::make_shared<Source>(std::forward<Args>(args)...));
}
template <typename Source, typename Base, typename Callable, typename... Args>
std::enable_if_t<std::is_invocable_r_v<void, Callable, fusion::Scope<Source, Base>, Base>, void>
build(Base base, Callable call, Args&&... args) {
    call(std::make_shared<fusion::scope<Source, Base>>(base, std::forward<Args>(args)...), base);
}