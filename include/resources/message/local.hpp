/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file local.hpp                                                     _| |  | (_-<  |   _ \    \ 
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#pragma once

#include "fusion.hpp"

namespace fusion {
namespace message {
    namespace local {
        using Address = std::string;

        /// =======================================================================================
        /// Messenger
        /// @brief
        /// =======================================================================================
        class Messenger : public Element {
          public:
            using Shared = std::shared_ptr<Messenger>;

            /// constructor
            /// @param local address
            Messenger(const Address& source, const Address& destination);

          protected:
            /// read
            /// @brief
            friend void read(Shared self, std::string& str);

            /// write
            /// @brief
            friend void write(Shared self, const std::string& str);
        };
    } // namespace local
} // namespace message
} // namespace fusion
