/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file remote.cpp                                                    _| |  | (_-<  |   _ \    \.
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#include "resources/stream/remote.hpp"

#include <memory>
#include <stdexcept>

#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace fusion::stream::remote {

/// ===============================================================================================
/// server
/// ===============================================================================================
Server::Server(const Address& local) {
    // bind parameters
    addrinfo hints;
    hints.ai_family    = AF_UNSPEC;
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_flags     = AI_PASSIVE;
    hints.ai_protocol  = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;

    // get address information
    addrinfo* result;
    auto& [host, port] = local;
    if (::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0)
        throw std::system_error(std::make_error_code(std::errc(errno)), "server::getaddrinfo");

    // pointer guard
    std::unique_ptr<addrinfo, void (*)(addrinfo*)> guard(result, freeaddrinfo);

    // find address
    for (auto rp = result; rp != NULL; rp = rp->ai_next) {
        // create a server handler
        native<void>(::socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol));

        // set options
        int opt = 1;
        if (::setsockopt(native<int>(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
            continue;
        if (::setsockopt(native<int>(), SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int)) < 0)
            continue;

        // bind address
        if (::bind(native<int>(), rp->ai_addr, rp->ai_addrlen) < 0)
            continue;

        // listen just one connection
        if (::listen(native<int>(), 1) < 0)
            continue;

        return;
    }
    // fallback
    throw std::system_error(std::make_error_code(std::errc::no_such_device_or_address));
}

Process build(input::Connection, Server& self, Process callback) {
    // after input waitting
    return [&self, callback = std::move(callback)] {
        // accept & update handler
        self.native<void>(::accept(self.native<int>(), NULL, NULL));
        // back to main function
        callback();
    };
}

void read(Server::Shared self, Buffer& b) { self->native<void, Buffer&>(b); }
void read(Server::Shared self, String& s) { self->native<void, String&>(s); }

void write(Server::Shared self, const Buffer& b) { self->native<void, const Buffer&>(b); }
void write(Server::Shared self, const String& s) { self->native<void, const String&>(s); }


/// ===============================================================================================
/// client
/// ===============================================================================================
Process build(output::Connection, Client& self, Process callback, const Address& remote) {
    auto& [host, port] = remote;

    // bind parameters
    addrinfo hints;
    hints.ai_family    = AF_UNSPEC;
    hints.ai_socktype  = SOCK_STREAM;
    hints.ai_flags     = AI_PASSIVE;
    hints.ai_protocol  = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr      = NULL;
    hints.ai_next      = NULL;

    // address information
    addrinfo* result;
    if (::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0)
        throw std::system_error(std::make_error_code(std::errc(errno)), "client::getaddrinfo");

    // pointer guard
    std::unique_ptr<addrinfo, void (*)(addrinfo*)> guard(result, freeaddrinfo);

    // find address
    for (auto rp = result; rp != NULL; rp = rp->ai_next) {
        // create a client handler
        self.native<void>(
          ::socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol));

        // try connect
        auto res = ::connect(self.native<int>(), rp->ai_addr, rp->ai_addrlen);

        // check result
        if (res != 0 && errno != EINPROGRESS)
            continue;

        // run after input waitting
        return [&self, callback = std::move(callback)] {
            // check connection
            auto result = int(0);
            auto length = socklen_t(sizeof(result));
            if (
              getsockopt(self.native<int>(), SOL_SOCKET, SO_ERROR, &result, &length) < 0
              || result != 0)
                throw std::system_error(std::make_error_code(std::errc::broken_pipe));

            // back to main function
            callback();
        };
    }
    // fallback
    throw std::system_error(std::make_error_code(std::errc::no_such_device_or_address));
}

void read(Client::Shared self, Buffer& b) { self->native<void, Buffer&>(b); }
void read(Client::Shared self, String& s) { self->native<void, String&>(s); }

void write(Client::Shared self, const Buffer& b) { self->native<void, const Buffer&>(b); }
void write(Client::Shared self, const String& s) { self->native<void, const String&>(s); }

} // namespace fusion::stream::remote
namespace fusion {
namespace stream {
    namespace remote {
        /// =======================================================================================
        /// server
        /// =======================================================================================
        Server::Server(const Address& local) {
            // bind parameters
            addrinfo hints;
            hints.ai_family    = AF_UNSPEC;
            hints.ai_socktype  = SOCK_STREAM;
            hints.ai_flags     = AI_PASSIVE;
            hints.ai_protocol  = 0;
            hints.ai_canonname = NULL;
            hints.ai_addr      = NULL;
            hints.ai_next      = NULL;

            // get address information
            addrinfo* result;
            auto& [host, port] = local;
            if (::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0)
                throw std::system_error(
                  std::make_error_code(std::errc(errno)), "server::getaddrinfo");

            // pointer guard
            std::unique_ptr<addrinfo, void (*)(addrinfo*)> guard(result, freeaddrinfo);

            // find address
            for (auto rp = result; rp != NULL; rp = rp->ai_next) {
                // create a server handler
                native<void>(
                  ::socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol));

                // set options
                int opt = 1;
                if (::setsockopt(native<int>(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) < 0)
                    continue;
                if (::setsockopt(native<int>(), SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int)) < 0)
                    continue;

                // bind address
                if (::bind(native<int>(), rp->ai_addr, rp->ai_addrlen) < 0)
                    continue;

                // listen just one connection
                if (::listen(native<int>(), 1) < 0)
                    continue;

                return;
            }
            // fallback
            throw std::system_error(std::make_error_code(std::errc::no_such_device_or_address));
        }


