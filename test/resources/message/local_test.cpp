#include <chrono>

#include <gtest/gtest.h>

#include "resources/message/local.hpp"


/// Test
/// @brief
TEST(resources_message_local, positive_test) {
    constexpr auto NBYTES = 10000;

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
                    call(self, [&data](auto self, auto scope) {
                        wait<fusion::Input>(scope, [&data, callable = self](auto self, auto space) {
                            data.resize(data.capacity());
                            read(self, data);
                            write(self, data + "i");
                            if (data.size() < data.capacity())
                                call(self, callable);
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
                    call(self, [&data](auto self, auto scope) {
                        wait<fusion::Input>(scope, [&data, callable = self](auto self, auto space) {
                            data.resize(data.capacity());
                            read(self, data);
                            write(self, data + "i");
                            if (data.size() < data.capacity())
                                call(self, callable);
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
