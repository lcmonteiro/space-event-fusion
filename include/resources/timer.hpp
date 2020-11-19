#pragma once

#include <chrono>

#include "fusion.hpp"

namespace fusion {
class Timer : public Element {
  public:
    using Shared = std::shared_ptr<Timer>;
    /// constructor
    /// @param point time
    /// @param step time period
    template <typename Clock, typename Duration>
    Timer(
      const std::chrono::time_point<Clock, Duration>& point,
      const std::chrono::milliseconds& step = std::chrono::milliseconds(0));

  protected:
    /// count
    /// @return number of expired times
    friend auto count(Shared) -> size_t;

    /// clear events
    friend auto clear(Shared) -> void;
};
} // namespace fusion