/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file remote.hpp                                                    _| |  | (_-<  |   _ \    \ 
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#pragma once

#include <string>
#include <tuple>

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
            using Shared   = std::shared_ptr<Server>;
            using Callback = std::function<void()>;

            /// constructor
            /// @param local address
            Server(const Address& local);

          protected:
            /// wait connection
            /// @brief
            friend Process wait(input::Connection, Server&, Callback);

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
        class Client : public Element {
          public:
            using Shared   = std::shared_ptr<Client>;
            using Callback = std::function<void()>;

          protected:
            /// wait connection
            /// @brief
            /// @param remote address
            friend Process wait(output::Connection, Client&, Callback, const Address& remote);

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
