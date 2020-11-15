#pragma once

#include <chrono>

#include "fusion.hpp"

namespace fusion {
namespace stream {
    namespace remote {
        class Client : public Element {
          public:
            /// constructor
            /// @param local address
            Client(const std::string& local);

          protected:
            /// wait connection
            /// @brief
            /// @param remote address
            friend Process wait(Connection, Client&, Process proc, const std::string& remote);
        };
    } // namespace remote
} // namespace stream
} // namespace fusion