/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file fusion.cpp                                                    _| |  | (_-<  |   _ \    \ 
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#include "fusion.hpp"

#include <error.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cassert>
#include <system_error>

namespace fusion {

/// ===============================================================================================
/// Handler implementation
/// - constructor
/// - desctructor
/// ===============================================================================================

template <>
Handler::Handler(int h) : native_{h} {
    if (native_ < 0)
        throw std::runtime_error(std::string("resource: ") + strerror(errno));
}

Handler::~Handler() {
    if (native_ > 0)
        close(native_);
}


/// ===============================================================================================
/// Space implementation
/// - run
/// - wait
/// ===============================================================================================
/// constructor
/// @brief
Space::Space() : handler_{::epoll_create1(0)} {}

/// run
/// @brief
void Space::run() {
    std::vector<epoll_event> events(10);

    // run until empty cache
    while (!cache_.empty()) {
        // wait for events
        auto count = ::epoll_wait(handler_.native(), events.data(), events.size(), -1);

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
            auto& [events, procs] = cache_[ev.data.fd];

            // process resource events
            for (auto& [id, mask] : EVENT_MAP) {
                if (ev.events & mask) {
                    events &= ~mask;
                    auto node = procs.extract(id);
                    try {
                        node.mapped()();
                    }
                    catch (const exception::Continue&) {
                        // reinsert
                        events |= mask;
                        procs.insert(std::move(node));
                        // update
                        epoll_event e;
                        e.events  = events;
                        e.data.fd = ev.data.fd;
                        assert(::epoll_ctl(handler_.native(), EPOLL_CTL_MOD, e.data.fd, &e) == 0);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "space::run:" << e.what() << std::endl;
                    }
                }
            }
            // check & update cache
            if (procs.empty())
                cache_.erase(ev.data.fd);
        }
        events.resize(events.capacity());
    }
}

/// Wait
/// @brief
template <int id, int flags, typename Cache, typename Process>
void wait(const Handler& base, Cache& cache, const Handler& source, Process& proc) {
    auto& [events, procs] = cache[source.native()];
    // update event
    epoll_event ev;
    ev.events  = events | flags | EPOLLONESHOT;
    ev.data.fd = source.native();
    assert(
      ::epoll_ctl(base.native(), events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, source.native(), &ev)
      == 0);
    // update cache
    procs[id] = proc;
    events    = ev.events;
}
void wait(Input, const Handler& handler, const Space::Shared& space, Process func) {
    wait<Input::id, EPOLLIN>(space->handler_, space->cache_, handler, func);
}
void wait(Output, const Handler& handler, const Space::Shared& space, Process func) {
    wait<Output::id, EPOLLOUT>(space->handler_, space->cache_, handler, func);
}
void wait(Error, const Handler& handler, const Space::Shared& space, Process func) {
    wait<Error::id, EPOLLERR>(space->handler_, space->cache_, handler, func);
}

/// ===============================================================================================
/// Cluster implementation
/// - wait
/// ===============================================================================================
///
/// @brief
///
void wait(const Handler& handler, Cluster::Shared space, Process func) {
    std::cout << __func__ << "" << __LINE__ << std::endl;
}
} // namespace fusion