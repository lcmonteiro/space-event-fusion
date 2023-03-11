#include <chrono>

#include <gtest/gtest.h>

#include "resources/stream/remote.hpp"
#include "resources/timer.hpp"

/// Test
/// @brief
TEST(fusion_space, positive_test) {
    build<fusion::Space>([](auto self) {
        // server
        build<fusion::stream::remote::Server>(
          self,
          [](auto self, auto space) {
              std::ignore = space;
              wait<fusion::input::Connection>(self, [](auto self, auto space) {
                  std::ignore = space;
                  wait_loop<fusion::Input>(self, [](auto self, auto space) {
                      std::ignore = space;
                      std::string data(100, '\0');
                      read(self, data);
                      std::cout << "rx= " << data << std::endl;
                      build<fusion::Timer>(
                        self,
                        [data](auto self, auto space) {
                            std::ignore = space;
                            wait<fusion::Input>(self, [data](auto self, auto space) {
                                std::ignore = self;
                                write(space, data + "s");
                            });
                        },
                        std::chrono::system_clock::now() + std::chrono::milliseconds{10});
                      return (data.size() < data.capacity());
              await<fusion::input::Connection>(self, [](auto self, auto space) {
                  call(self, [](auto self, auto callback) {
                      await<fusion::Input>(self, [callback](auto self, auto space) {
                          std::string data(100, '\0');
                          read(self, data);
                          std::cout << "rx= " << data << std::endl;
                          build<fusion::Timer>(
                            self,
                            [data](auto self, auto space) {
                                await<fusion::Input>(self, [data](auto self, auto space) {
                                    write(space, data + "s");
                                });
                            },
                            std::chrono::system_clock::now() + std::chrono::milliseconds{10});
                          if (data.size() < data.capacity())
                              call(self, callback);
                      });
                  });
              });
          },
          fusion::stream::remote::Address{"localhost", 10000});

        // client
        build<fusion::stream::remote::Client>(self, [](auto self, auto space) {
            std::ignore = space;
            wait<fusion::output::Connection>(
              self,
              [](auto self, auto space) {
                  std::ignore = space;
                  wait_loop<fusion::Input>(self, [](auto self, auto space) {
                      std::ignore = space;
                      std::string data(100, '\0');
                      read(self, data);
                      std::cout << "rx= " << data << std::endl;
                      build<fusion::Timer>(
                        self,
                        [data](auto self, auto space) {
                            std::ignore = space;
                            wait<fusion::Input>(self, [data](auto self, auto space) {
                                std::ignore = self;
                                write(space, data + "c");
                            });
                        },
                        std::chrono::system_clock::now() + std::chrono::milliseconds{20});
                      return (data.size() < data.capacity());
            await<fusion::output::Connection>(
              self,
              [](auto self, auto space) {
                  call(self, [](auto self, auto callback) {
                      await<fusion::Input>(self, [callback](auto self, auto space) {
                          std::string data(100, '\0');
                          read(self, data);
                          std::cout << "rx= " << data << std::endl;
                          build<fusion::Timer>(
                            self,
                            [data](auto self, auto space) {
                                await<fusion::Input>(self, [data](auto self, auto space) {
                                    write(space, data + "c");
                                });
                            },
                            std::chrono::system_clock::now() + std::chrono::milliseconds{20});
                          if (data.size() < data.capacity())
                              call(self, callback);
                      });
                  });
                  write(self, std::string("c"));
              },
              fusion::stream::remote::Address{"127.0.0.1", 10000});
        });
    });
}