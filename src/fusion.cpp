#include "fusion.hpp"

#include <string.h>
#include <unistd.h>

namespace fusion {


Element::Element() : handler{-1} {}

Element::Element(int h) : handler{h} {
    if (handler < 0)
        throw std::runtime_error(std::string("resource: ") + strerror(errno));
}
Element::~Element() {
    if (handler > 0)
        close(handler);
}


void wait(pElement, pCluster, Task) {}

void wait(pElement, pSpace, Task) {}

} // namespace fusion