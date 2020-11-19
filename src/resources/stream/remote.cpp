
#include "resources/stream/remote.hpp"

#include <memory>
#include <stdexcept>

#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace fusion {
namespace stream {
    namespace remote {
        /// =======================================================================================
        /// server
        /// =======================================================================================
        Server::Server(const Address& local)
          : Element([&local]() {
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
                    auto h = Handler(
                      ::socket(rp->ai_family, rp->ai_socktype | SOCK_NONBLOCK, rp->ai_protocol));

                    // set options
                    int opt = 1;
                    if (::setsockopt(h.native(), SOL_SOCKET, SO_REUSEADDR, (int*)&opt, sizeof(int)) < 0)
                        continue;
                    if (::setsockopt(h.native(), SOL_SOCKET, SO_REUSEPORT, (int*)&opt, sizeof(int)) < 0)
                        continue;

                    // bind address
                    if (::bind(h.native(), rp->ai_addr, rp->ai_addrlen) < 0)
                        continue;

                    // listen just one connection
                    if (::listen(h.native(), 1) < 0)
                        continue;

                    return h;
                }
                // fallback
                throw std::system_error(std::make_error_code(std::errc::no_such_device_or_address));
            }()) {}


        Process wait(input::Connection, Server& self, Process proc) {
            // after input waitting
            return [&self, process = std::move(proc)] {
                // accept & update handler
                self.handler = Handler(::accept(self.handler.native(), NULL, NULL));
                // call the register process
                process();
            };
        }


        void read(Server::Shared self, std::string& str) {
            auto count = ::recv(self->handler.native(), str.data(), str.size(), 0);
            if (count <= 0)
                throw std::system_error(std::make_error_code(std::errc(errno)));
            str.resize(count);
        }

        void write(Server::Shared self, const std::string& str) {
            if (::send(self->handler.native(), str.data(), str.size(), MSG_NOSIGNAL) < 0)
                throw std::system_error(std::make_error_code(std::errc(errno)));
        }


        /// =======================================================================================
        /// client
        /// =======================================================================================
        template <>
        Client::Client(IPv4) : Element(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) {}
        template <>
        Client::Client(IPv6) : Element(::socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0)) {}

        Process wait(output::Connection, Client& self, Process proc, const Address& remote) {
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
                // try connect
                auto res = ::connect(self.handler.native(), rp->ai_addr, rp->ai_addrlen);

                // check result
                if (res != 0 && errno != EINPROGRESS)
                    continue;

                // run after input waitting
                return [native = self.handler.native(), process = std::move(proc)] {
                    // check connection
                    auto result = int(0);
                    auto length = socklen_t(sizeof(result));
                    if (getsockopt(native, SOL_SOCKET, SO_ERROR, &result, &length) < 0 || result != 0)
                        throw std::system_error(std::make_error_code(std::errc::broken_pipe));

                    // call the register process
                    process();
                };
            }
            // fallback
            throw std::system_error(std::make_error_code(std::errc::no_such_device_or_address));
        }

        void read(Client::Shared self, std::string& str) {
            auto count = ::recv(self->handler.native(), str.data(), str.size(), 0);
            if (count <= 0)
                throw std::system_error(std::make_error_code(std::errc(errno)), "client::read");
            str.resize(count);
        }

        void write(Client::Shared self, const std::string& str) {
            if (::send(self->handler.native(), str.data(), str.size(), MSG_NOSIGNAL) < 0)
                throw std::system_error(std::make_error_code(std::errc(errno)), "client::write");
        }
    } // namespace remote
} // namespace stream
} // namespace fusion