#pragma once

#include <memory>

namespace Fusion {

class Kernel {};

template <typename Kernel, typename Main>
void Space(Main func) {
    func(std::make_shared<Kernel>());
}

namespace {
    template <typename Type, typename Home>
    struct Decorator : Type {
        Decorator(Home h) {}
        template <typename Call>
        void wait(Call func) {}
    };
} // namespace
template <typename Type, typename Home, typename Main>
void Scope(Home home, Main func) {
    func(std::make_shared<Decorator<Type, Home>>(home), home);
}

} // namespace Fusion