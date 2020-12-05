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
#include <sys/poll.h>
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
        throw std::system_error(std::make_error_code(std::errc(errno)));
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

/// definitions
constexpr int INPUT  = EPOLLIN;
constexpr int OUTPUT = EPOLLOUT;
constexpr int ERROR  = EPOLLERR | EPOLLHUP;

/// cache
/// @brief
struct Space::Cache {
    struct Resource {
        struct State {
            int curr{0};
            int next{0};
        } state;
        std::map<int, Process> callables;
    };
    std::unordered_map<int, Resource> resources;
};


/// constructor
/// @brief
Space::Space() : handler_{::epoll_create1(0)}, cache_{std::make_unique<Cache>()} {}

/// destructor
/// @brief
Space::~Space() = default;

/// run
/// @brief
void Space::run() {
    auto events     = std::vector<epoll_event>(10);
    auto& resources = cache_->resources;

    // run until empty cache
    while (!resources.empty()) {
        // wait for events
        auto count = ::epoll_wait(handler_.native(), events.data(), events.size(), -1);

        // check error
        if (count < 0)
            throw std::system_error(std::make_error_code(std::errc(errno)));

        // process events
        constexpr auto EVENT_MAP = std::array{
          std::tuple{Input::id, INPUT},
          std::tuple{Output::id, OUTPUT},
          std::tuple{Error::id, ERROR}};
        events.resize(count);
        for (auto& ev : events) {
            // find resource cached processes
            auto& resource = resources[ev.data.fd];
            // process resource events
            for (auto& [id, mask] : EVENT_MAP) {
                if (ev.events & mask & resource.state.next) {
                    // remove
                    resource.state.next &= ~mask;
                    auto node = resource.callables.extract(id);
                    try {
                        // execute
                        node.mapped()();
                    }
                    catch (const exception::Continue&) {
                        // reinsert
                        resource.state.next |= mask;
                        resource.callables.insert(std::move(node));
                    }
                    catch (const std::exception& e) {
                        std::cerr << "space::run:" << e.what() << std::endl;
                    }
                }
            }
            // no callables
            if (resource.state.next == 0) {
                resources.erase(ev.data.fd);
                continue;
            }
            // error handling
            if (ev.events & ERROR) {
                auto fds = std::array{pollfd{.fd = ev.data.fd, .events = 0, .revents = 0}};
                if (::poll(fds.data(), fds.size(), 0) < 0 || fds.front().revents) {
                    resources.erase(ev.data.fd);
                    continue;
                }
            }
            // sync events
            if (resource.state.next != resource.state.curr) {
                ev.events = resource.state.next;
                assert(::epoll_ctl(handler_.native(), EPOLL_CTL_MOD, ev.data.fd, &ev) == 0);
                resource.state.curr = ev.events;
            }
        }
        events.resize(events.capacity());
    }
}

/// Wait
/// @brief
template <int id, int flags, typename Cache, typename Process>
void wait(const Handler& base, Cache& cache, const Handler& source, Process& proc) {
    auto& resource = cache->resources[source.native()];
    // update next
    resource.callables[id] = proc;
    resource.state.next |= flags;

    // check current state
    if (resource.state.curr != resource.state.next) {
        // update event triggers
        epoll_event ev;
        ev.events  = resource.state.next;
        ev.data.fd = source.native();
        if (resource.state.curr)
            assert(::epoll_ctl(base.native(), EPOLL_CTL_MOD, ev.data.fd, &ev) == 0);
        else
            assert(::epoll_ctl(base.native(), EPOLL_CTL_ADD, ev.data.fd, &ev) == 0);
        // update current
        resource.state.curr = ev.events;
    }
}
void wait(Input, const Handler& handler, const Space::Shared& space, Process func) {
    wait<Input::id, INPUT>(space->handler_, space->cache_, handler, func);
}
void wait(Output, const Handler& handler, const Space::Shared& space, Process func) {
    wait<Output::id, OUTPUT>(space->handler_, space->cache_, handler, func);
}
void wait(Error, const Handler& handler, const Space::Shared& space, Process func) {
    wait<Error::id, ERROR>(space->handler_, space->cache_, handler, func);
}

} // namespace fusion