        Process decorate(input::Connection, Process&& upstream, Server& self) {
            // after input waitting
            return [&self, upstream = std::move(upstream)] {
                // accept & update handler
                self.native<void>(::accept(self.native<int>(), NULL, NULL));
                // back to main function
                upstream();
            };
        }

        void read(Server::Shared self, Buffer& b) { self->native<void, Buffer&>(b); }
        void read(Server::Shared self, String& s) { self->native<void, String&>(s); }

        void write(Server::Shared self, const Buffer& b) { self->native<void, const Buffer&>(b); }
        void write(Server::Shared self, const String& s) { self->native<void, const String&>(s); }


        /// =======================================================================================
        /// client
        /// =======================================================================================
        Process
        decorate(output::Connection, Process&& upstream, Client& self, const Address& remote) {
            auto& [host, port] = remote;

            // bind parameters
            addrinfo hints;
            hints.ai_family    = AF_UNSPEC;
            hints.ai_socktype  = SOCK_STREAM;
            hints.ai_flags     = AI_PASSIVE;
            hints.ai_protocol  = 0;
            hints.ai_canonname = NULL;
            hints.ai_addr      = NULL;
            hints.ai_next      = NULL;

            // address information
            addrinfo* result;
            if (::getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0)
                throw std::system_error(
                  std::make_error_code(std::errc(errno)), "client::getaddrinfo");

            // pointer guard
            std::unique_ptr<addrinfo, void (*)(addrinfo*)> guard(result, freeaddrinfo);

            // find address
            for (auto rp = result; rp != NULL; rp = rp->ai_next) {
                // create a client handler
                self.native<void>(
                  ::socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol));

                // try connect
                auto res = ::connect(self.native<int>(), rp->ai_addr, rp->ai_addrlen);

                // check result
                if (res != 0 && errno != EINPROGRESS)
                    continue;

                // run after input waitting
                return [&self, upstream = std::move(upstream)] {
                    // check connection
                    auto result = int(0);
                    auto length = socklen_t(sizeof(result));
                    if (
                      getsockopt(self.native<int>(), SOL_SOCKET, SO_ERROR, &result, &length) < 0
                      || result != 0)
                        throw std::system_error(std::make_error_code(std::errc::broken_pipe));

                    // back to main function
                    upstream();
                };
            }
            // fallback
            throw std::system_error(std::make_error_code(std::errc::no_such_device_or_address));
        }


        void read(Client::Shared self, Buffer& b) { self->native<void, Buffer&>(b); }
        void read(Client::Shared self, String& s) { self->native<void, String&>(s); }

        void write(Client::Shared self, const Buffer& b) { self->native<void, const Buffer&>(b); }
        void write(Client::Shared self, const String& s) { self->native<void, const String&>(s); }

    } // namespace remote
} // namespace stream
} // namespace fusion
