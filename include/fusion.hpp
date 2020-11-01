#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <type_traits>

namespace fusion {

/// Fusion Elements
/// @brief
struct Element;
struct Cluster;
struct Space;

namespace {}

using pElement = std::shared_ptr<Element>;
using pCluster = std::shared_ptr<Cluster>;
using pSpace   = std::shared_ptr<Space>;

namespace {}

using Task = std::function<void()>;

/// Space
/// @brief
/// @param func
template <typename Space, typename Callable>
void space(Callable func) {
    func(std::make_shared<Space>());
}



/// wait functions
/// @brief
template <typename Resource, typename Base, typename Callable>
std::enable_if_t<
    std::conjunction_v<
        std::is_convertible<Resource, pElement>,
        std::is_invocable_r<void, Callable, Resource, Base>>,
    void>
wait(Resource resource, Base base, Callable func) {
    wait(std::static_pointer_cast<Element>(resource), base, Task([=]() { func(resource, base); }));
}



/// wait functions
/// @brief
// template <typename Base>
// std::enable_if_t<std::conjunction_v<std::is_convertible<Base, pSpace>>, void>
// wait(pElement resource, Base base, Task func) {
//     wait(resource, std::static_pointer_cast<Space>(base), func);
// }

template <typename Resource, typename... Args>
void build(Resource resource, Args&&... args) {}

template <typename Resource, typename... Args>
void clear(Resource resource, Args&&... args) {}


/// Element
/// @brief
struct Element {
    using Handler = int;

    Element();
    virtual ~Element();

    Element(Element&&) = default;
    Element& operator=(Element&&) = default;

  protected:
    Element(Handler handler);

    const Handler handler;
};

/// Cluster
/// @brief
struct Cluster : Element {
    friend void wait(pElement, pCluster, Task);
};

/// Space object
/// @brief
struct Space {
    friend void wait(pElement, pSpace, Task);
};

namespace {
    template <typename R, typename B>
    struct Scope;
    template <typename R, typename B>
    using pScope = std::shared_ptr<Scope<R, B>>;

    template <typename Resource, typename Base>
    void wait(pElement resource, pScope<Resource, Base> base, Task func);
    template <typename Resource, typename Base>
    struct Scope : Resource {
        Scope(Base base) : next(base) {}

      protected:
        friend void wait<>(pElement resource, pScope<Resource, Base> base, Task func);

      private:
        const Base next;
    };

    /// wait functions
    /// @brief
    template <typename Resource, typename Base>
    void wait(pElement resource, pScope<Resource, Base> base, Task func) {
        wait(resource, base->next, func);
    }
} // namespace

/// Scope
/// @brief
template <typename Resource, typename Space, typename Callable>
void scope(Space space, Callable func) {
    try {
        func(std::make_shared<Scope<Resource, Space>>(space), space);
    } catch (...) {}
}
} // namespace fusion
