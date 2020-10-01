
#include "resources/timer.hpp"

#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>

namespace Fusion {
namespace Resource {

    template <typename Point, typename Duration>
    void setup(int handler, const Point& point, const Duration& step) {
        struct itimerspec config;
        {
            auto duration = point.time_since_epoch();
            auto seconds  = std::chrono::duration_cast<std::chrono::seconds>(duration);
            auto nanosecs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                duration - seconds);

            config.it_value.tv_sec  = time_t(seconds.count());
            config.it_value.tv_nsec = long(nanosecs.count());
        }
        {
            auto duration = step;
            auto seconds  = std::chrono::duration_cast<std::chrono::seconds>(duration);
            auto nanosecs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                duration - seconds);

            config.it_interval.tv_sec  = time_t{seconds.count()};
            config.it_interval.tv_nsec = long{nanosecs.count()};
        }
        if (timerfd_settime(handler, TFD_TIMER_ABSTIME, &config, NULL) < 0)
            throw std::runtime_error(std::string("timerfd_settime: ") + strerror(errno));
    }

    template <>
    Timer::Timer(
        const std::chrono::steady_clock::time_point& point,
        const std::chrono::nanoseconds& duration)
      : Base(timerfd_create(CLOCK_MONOTONIC, 0)) {
        setup(native_(), point, duration);
    }

    template <>
    Timer::Timer(
        const std::chrono::system_clock::time_point& point,
        const std::chrono::nanoseconds& duration)
      : Base(timerfd_create(CLOCK_REALTIME, 0)) {
        setup(native_(), point, duration);
    }

} // namespace Resource
} // namespace Fusion