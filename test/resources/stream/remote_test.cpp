#include <chrono>

#include <gtest/gtest.h>

#include "resources/stream/remote.hpp"

/// Test
/// @brief
TEST(resources_stream_remote, positive_test) {
    std::cerr << __FILE__ << __LINE__ << std::endl;
    build<fusion::Space>([](auto self) {
        std::cout << __FILE__ << __LINE__ << std::endl;
        build<fusion::stream::remote::Client>(
          self,
          [](auto self, auto space) {
              std::cout << __FILE__ << __LINE__ << std::endl;
              wait<fusion::Connection>(
                self,
                [](auto self, auto space) { std::cout << __FILE__ << __LINE__ << std::endl; },
                std::string{"test"});
          },
          std::string{""});
    });
}