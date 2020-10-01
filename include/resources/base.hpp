#pragma once

#include <memory>

namespace Fusion {
namespace Resource {
    class Base : std::enable_shared_from_this<Base> {
      public:
        Base() = default;
        Base(int handler);
        auto self() { return shared_from_this(); }

      protected:
        auto native_() { return handler_; }

      private:
        int handler_{-1};
    };
} // namespace Resource
} // namespace Fusion