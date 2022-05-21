/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file fusion.hpp                                                    _| |  | (_-<  |   _ \    \.
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <type_traits>
#include <variant>

namespace fusion {
/// ===============================================================================================
/// Types
/// @brief
///
/// ===============================================================================================
/// waiting base types
/// @brief
template <int n>
struct basetype {
    constexpr static int id = n;
    explicit basetype()     = default;
};
using Output   = basetype<1>;
using Input    = basetype<2>;
using Error    = basetype<3>;
using Continue = basetype<0>;
using Delete   = basetype<-1>;

/// extended wainting types
/// - Connection
template <typename Type>
struct Connection : Type {};
namespace output {
    using Connection = fusion::Connection<Output>;
}
namespace input {
    using Connection = fusion::Connection<Input>;
}

/// exceptions
namespace exception {
    struct Rollback : std::exception {};
} // namespace exception

/// data
using Buffer = std::vector<std::byte>;
using String = std::string;

// - process
using Process = std::function<void()>;
template <typename... Args>
using Callback = std::function<void(Args...)>;

/// ===============================================================================================
/// Element
/// @brief
///
/// ===============================================================================================
struct Element {
    struct Handler;
    using Shared = std::shared_ptr<Handler>;

  protected:
    /// base constructor
    /// @brief
    Element();

    /// base destructor
    /// @brief
    ~Element();

    /// native constructor
    /// @brief
    template <typename Native>
    Element(Native handler);

    /// native interface
    /// @brief
    template <typename Result, typename... Args>
    Result native(Args... args);

    /// native handler
    /// @brief
    const std::shared_ptr<Handler> handler_;
};

/// ===============================================================================================
/// Space
/// @brief
///
/// ===============================================================================================
struct Space {
    using Pointer = std::shared_ptr<Space>;
    /// constructor
    /// @brief
    Space();

    /// destructor
    /// @brief
    ~Space();

    /// run
    /// @brief
    void run();

  protected:
    /// wait
    /// @brief
    friend void wait(Input, const Element::Shared& handler, const Pointer& space, Process&& func);
    friend void wait(Output, const Element::Shared& handler, const Pointer& space, Process&& func);
    friend void wait(Error, const Element::Shared& handler, const Pointer& space, Process&& func);

  private:
    struct Handler;

    /// native resource handler
    const std::unique_ptr<Handler> handler_;
};

/// ===============================================================================================
/// Scope
/// @brief
///
/// ===============================================================================================
namespace {
    template <typename Source, typename Base>
    struct scope : Source {
        using Pointer = std::shared_ptr<scope>;

        /// constructor
        /// @brief
        template <typename... Args>
        scope(Base base, Args&&... args)
          : Source(std::forward<Args>(args)...), base_(base), delete_() {}

      protected:
        /// wait
        /// @brief
        template <typename Action, typename Callable, typename... Args>
        friend constexpr void wait(Action, const Pointer& scope, Callable func, Args&&... args) {
            wait(
              Action(),
              scope->handler_,
              scope->base_,
              wait(
                Action(),
                *scope,
                [call = std::move(func), guard = Guard(scope), next = scope->base_](auto... args) {
                    call(guard.scope, next, std::move(args)...);
                },
                std::forward<Args>(args)...));
        }

        /// wait
        /// @brief
        template <int n, typename Callable, typename... Args>
        friend constexpr void
        wait(basetype<n>, const Pointer& scope, Callable func, Args&&... args) {
            wait(
              basetype<n>(),
              scope->handler_,
              scope->base_,
              [call = std::move(func), guard = Guard(scope), next = scope->base_] {
                  call(guard.scope, next);
              });
        }

        /// wait
        /// @brief
        template <typename Callable, typename... Args>
        friend constexpr void wait(Delete, const Pointer& scope, Callable func) {
            scope->destroy_ = [call = std::move(func), scope, next = scope->base_] {
                call(scope, next);
            };
        }

        /// wait
        /// @brief recursive method until space fusion
        template <typename Type, typename Handler, typename Callable>
        friend constexpr void
        wait(Type, const Handler& handler, const Pointer& scope, Callable&& func) {
            wait(Type(), handler, scope->base_, std::forward<Callable>(func));
        }

      protected:
        /// scope guard
        /// @brief
        struct Guard {
            Guard(const Pointer& _scope) : scope(_scope) {}
            ~Guard() {
                if (scope.use_count() == 1)
                    if (scope->delete_)
                        scope->delete_(scope, scope->base_);
            }
            Pointer scope;
        };

      private:
        // base space
        const Base base_;

        // callback before end
        std::function<void(const Pointer&, const Base&)> delete_;
    };
    template <typename Source>
    struct scope<Source, void> : Source {
        using Pointer = std::shared_ptr<scope>;
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
template <typename Type, typename Pointer, typename Callable, typename... Args>
constexpr void wait(Pointer&& scope, Callable&& callable, Args&&... args) {
    wait(
      Type(),
      std::forward<Pointer>(scope),
      std::forward<Callable>(callable),
      std::forward<Args>(args)...);
}

/// ===============================================================================================
/// Call - Interfaces 
/// @brief
///
/// ===============================================================================================
template <typename Scope, typename Callable>
constexpr void call(Scope scope, Callable func) {
    func(scope, std::move(func));
}
template <typename Scope, typename Callable>
constexpr void call(Scope scope, const std::shared_ptr<Callable>& pfunc) {
    (*pfunc)(scope, pfunc);
}
