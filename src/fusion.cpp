#include "fusion.hpp"

#include <error.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>

#include <system_error>

namespace fusion {

/// Handler
/// @brief
///
template <>
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
void wait(const Handler& handler, Space::Shared space, Process func) {
    // update process list
    space->processes_[handler.native] = func;

    // transform
    std::vector<pollfd> fds;
    std::transform(
        space->processes_.begin(),
        space->processes_.end(),
        std::back_inserter(fds),
        [](auto& v) {
            return pollfd{
                .fd      = std::get<0>(v),
                .events  = POLLIN | POLLERR | POLLRDHUP,
                .revents = 0};
        });

    // wait for events
    auto count = ::poll(fds.data(), fds.size(), -1);

    // check error
    if (count < 0)
        throw std::system_error(std::make_error_code(std::errc(errno)));

    // process events
    for (auto& fd : fds) {
        if (fd.revents) {
            if (POLLIN == fd.revents) {
                try {
                    space->processes_[fd.fd]();
                } catch (const std::exception& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            if (--count == 0) {
                return;
            }
        }
    }
}

/// Cluster
/// @brief
///
void wait(const Handler& handler, Cluster::Shared space, Process func) {
    std::cout << __func__ << "" << __LINE__ << std::endl;
}



} // namespace fusion