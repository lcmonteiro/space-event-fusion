#include <chrono>

#include <gtest/gtest.h>

#include "resources/timer.hpp"


struct Monitor {
    Monitor() { std::cout << "build " << this << std::endl; }
    Monitor(const Monitor&) { std::cout << "copy " << this << std::endl; }
    Monitor(Monitor&&) { std::cout << "move " << this << std::endl; }
};

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
              call(self, [&life](auto self, auto scope) {
                  wait<fusion::Input>(scope, [&life, callable = self](auto self, auto space) {
                      clear(self);
                      if (life) {
                          ++life;
                          call(self, callable);
                      }
                      wait<fusion::Continue>(scope);
                  });
              });

              // top timer
              build<fusion::Timer>(
                self,
                [&life](auto self, auto space) {
                    call(self, [&life](auto self, auto scope) {
                        wait<fusion::Input>(scope, [&life, callable = self](auto self, auto space) {
                            clear(self);
                            if (life) {
                                --life;
                                call(self, callable);
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