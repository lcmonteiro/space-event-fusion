/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file fusion.cpp                                                    _| |  | (_-<  |   _ \    \ 
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#include "fusion.hpp"

#include <error.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <set>
#include <system_error>

namespace fusion {


/// ===============================================================================================
/// Element implementation
/// - handler
/// - constructor
/// - native interface
/// - desctructor
/// ===============================================================================================
struct Element::Handler {
    int native;
    std::tuple<int, int> events;
    std::tuple<Process, Process, Process> binds;
};

Element::Element() : handler_{std::make_shared<Handler>()} {
    handler_->native = -1;
}

template <>
Element::Element(int native) : handler_{std::make_shared<Handler>()} {
    if (native < 0)
        throw std::system_error(std::make_error_code(std::errc(errno)));
    handler_->native = native;
}

template <>
int Element::native() {
    return handler_->native;
}

/// update native handler
/// @brief
/// this method will:
/// - close the current handler,
/// - reset the current events
/// - and save the new handler
template <>
void Element::native(int native) {
    std::get<0>(handler_->events) = 0;
    if (handler_->native)
        close(handler_->native);
    if (native < 0)
        throw std::system_error(std::make_error_code(std::errc(errno)));
    handler_->native = native;
}

/// read native interface
/// @brief
/// this method will:
template <>
void Element::native(std::string& buf) {
    auto count = ::recv(handler_->native, buf.data(), buf.size(), 0);
    if (count <= 0)
        throw std::system_error(std::make_error_code(std::errc(errno)));
    buf.resize(count);
}
template <>
void Element::native(std::vector<std::byte>& buf) {
    auto count = ::recv(handler_->native, buf.data(), buf.size(), 0);
    if (count <= 0)
        throw std::system_error(std::make_error_code(std::errc(errno)));
    buf.resize(count);
}

/// write native interface
/// @brief
/// this method will:
template <>
void Element::native(const std::string& buf) {
    if (::send(handler_->native, buf.data(), buf.size(), MSG_NOSIGNAL) < 0)
        throw std::system_error(std::make_error_code(std::errc(errno)));
}
template <>
void Element::native(const std::vector<std::byte>& buf) {
    if (::send(handler_->native, buf.data(), buf.size(), MSG_NOSIGNAL) < 0)
        throw std::system_error(std::make_error_code(std::errc(errno)));
}

Element::~Element() {
    if (handler_->native)
        close(handler_->native);
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

///
struct Space::Handler {
    int native{-1};
    std::unordered_map<Element::Handler*, Element::Shared> cache;
};

/// Wait
/// @brief
template <int i, int flags, typename Source, typename Base, typename Process>
void await(const Source& source, const Base& base, Process& proc) {
    // update future
    std::get<i>(source->binds)   = std::move(proc);
    std::get<1>(source->events) |= flags;

    // update state
    if (std::get<0>(source->events) != std::get<1>(source->events)) {
        // update element
        base->cache.emplace(source.get(), source);

        // update events
        epoll_event ev;
        ev.events   = std::get<1>(source->events);
        ev.data.ptr = source.get();
        if (std::get<0>(source->events))
            assert(::epoll_ctl(base->native, EPOLL_CTL_MOD, source->native, &ev) == 0);
        else
            assert(::epoll_ctl(base->native, EPOLL_CTL_ADD, source->native, &ev) == 0);

        // update current
        std::get<0>(source->events) = ev.events;
    }
}

/// process
/// @brief
template <int i, int flags, typename Event, typename Source>
static inline void process(const Event& events, Source* source) {
    if (events & flags & std::get<1>(source->events)) {
        // remove
        std::get<1>(source->events) &= ~flags;
        auto callable = std::move(std::get<i>(source->binds));
        try {
            // execute
            callable();
        }
        catch (const exception::Rollback&) {
            // reinsert
            std::get<1>(source->events) |= flags;
            std::get<i>(source->binds) = std::move(callable);
        }
        catch (const std::exception& e) {
            // error
            std::cerr << "space::run:" << e.what() << std::endl;
        }
    }
}

/// cleanup
/// @brief
template <typename Event, typename Source, typename Base>
static inline void cleanup(Event& events, Source* source, Base* handler) {
    if (std::get<1>(source->events) == 0) {
        handler->cache.erase(source);
        return;
    }
    if (events & ERROR) {
        auto fds = std::array{pollfd{.fd = source->native, .events = 0, .revents = 0}};
        if (::poll(fds.data(), fds.size(), 0) < 0 || fds.front().revents) {
            source->binds = std::tuple{nullptr, nullptr, nullptr};
            handler->cache.erase(source);
            return;
        }
    }
    if (std::get<1>(source->events) != std::get<0>(source->events)) {
        epoll_event ev;
        ev.events   = std::get<1>(source->events);
        ev.data.ptr = source;
        assert(::epoll_ctl(handler->native, EPOLL_CTL_MOD, source->native, &ev) == 0);
        std::get<0>(source->events) = events;
    }
}

/// constructor
/// @brief
Space::Space() : handler_{std::make_unique<Handler>()} {
    handler_->native = ::epoll_create1(0);
}

/// destructor
/// @brief
Space::~Space() {
    close(handler_->native);
}

/// run
/// @brief
void Space::run() {
    auto events = std::vector<epoll_event>(10);
    // run until empty cache
    while (!handler_->cache.empty()) {
        // wait for events
        auto count = ::epoll_await(handler_->native, events.data(), events.size(), -1);

        // check error
        if (count < 0)
            throw std::system_error(std::make_error_code(std::errc(errno)));

        // process events
        events.resize(count);
        for (auto& ev : events) {
            auto source = static_cast<Element::Handler*>(ev.data.ptr);
            auto events = ev.events;

            process<0, INPUT>(events, source);
            process<1, OUTPUT>(events, source);
            process<2, ERROR>(events, source);

            cleanup(events, source, handler_.get());
        }
        events.resize(events.capacity());
    }
}

template<>
void Space::decorate(input, const Element::Shared& handler, Process&& func) {
    wait<0, INPUT>(handler, this->handler_, func);
}
template<>
void Space::await(Output, const Element::Shared& handler, Process&& func) {
    wait<1, OUTPUT>(handler, this->handler_, func);
}
template<>
void Space::await(Error, const Element::Shared& handler, Process&& func) {
    wait<2, ERROR>(handler, this->handler_, func);
}

} // namespace fusion
