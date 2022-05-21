/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file remote.cpp                                                    _| |  | (_-<  |   _ \    \. 
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
        Messenger::Messenger(const Address& source)
          : Element{::socket(PF_LOCAL, SOCK_DGRAM | SOCK_NONBLOCK, 0)} {
            struct sockaddr_un addr;
            std::memset(&addr, 0, sizeof(addr));
            // bind source
            addr.sun_family  = PF_LOCAL;
            addr.sun_path[0] = '\0';
            std::strncpy(&addr.sun_path[1], source.data(), sizeof(addr.sun_path) - 2);
            if (::bind(native<int>(), (struct sockaddr*)&addr, sizeof(addr)) < 0)
                throw std::system_error(std::make_error_code(std::errc(errno)));
        }

        Process
        wait(output::Connection, Messenger& self, Callback<> callable, const Address& destination) {
            return [native = self.native<int>(), callable = std::move(callable), destination] {
                struct sockaddr_un addr;
                std::memset(&addr, 0, sizeof(addr));
                // connect to destination
                addr.sun_family  = PF_LOCAL;
                addr.sun_path[0] = '\0';
                std::strncpy(&addr.sun_path[1], destination.data(), sizeof(addr.sun_path) - 2);
                if (::connect(native, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                    throw std::system_error(std::make_error_code(std::errc(errno)));
                // back to main function
                callable();
            };
        }

        void read(Messenger::Shared s, Buffer& b) { s->native<void, Buffer&>(b); }
        void read(Messenger::Shared s, String& b) { s->native<void, String&>(b); }
        
        void write(Messenger::Shared s, const Buffer& b) { s->native<void, const Buffer&>(b); }
        void write(Messenger::Shared s, const String& b) { s->native<void, const String&>(b); }


    } // namespace local
} // namespace message
} // namespace fusion