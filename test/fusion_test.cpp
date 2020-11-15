#include <chrono>

#include <gtest/gtest.h>

#include "resources/timer.hpp"

namespace fusion {} // namespace fusion


/// Test
/// @brief
TEST(fusion_space, positive_test) {
    build<fusion::Space>([](auto self) {
        build<fusion::Timer>(
          self,
          [](auto self, auto space) {
              auto entry_point = [self](auto entry_point) -> void {
                  wait<fusion::Input>(self, [entry_point](auto self, auto space) {
                      std::cout << "tick1..." << std::endl;
                      clear(self);
                      entry_point(entry_point);
                  });
              };
              entry_point(entry_point);

              build<fusion::Timer>(
                self,
                [](auto self, auto space) {
                    auto entry_point = [self](auto entry_point) -> void {
                        wait<fusion::Input>(self, [entry_point](auto self, auto space) {
                            std::cout << "tick2..." << std::endl;
                            clear(self);
                            entry_point(entry_point);
                        });
                    };
                    entry_point(entry_point);
                },
                std::chrono::system_clock::now(),
                std::chrono::seconds{1});
          },
          std::chrono::system_clock::now() + std::chrono::seconds{5},
          std::chrono::seconds{1});
    });

    // Check
    EXPECT_EQ(1, 1);
}