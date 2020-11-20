
#include "resources/timer.hpp"

#include <string.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace fusion {
template <typename Point, typename Duration>
void setup(const Handler& handler, const Point& point, const Duration& step) {
    struct itimerspec config;
    {
        auto duration = point.time_since_epoch();
        auto seconds  = std::chrono::duration_cast<std::chrono::seconds>(duration);
        auto nanosecs = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);

        config.it_value.tv_sec  = time_t(seconds.count());
        config.it_value.tv_nsec = long(nanosecs.count());
    }
    {
        auto duration = step;
        auto seconds  = std::chrono::duration_cast<std::chrono::seconds>(duration);
        auto nanosecs = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);

        config.it_interval.tv_sec  = time_t{seconds.count()};
        config.it_interval.tv_nsec = long{nanosecs.count()};
    }
    if (timerfd_settime(handler.native(), TFD_TIMER_ABSTIME, &config, NULL) < 0)
        throw std::system_error(std::make_error_code(std::errc(errno)), "timerfd_settime");
}

template <>
Timer::Timer(
  const std::chrono::steady_clock::time_point& point,
  const std::chrono::milliseconds& duration)
  : Element{timerfd_create(CLOCK_MONOTONIC, 0)} {
    setup(handler, point, duration);
}

template <>
Timer::Timer(
  const std::chrono::system_clock::time_point& point,
  const std::chrono::milliseconds& duration)
  : Element{timerfd_create(CLOCK_REALTIME, 0)} {
    setup(handler, point, duration);
}

auto count(std::shared_ptr<Timer> self) -> size_t {
    std::uint64_t u = 0;
    if (::read(self->handler.native(), &u, sizeof(uint64_t)) != sizeof(uint64_t))
        throw std::system_error(std::make_error_code(std::errc(errno)), "timer::count:");
    return std::size_t(u);
}

auto clear(std::shared_ptr<Timer> self) -> void {
    std::uint64_t u = 0;
    if (::read(self->handler.native(), &u, sizeof(uint64_t)) != sizeof(uint64_t))
        throw std::system_error(std::make_error_code(std::errc(errno)), "timer::clear:");
}
} // namespace fusion