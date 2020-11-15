#include "fusion.hpp"

#include <error.h>
#include <string.h>
#include <sys/epoll.h>
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

/// constructor
/// @brief
///
Space::Space() : handler{::epoll_create1(0)} {}

/// run
/// @brief
///
void Space::run() {
    std::vector<epoll_event> events(10);
    for (;;) {
        // wait for events
        auto count = ::epoll_wait(handler.native, events.data(), events.size(), -1);

        // check error
        if (count < 0)
            throw std::system_error(std::make_error_code(std::errc(errno)));

        // process events
        const std::array<std::tuple<int, int>, 3> EVENT_MAP{
          std::tuple{Input::id, EPOLLIN},
          std::tuple{Output::id, EPOLLOUT},
          std::tuple{Error::id, EPOLLERR | EPOLLHUP}};
        events.resize(count);
        for (auto& ev : events) {
            // find resource cached processes
            auto& [events, proc] = cache_[ev.data.fd];

            // process resource events
            for (auto& [id, mask] : EVENT_MAP) {
                if (ev.events & mask) {
                    ev.events &= ~mask;
                    try {
                        auto node = proc.extract(id);
                        node.mapped()();
                    }
                    catch (const std::exception& e) {
                        std::cerr << e.what() << std::endl;
                    }
                }
            }
            // check resources process
            if (proc.empty()) {
                ::epoll_ctl(handler.native, EPOLL_CTL_DEL, ev.data.fd, NULL);
            }
        }
        events.resize(events.capacity());
    }
}

/// Space
/// @brief
///
void wait(Input, const Handler& handler, const Space::Shared& space, Process func) {
    auto& [events, procs] = space->cache_[handler.native];
    // update event
    epoll_event ev;
    ev.events  = events | EPOLLIN;
    ev.data.fd = handler.native;
    ::epoll_ctl(space->handler.native, events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, handler.native, &ev);

    // update cache
    procs[Input::id] = func;
    events           = ev.events;
}
void wait(Error, const Handler& handler, const Space::Shared& space, Process func) {
    auto& [events, procs] = space->cache_[handler.native];
    // update event
    epoll_event ev;
    ev.events  = events | EPOLLERR | EPOLLHUP;
    ev.data.fd = handler.native;
    ::epoll_ctl(space->handler.native, events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, handler.native, &ev);

    // update cache
    procs[Error::id] = func;
    events           = ev.events;
}

/// Cluster
/// @brief
///
void wait(const Handler& handler, Cluster::Shared space, Process func) {
    std::cout << __func__ << "" << __LINE__ << std::endl;
}



} // namespace fusion