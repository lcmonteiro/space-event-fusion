#pragma once

#include <memory>

namespace Fusion {

/// Kernel 
class Kernel {};

/// Space 
/// @param func 
template <typename Kernel, typename Main>
void Space(Main func) {
    func(std::make_shared<Kernel>());
}

/// Scope
/// @param home 
/// @param func 
namespace {
    template <typename Type, typename Home>
    struct Wrapper : Type {
        Wrapper(Home h) {}
        template <typename Call>
        void wait(Call func) {}
    };
} // namespace
template <typename Type, typename Home, typename Main>
void Scope(Home home, Main func) {
    func(std::make_shared<Wrapper<Type, Home>>(home), home);
}

} // namespace Fusion