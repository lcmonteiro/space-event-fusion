#include <chrono>

#include <gtest/gtest.h>

#include "resources/message/local.hpp"

/// Test
/// @brief
TEST(resources_message_local, positive_test) {
    std::string expect(100, 'c');
    std::transform(
      expect.begin(),
      std::prev(expect.end()),
      std::next(expect.begin()),
      std::next(expect.begin()),
      [](auto a, auto b) { return a != b ? 'c' : 's'; });

    // process
    std::string data(100, '\0');
    build<fusion::Space>([&data](auto self) {
        // ping
        build<fusion::message::local::Messenger>(
          self,
          [&data](auto self, auto space) {
              function(self, [&data](auto self, auto process) {
                  wait<fusion::Input>(self, [&data, process](auto self, auto space) {
                      data.resize(data.capacity());
                      read(self, data);
                      write(self, data + "s");

                      if (data.size() < 100)
                          function(self, process);
                  });
              });
          },
          fusion::message::local::Address{"aaaa"},
          fusion::message::local::Address{"bbbb"});

        // pong
        build<fusion::message::local::Messenger>(
          self,
          [&data](auto self, auto space) {
              function(self, [&data](auto self, auto process) {
                  wait<fusion::Input>(self, [&data, process](auto self, auto space) {
                      data.resize(data.capacity());
                      read(self, data);
                      write(self, data + "c");
                      if (data.size() < 100)
                          function(self, process);
                  });
              });
              write(self, std::string("c"));
          },
          fusion::message::local::Address{"bbbb"},
          fusion::message::local::Address{"aaaa"});
    });

    // check
    EXPECT_EQ(expect, data);
}