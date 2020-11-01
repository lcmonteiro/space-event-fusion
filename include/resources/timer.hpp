#pragma once

#include <chrono>

#include "fusion.hpp"

namespace fusion {
class Timer : public Element {
  public:
    // default
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
    friend auto count(std::shared_ptr<Timer>) -> size_t;
    /// clear events
    friend auto clear(std::shared_ptr<Timer>) -> void;
};
auto count(std::shared_ptr<Timer>) -> size_t;
auto clear(std::shared_ptr<Timer>) -> void;

} // namespace fusion