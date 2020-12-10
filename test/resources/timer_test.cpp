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
              call(self, [&life](auto self, auto callback) {
                  wait<fusion::Input>(self, [&life, callback](auto self, auto space) {
                      clear(self);
                      if (life) {
                          ++life;
                          call(self, callback);
                      }
                  });
              });

              // top timer
              build<fusion::Timer>(
                self,
                [&life](auto self, auto space) {
                    call(self, [&life](auto self, auto callback) {
                        wait<fusion::Input>(self, [&life, callback](auto self, auto space) {
                            clear(self);
                            if (life) {
                                --life;
                                call(self, callback);
                            }
                        });
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