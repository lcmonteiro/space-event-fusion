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

// base 
using Process = std::function<void()>;

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

/// Cluster
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

/// Cluster
/// @brief
struct Cluster {
    using Shared  = std::shared_ptr<Cluster>;

    /// wait
    /// @brief
    ///
    friend void wait(const Handler& handler, Shared& space, Process func);

  protected:
    Handler handler_;
};

/// Space
/// @brief
struct Space {
    using Shared  = std::shared_ptr<Space>;
    
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
    friend void wait(Output, const Handler& handler, const Shared& space, Process func);
    friend void wait(Error, const Handler& handler, const Shared& space, Process func);

  private:
    Handler handler_;

    /// processes cache
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
        template <typename Action, typename Callable, typename... Args>
        friend void wait(Action, const Shared& scope, Callable func, Args&&... args) {
            wait(
              Action(),
              scope->handler_,
              scope->base_,
              wait(
                Action(),
                *scope,
                [call = std::move(func), scope, next = scope->base_](auto... args) {
                    call(scope, next, std::move(args)...);
                },
                std::forward<Args>(args)...));
        }
        template <int n, typename Callable, typename... Args>
        friend void wait(basetype<n>, const Shared& scope, Callable func, Args&&... args) {
            wait(
              basetype<n>(),
              scope->handler_,
              scope->base_,
              [call = std::move(func), scope, next = scope->base_] { call(scope, next); });
        }

        /// wait
        /// @brief
        template <typename Type, typename Process>
        friend void wait(Type, const Handler& handler, const Shared& scope, Process&& func) {
            wait(Type(), handler, scope->base_, std::forward<Process>(func));
        }

      private:
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
template <typename Space, typename Process>
void function(Space space, Process func) {
    func(space, func);
}
