#include <chrono>

#include <gtest/gtest.h>

#include "resources/timer.hpp"


/// Test
/// @brief
TEST(resource_timer, positive_test) {
    // process
    auto life = 10;
    build<fusion::Space>([&life](auto self) {
        // base timer
        build<fusion::Timer>(
          self,
          [&life](auto self, auto space) {
              std::ignore = space;
              wait_loop<fusion::Input>(self, [&life](auto self, auto space) {
                  std::ignore = space;
                  clear(self);
                  return life?++life:0;
              });
              // top timer
              build<fusion::Timer>(
                self,
                [&life](auto self, auto space) {
                    std::ignore = space;
                    wait_loop<fusion::Input>(self, [&life](auto self, auto space) {
                        std::ignore = space;
                        clear(self);
                        return life?--life:0;
                    });
                },
                std::chrono::system_clock::now() + std::chrono::milliseconds{300},
                std::chrono::milliseconds{40});
          },
          std::chrono::system_clock::now(),
          std::chrono::milliseconds{120});
    });

    // check
    EXPECT_EQ(0, life);
}