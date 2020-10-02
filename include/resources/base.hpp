#pragma once

#include <memory>

namespace Fusion {
namespace Resource {
    class Base : std::enable_shared_from_this<Base> {
      public:
        Base();
        auto self() { return shared_from_this(); }
        virtual ~Base();

      protected:
        Base(int handler);
        auto native_() { return handler_; }

      private:
        int handler_;
    };
} // namespace Resource
} // namespace Fusion