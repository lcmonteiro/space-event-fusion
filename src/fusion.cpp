#include "fusion.hpp"


#include <error.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cassert>
#include <system_error>

namespace fusion {

template <typename T>
void print_cache(T& cache) {
    for (auto& [fd, data] : cache) {
        std::cout << "fd= " << fd << std::endl;
        auto& [ev, procs] = data;
        std::cout << "ev= " << ev << std::endl;
        for (auto& [id, pr] : procs) {
            std::cout << "- id= " << id << std::endl;
            std::cout << "- pr= " << &pr << std::endl;
        }
    }
    std::cout << std::endl;
}

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
    for (;;) {
        // wait for events
        auto count = ::epoll_wait(handler.native(), events.data(), events.size(), -1);

        std::cout << __FILE__ << ":" << __LINE__ << ":" << events.size() << std::endl;
        print_cache(cache_);
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

            std::cout << "fd= " << ev.data.fd << "---------------------" << std::endl;
            // process resource events
            for (auto& [id, mask] : EVENT_MAP) {
                if (ev.events & mask) {
                    events &= ~mask;
                    try {

                        std::cout << "id= " << id << std::endl;
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
                if (::epoll_ctl(handler.native(), EPOLL_CTL_DEL, ev.data.fd, NULL) < 0)
                    std::cerr << ev.data.fd << std::endl;
                    
                // throw std::system_error(std::make_error_code(std::errc(errno)));
                cache_.erase(ev.data.fd);
            }
        }
        events.resize(events.capacity());
    }
}

/// Space
/// @brief
///
void wait(Input, const Handler& handler, const Space::Shared& space, Process func) {
    std::cout << "wait input= " << handler.native() << std::endl;
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