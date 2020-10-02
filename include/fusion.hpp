#pragma once

#include <memory>

namespace Fusion {

/// Kernel
class Kernel {
  public:
    void wait(std::function<void(void)>) {}
};

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
    class Wrapper : public Type {
      public:
        Wrapper(Home h)
          : home_{h} {}

        template <typename... Args>
        void build(Args&&... args) {
            *this = Type(std::forward<Args>(args)...);
        }

        template <typename Call>
        void wait(Call func) {
            home_->wait([func, this, self = this->self()]() {
                func(std::static_pointer_cast<Wrapper>(self), home_);
            });
        }

      protected:
        using Type::operator=;

      private:
        Home home_;
    };
} // namespace
template <typename Type, typename Home, typename Main>
void Scope(Home home, Main func) {
    func(std::make_shared<Wrapper<Type, Home>>(home), home);
}

} // namespace Fusion