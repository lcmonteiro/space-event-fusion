/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)
/// @file local.hpp                                                     _| |  | (_-<  |   _ \    \.
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020
/// ===============================================================================================
#pragma once

#include <vector>

#include "fusion.hpp"

namespace fusion::message::local {

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
    Messenger(const Address& source);

  protected:
    /// build connection process
    /// @brief
    /// @param remote address
    friend Process build(output::Connection, Messenger&, Process, const Address& destination);

    /// read
    /// @brief
    friend void read(Shared self, Buffer& buf);
    friend void read(Shared self, std::string& buf);

    /// write
    /// @brief
    friend void write(Shared self, const Buffer& buf);
    friend void write(Shared self, const std::string& buf);
};

} // namespace fusion::message::local
