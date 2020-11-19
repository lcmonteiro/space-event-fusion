#include <chrono>

#include <gtest/gtest.h>

#include "resources/stream/remote.hpp"
#include "resources/timer.hpp"

/// Test
/// @brief
template <typename Self, typename Space>
void process(Self self, Space space) {
    wait<fusion::Input>(self, [](auto self, auto space) {
        build<fusion::Timer>(
          self,
          [](auto self, auto space) { // write(self, data);

          },
          std::chrono::system_clock::now() + std::chrono::seconds{1});
        process(self, space);
    });
}

TEST(resources_stream_remote, positive_test) {
    std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    build<fusion::Space>([](auto self) {
        // server
        build<fusion::stream::remote::Server>(
          self,
          [](auto self, auto space) {
              std::cout << __FILE__ << ":" << __LINE__ << std::endl;
              wait<fusion::input::Connection>(self, [](auto self, auto space) {
                  std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                  function(self, [](auto self, auto process) {
                      wait<fusion::Input>(self, [process](auto self, auto space) {
                          std::string data(100, '\0');
                          read(self, data);
                          std::cout << "rx= " << data << std::endl;
                          build<fusion::Timer>(
                            self,
                            [data](auto self, auto space) {
                                wait<fusion::Input>(self, [data](auto self, auto space) {
                                    write(space, data + "s");
                                });
                            },
                            std::chrono::system_clock::now() + std::chrono::seconds{1});
                          function(self, process);
                      });
                  });
              });
          },
          fusion::stream::remote::Address{"localhost", 10000});

        // client
        build<fusion::stream::remote::Client>(
          self,
          [](auto self, auto space) {
              std::cout << __FILE__ << ":" << __LINE__ << std::endl;
              wait<fusion::output::Connection>(
                self,
                [](auto self, auto space) {
                    std::cout << __FILE__ << ":" << __LINE__ << std::endl;

                    function(self, [](auto self, auto process) {
                        wait<fusion::Input>(self, [process](auto self, auto space) {
                            std::string data(100, '\0');
                            read(self, data);
                            std::cout << "rx= " << data << std::endl;
                            build<fusion::Timer>(
                              self,
                              [data](auto self, auto space) {
                                  wait<fusion::Input>(self, [data](auto self, auto space) {
                                      write(space, data + "c");
                                  });
                              },
                              std::chrono::system_clock::now() + std::chrono::seconds{1});
                            function(self, process);
                        });
                    });
                    write(self, std::string("c"));
                },
                fusion::stream::remote::Address{"127.0.0.1", 10000});
          },
          fusion::stream::remote::IPv4());
    });
}