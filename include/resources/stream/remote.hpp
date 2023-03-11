/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file remote.hpp                                                    _| |  | (_-<  |   _ \    \.
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#pragma once

#include <string>
#include <tuple>

#include "fusion.hpp"

namespace fusion::stream::remote {

using Address = std::tuple<std::string, std::uint16_t>;
        /// =======================================================================================
        /// server
        /// @brief
        /// =======================================================================================
        class Server : public Element {
          public:
            using Shared   = std::shared_ptr<Server>;

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
            /// decorate with connection process 
            /// @brief
            friend Process decorate(input::Connection, Process&&, Server&);

  protected:
    /// build connection process
    /// @brief
    friend Process build(input::Connection, Server&, Process);

    /// read
    /// @brief
    friend void read(Shared self, String& buf);
    friend void read(Shared self, Buffer& buf);

    /// write
    /// @brief
    friend void write(Shared self, const Buffer& buf);
    friend void write(Shared self, const String& str);
};


/// =======================================================================================
/// Client
/// @brief
/// =======================================================================================
class Client : public Element {
  public:
    using Shared = std::shared_ptr<Client>;
            /// write
            /// @brief
            friend void write(Shared self, const Buffer& buf);
            friend void write(Shared self, const String& str);
        };
        

        /// =======================================================================================
        /// Client
        /// @brief
        /// =======================================================================================
        class Client : public Element {
          public:
            using Shared   = std::shared_ptr<Client>;

          protected:
            /// wait connection
            /// @brief
            /// @param remote address
            friend Process decorate(output::Connection, Process&&, Client&, const Address& remote);

  protected:
    /// build connection process
    /// @brief
    /// @param remote address
    friend Process build(output::Connection, Client&, Process, const Address& remote);

    /// read
    /// @brief
    friend void read(Shared self, String& buf);
    friend void read(Shared self, Buffer& buf);

    /// write
    /// @brief
    friend void write(Shared self, const Buffer& buf);
    friend void write(Shared self, const String& str);
};
} // namespace fusion::stream::remote
