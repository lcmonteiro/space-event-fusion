#include <chrono>

#include <gtest/gtest.h>

#include "resources/stream/remote.hpp"
#include "resources/timer.hpp"

/// Test
/// @brief
TEST(resources_stream_remote, positive_test) {
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
        // server
        build<fusion::stream::remote::Server>(
          self,
          [&data](auto self, auto space) {
              wait<fusion::input::Connection>(self, [&data](auto self, auto space) {
                  function(self, [&data](auto self, auto process) {
                      wait<fusion::Input>(self, [&data, process](auto self, auto space) {
                          data.resize(data.capacity());
                          read(self, data);
                          write(self, data + "s");

                          if (data.size() < 100)
                              function(self, process);
                      });
                  });
              });
          },
          fusion::stream::remote::Address{"localhost", 10000});

        // client
        build<fusion::stream::remote::Client>(self, [&data](auto self, auto space) {
            wait<fusion::output::Connection>(
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
              fusion::stream::remote::Address{"127.0.0.1", 10000});
        });
    });

    // check
    EXPECT_EQ(expect, data);
}