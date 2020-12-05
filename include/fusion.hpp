/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file fusion.hpp                                                    _| |  | (_-<  |   _ \    \
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
using Output = basetype<1>;
using Input  = basetype<2>;
using Error  = basetype<3>;
using Delete = basetype<-1>;

// base
using Process = std::function<void()>;

// callback
template <typename... Args>
using Callback = std::function<void(Args...)>;

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

// exceptions
namespace exception {
    struct Continue : std::runtime_error {};
} // namespace exception


/// ===============================================================================================
/// Resources
/// @brief
///
/// ===============================================================================================
struct Handler {
    // deleted
    Handler(const Handler&) = delete;

    // default
    Handler() = default;

    /// constructor
    /// @brief
    template <typename Native>
    Handler(Native id);

    /// move constructor
    /// @brief
    Handler(Handler&& h) { std::swap(native_, h.native_); }

    /// move assign
    /// @brief
    Handler& operator=(Handler&& h) {
        std::swap(native_, h.native_);
        return *this;
    }

    /// native
    /// @brief
    auto native() const { return native_; }

    /// destructor
    /// @brief
    ~Handler();

  private:
    int native_{-1};
};

/// Element
/// @brief
struct Element {
  protected:
    Element() = default;

    /// constructor
    /// @brief
    Element(Handler&& id) : handler_{std::move(id)} {}

  protected:
    Handler handler_;
};


/// Space
/// @brief
struct Space {
    using Shared = std::shared_ptr<Space>;

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
    friend void wait(Input, const Handler& handler, const Shared& space, Process func);
    friend void wait(Output, const Handler& handler, const Shared& space, Process func);
    friend void wait(Error, const Handler& handler, const Shared& space, Process func);

  private:
    struct Cache;

    /// native resource handler
    const Handler handler_;

    /// state cache
    const std::unique_ptr<Cache> cache_;
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
        scope(Base base, Args&&... args)
          : Source(std::forward<Args>(args)...), base_(base), delete_() {}

      protected:
        /// wait
        /// @brief
        template <typename Action, typename Callable, typename... Args>
        friend constexpr void wait(Action, const Shared& scope, Callable func, Args&&... args) {
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
        wait(basetype<n>, const Shared& scope, Callable func, Args&&... args) {
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
        friend constexpr void wait(Delete, const Shared& scope, Callable func, Args&&... args) {
            scope->destroy_ = [call = std::move(func), scope, next = scope->base_] {
                call(scope, next);
            };
        }

        /// wait
        /// @brief recursive method until space fusion
        template <typename Type, typename Process>
        friend constexpr void
        wait(Type, const Handler& handler, const Shared& scope, Process&& func) {
            wait(Type(), handler, scope->base_, std::forward<Process>(func));
        }

      protected:
        /// scope guard
        /// @brief
        struct Guard {
            Guard(const Shared& _scope) : scope(_scope) {}
            ~Guard() {
                if (scope.use_count() == 1)
                    if (scope->delete_)
                        scope->delete_(scope, scope->base_);
            }
            Shared scope;
        };

      private:
        // base space
        const Base base_;

        // callback before end
        std::function<void(const Shared&, const Base&)> delete_;
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
template <typename Type, typename Shared, typename Callable, typename... Args>
void wait(Shared&& scope, Callable&& callable, Args&&... args) {
    wait(
      Type(),
      std::forward<Shared>(scope),
      std::forward<Callable>(callable),
      std::forward<Args>(args)...);
}

/// ===============================================================================================
/// Helpers
/// @brief
///
/// ===============================================================================================
template <typename Scope, typename Callable>
void call(Scope scope, Callable func) {
    func(func, scope);
}
