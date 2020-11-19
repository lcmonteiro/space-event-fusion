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
using Output = type<1>;
using Input  = type<2>;
using Error  = type<3>;

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
    Handler(const Handler&) = delete;

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
    /// @brief
    ///
    int native_;
};

/// Cluster
/// @brief
struct Element {
  protected:
    /// @brief
    ///
    Element(Handler&& id) : handler{std::move(id)} {}

    /// wait
    /// @brief
    template <typename Type>
    friend Process wait(Type, Element& elem, Process proc) {
        return proc;
    }

  protected:
    /// handler
    /// @brief
    Handler handler;
};

/// Cluster
/// @brief
struct Cluster {
    using Shared = std::shared_ptr<Cluster>;

    /// wait
    /// @brief
    template <typename Type>
    friend Process wait(Type, Cluster& cluster, Process proc) {
        return proc;
    }

    /// wait
    /// @brief
    ///
    friend void wait(const Handler& handler, Shared& space, Process func);

  protected:
    /// @brief
    ///
    Handler handler;
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
    friend void wait(Output, const Handler& handler, const Shared& space, Process func);
    friend void wait(Error, const Handler& handler, const Shared& space, Process func);

  private:
    /// @brief
    ///
    Handler handler;

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
                [call = std::move(func), scope, next = scope->base_] { call(scope, next); },
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

/// ===============================================================================================
/// Helpers
/// @brief
///
/// ===============================================================================================
template <typename Space, typename Process>
void function(Space space, Process func) {
    func(space, func);
}
