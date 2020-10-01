#pragma once

#include <chrono>

#include "base.hpp"

namespace Fusion {
namespace Resource {

    class Timer : protected Base {
      public:
        Timer() = default;
        
        /// constructor
        /// @param point time
        /// @param step time period
        template <typename Clock, typename Duration>
        Timer(const std::chrono::time_point<Clock, Duration>& point, const Duration& step);

        /// read
        /// @return number of expired times
        std::size_t read();
    };

} // namespace Resource
} // namespace Fusion