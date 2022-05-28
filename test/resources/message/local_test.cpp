/// ===============================================================================================
/// @copyright (c) 2020 LCMonteiro                                      _|           _)            
/// @file local_test.cpp                                                _| |  | (_-<  |   _ \    \. 
/// @author Luis Monteiro                                             _|  \_,_| ___/ _| \___/ _| _|
/// @date November 20, 2020        
/// ===============================================================================================
#include <chrono>

#include <gtest/gtest.h>

#include "resources/message/local.hpp"

/// Test
/// @brief
TEST(resources_message_local, positive_test) {
    constexpr auto NBYTES = 1000;

    std::string expect(NBYTES, 'i');

    // process
    std::string data(NBYTES, '\0');
    build<fusion::Space>([&data](auto self) {
        // ping
        build<fusion::message::local::Messenger>(
          self,
          [&data](auto self, auto space) {
              std::ignore = space;
              wait<fusion::output::Connection>(
                self,
                [&data](auto self, auto space) {
                    std::ignore = space;
                    wait_loop<fusion::Input>(self, [&data](auto self, auto space) {
                        std::ignore = space;
                        std::string data;
                        data.resize(NBYTES);
                        read(self, data);
                        write(self, data + "i");
                        return (data.size() < NBYTES);
                    });
                },
                fusion::message::local::Address{"bbbb"});
          },
          fusion::message::local::Address{"aaaa"});

        // pong
        build<fusion::message::local::Messenger>(
          self,
          [&data](auto self, auto space) {
              std::ignore = space;
              wait<fusion::output::Connection>(
                self,
                [&data](auto self, auto space) {
                    std::ignore = space;
                    wait_loop<fusion::Input>(self, [&data](auto self, auto space) {
                        std::ignore = space;
                        data.resize(NBYTES);
                        read(self, data);
                        write(self, data + "i");
                        return (data.size() < NBYTES);
                    });
                    write(self, std::string("i"));
                },
                fusion::message::local::Address{"aaaa"});
          },
          fusion::message::local::Address{"bbbb"});
    });

    // check
    EXPECT_EQ(expect, data);
}
