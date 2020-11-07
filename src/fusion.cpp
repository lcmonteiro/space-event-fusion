#include "fusion.hpp"

#include <string.h>
#include <unistd.h>

namespace fusion {

/// Handler
/// @brief
///
template<>
Handler::Handler(int h) : native{h} {
    if (native < 0)
        throw std::runtime_error(std::string("resource: ") + strerror(errno));
}
Handler::~Handler() {
    if (native > 0)
        close(native);
}

/// Space
/// @brief
///


/// Space
/// @brief
///
void wait(const Handler& handler, Space::Shared space, Process func) {
    // update process list
    //space->processes_[handler] = func;


    // int r = 0;
    // if ((r = ::poll(handlers_.data(), handlers_.size(), -1) < 0) {
    //     throw std::runtime_error(make_error_code(errc(errno)));
    // }
    // /**
    //  * check
    //  */
    // list<size_t> res;
    // for (size_t i = 0, n = r; (i < __locations.size()) && (res.size() < n); ++i) {
    //     if (__locations[i].revents & __locations[i].events) {
    //         res.emplace_back(i);
    //     }
    //     /**
    //      * clear event
    //      */
    //     __locations[i].revents = 0;
    // }
    // return res;
    std::cout << __func__ << "" << __LINE__ << std::endl;
}

/// Cluster
/// @brief
///
void wait(const Handler& handler, Cluster::Shared space, Process func) {
    std::cout << __func__ << "" << __LINE__ << std::endl;
}



} // namespace fusion