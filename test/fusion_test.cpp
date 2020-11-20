#include <chrono>

#include <gtest/gtest.h>

#include "resources/timer.hpp"

namespace fusion {} // namespace fusion


/// Test
/// @brief
TEST(fusion_space, positive_test) {
    build<fusion::Space>([](auto self) {
   
    });

    // Check
    EXPECT_EQ(1, 1);
}