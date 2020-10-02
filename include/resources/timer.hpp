#pragma once

#include <chrono>

#include "base.hpp"

namespace Fusion {
namespace Resource {
    class Timer : public Base {
      public:
        Timer()        = default;
        Timer(Timer&&) = default;
        Timer& operator=(Timer&&) = default;

        /// constructor
        /// @param point time
        /// @param step time period
        template <typename Clock, typename Duration>
        Timer(
            const std::chrono::time_point<Clock, Duration>& point,
            const std::chrono::milliseconds& step);

        /// count
        /// @return number of expired times
        std::size_t count();

        /// clear events
        void clear();
    };

} // namespace Resource
} // namespace Fusion