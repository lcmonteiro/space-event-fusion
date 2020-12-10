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
              wait<fusion::output::Connection>(
                self,
                [&data](auto self, auto space) {
                    call(self, [&data](auto self, auto callback) {
                        wait<fusion::Input>(self, [&data, callback](auto self, auto space) {
                            std::string data;
                            data.resize(NBYTES);
                            read(self, data);
                            write(self, data + "i");
                            if (data.size() < NBYTES)
                                call(self, callback);
                        });
                    });
                },
                fusion::message::local::Address{"bbbb"});
          },
          fusion::message::local::Address{"aaaa"});

        // pong
        build<fusion::message::local::Messenger>(
          self,
          [&data](auto self, auto space) {
              wait<fusion::output::Connection>(
                self,
                [&data](auto self, auto space) {
                    call(self, [&data](auto self, auto callback) {
                        wait<fusion::Input>(
                          self, [&data, callback = callback](auto self, auto space) {
                              data.resize(NBYTES);
                              read(self, data);
                              write(self, data + "i");
                              if (data.size() < NBYTES)
                                  call(self, callback);
                          });
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
