/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)            
/// @file remote.hpp                                                    _| |  | (_-<  |   _ \    \ 
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020        
/// ===============================================================================================
#pragma once

#include <tuple>
#include <string>

#include "fusion.hpp"

namespace fusion {
namespace stream {
    namespace remote {
        using Address = std::tuple<std::string, std::uint16_t>;

        /// =======================================================================================
        /// server
        /// @brief
        /// =======================================================================================
        class Server : public Element {
          public:
            using Shared = std::shared_ptr<Server>;
            /// constructor
            /// @param local address
            Server(const Address& local);

          protected:
            /// wait connection
            /// @brief
            /// @param remote address
            friend Process wait(input::Connection, Server&, Process proc);

            /// read
            /// @brief
            friend void read(Shared self, std::string& str);

            /// write
            /// @brief
            friend void write(Shared self, const std::string& str);
        };


        /// =======================================================================================
        /// Client
        /// @brief
        /// =======================================================================================
        template <int n>
        struct Domain {};
        using IPv6 = Domain<6>;
        using IPv4 = Domain<4>;
        class Client : public Element {
          public:
            using Shared = std::shared_ptr<Client>;
            /// constructor
            /// @brief create 
            /// @param domain IPv4 or IPv6
            template <typename Domain>
            Client(Domain);

          protected:
            /// wait connection
            /// @brief
            /// @param remote address
            friend Process wait(output::Connection, Client&, Process proc, const Address& remote);

            /// read
            /// @brief
            friend void read(Shared self, std::string& str);

            /// write
            /// @brief
            friend void write(Shared self, const std::string& str);
        };
    } // namespace remote
} // namespace stream
} // namespace fusion
/// ===============================================================================================
/// ===============================================================================================
