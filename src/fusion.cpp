#include "fusion.hpp"


#include <error.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cassert>
#include <system_error>

namespace fusion {

/// Handler
/// @brief
///
template <>
Handler::Handler(int h) : native_{h} {
    if (native_ < 0)
        throw std::runtime_error(std::string("resource: ") + strerror(errno));
}

Handler::~Handler() {
    if (native_ > 0)
        close(native_);
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

    // run until empty cache 
    while (!cache_.empty()) {
        // wait for events
        auto count = ::epoll_wait(handler.native(), events.data(), events.size(), -1);

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
                    events &= ~mask;
                    try {
                        auto node = proc.extract(id);
                        node.mapped()();
                    }
                    catch (const std::exception& e) {
                        std::cerr << "space::run:" << e.what() << std::endl;
                    }
                }
            }
            // check & update cache
            if (proc.empty())
                cache_.erase(ev.data.fd);
        }
        events.resize(events.capacity());
    }
}

/// Space
/// @brief
///
void wait(Input, const Handler& handler, const Space::Shared& space, Process func) {
    auto& [events, procs] = space->cache_[handler.native()];
    // update event
    epoll_event ev;
    ev.events  = events | EPOLLIN | EPOLLONESHOT;
    ev.data.fd = handler.native();
    assert(
      ::epoll_ctl(
        space->handler.native(), events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, handler.native(), &ev)
      == 0);
    // update cache
    procs[Input::id] = func;
    events           = ev.events;
}
void wait(Output, const Handler& handler, const Space::Shared& space, Process func) {
    auto& [events, procs] = space->cache_[handler.native()];
    // update event
    epoll_event ev;
    ev.events  = events | EPOLLOUT | EPOLLONESHOT;
    ev.data.fd = handler.native();
    assert(
      ::epoll_ctl(
        space->handler.native(), events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, handler.native(), &ev)
      == 0);
    // update cache
    procs[Output::id] = func;
    events            = ev.events;
}
void wait(Error, const Handler& handler, const Space::Shared& space, Process func) {
    auto& [events, procs] = space->cache_[handler.native()];
    // update event
    epoll_event ev;
    ev.events  = events | EPOLLERR | EPOLLHUP | EPOLLONESHOT;
    ev.data.fd = handler.native();
    assert(
      ::epoll_ctl(
        space->handler.native(), events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, handler.native(), &ev)
      == 0);
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