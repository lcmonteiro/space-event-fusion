#include <chrono>

#include <gtest/gtest.h>

#include "resources/stream/remote.hpp"
#include "resources/timer.hpp"

/// Test
/// @brief
TEST(resources_stream_remote, positive_test) {
    constexpr auto NBYTES = 1000;

    std::string expect(NBYTES, 'c');
    std::transform(
      expect.begin(),
      std::prev(expect.end()),
      std::next(expect.begin()),
      std::next(expect.begin()),
      [](auto a, auto b) { return a != b ? 'c' : 's'; });

    // process
    std::string data(NBYTES, '\0');
    build<fusion::Space>([&data](auto self) {
        // server
        build<fusion::stream::remote::Server>(
          self,
          [&data](auto self, auto space) {
              std::ignore = space;
              wait<fusion::input::Connection>(self, [&data](auto self, auto space) {
                  std::ignore = space;
                  wait_loop<fusion::Input>(self, [&data](auto self, auto space) {
                      std::ignore = space;
                      data.resize(NBYTES);
                      read(self, data);
                      write(self, data + "s");
                      return (data.size() < NBYTES);
              await<fusion::input::Connection>(self, [&data](auto self, auto space) {
                  call(self, [&data](auto self, auto callback) {
                      await<fusion::Input>(self, [&data, callback](auto self, auto space) {
                          data.resize(NBYTES);
                          read(self, data);
                          write(self, data + "s");

                          if (data.size() < NBYTES)
                              call(self, callback);
                      });
                  });
              });
          },
          fusion::stream::remote::Address{"localhost", 10000});

        // client
        build<fusion::stream::remote::Client>(self, [&data](auto self, auto space) {
            std::ignore = space;
            wait<fusion::output::Connection>(
              self,
              [&data](auto self, auto space) {
                  std::ignore = space;
                  wait_loop<fusion::Input>(self, [&data](auto self, auto space) {
                      std::ignore = space;
                      data.resize(NBYTES);
                      read(self, data);
                      write(self, data + "c");
                      return (data.size() < NBYTES);
            await<fusion::output::Connection>(
              self,
              [&data](auto self, auto space) {
                  call(self, [&data](auto self, auto callback) {
                      await<fusion::Input>(self, [&data, callback](auto self, auto space) {
                          data.resize(NBYTES);
                          read(self, data);
                          write(self, data + "c");
                          if (data.size() < NBYTES)
                              call(self, callback);
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