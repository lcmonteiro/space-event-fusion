/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file remote.cpp                                                    _| |  | (_-<  |   _ \    \ 
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#include "resources/message/local.hpp"

#include <cstring>
#include <memory>
#include <stdexcept>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace fusion {
namespace message {
    namespace local {
        /// =======================================================================================
        /// Messenger
        /// - constructor
        /// - read 
        /// - write
        /// =======================================================================================
        Messenger::Messenger(const Address& source, const Address& destination)
          : Element([&source, &destination] {
                sockaddr_un addr;

                // create a messanger handler
                auto h = Handler(::socket(PF_LOCAL, SOCK_DGRAM | SOCK_NONBLOCK, 0));

                // bind source
                addr.sun_family  = PF_LOCAL;
                addr.sun_path[0] = '\0';
                std::strncpy(&addr.sun_path[1], source.data(), sizeof(addr.sun_path) - 2);
                if (::bind(h.native(), (sockaddr*)&addr, sizeof(addr)) < 0)
                    throw std::system_error(std::make_error_code(std::errc(errno)));

                // connect to destination
                addr.sun_family  = PF_LOCAL;
                addr.sun_path[0] = '\0';
                std::strncpy(&addr.sun_path[1], destination.data(), sizeof(addr.sun_path) - 2);
                if (::connect(h.native(), (sockaddr*)&addr, sizeof(addr)) < 0)
                    throw std::system_error(std::make_error_code(std::errc(errno)));

                // return handler
                return h;
            }()) {}


        void read(Messenger::Shared self, std::string& str) {
            auto count = ::recv(self->handler_.native(), str.data(), str.size(), 0);
            if (count <= 0)
                throw std::system_error(std::make_error_code(std::errc(errno)));
            str.resize(count);
        }

        void write(Messenger::Shared self, const std::string& str) {
            if (::send(self->handler_.native(), str.data(), str.size(), MSG_NOSIGNAL) < 0)
                throw std::system_error(std::make_error_code(std::errc(errno)));
        }

    } // namespace local
} // namespace message
} // namespace fusion