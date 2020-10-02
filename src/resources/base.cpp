#include "resources/base.hpp"

#include <unistd.h>
#include <string.h>

namespace Fusion {
namespace Resource {
    Base::Base()
      : handler_{-1} {}
    Base::Base(int handler)
      : handler_{handler} {
        if (handler_ < 0)
            throw std::runtime_error(std::string("resource: ") + strerror(errno));
    }
    Base::~Base() {
        if (handler_ > 0)
            close(handler_);
    }
} // namespace Resource
} // namespace Fusion