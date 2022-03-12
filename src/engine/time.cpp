#include "time.hpp"
#include "log.hpp"

#ifdef WIN32
#include <Windows.h>
#else // WIN32
#include <sys/time.h>
#endif // WIN32

namespace dc
{

std::int64_t current_time_millis()
{
#ifdef WIN32
  return GetTickCount64();
#else  // WIN32
  timeval t;
  gettimeofday(&t, nullptr);
  auto ret = t.tv_sec * 1000 + t.tv_usec / 1000;

  return ret;
#endif // WIN32
}

Timer::Timer() { reset(); }

void Timer::reset() { start_ = std::chrono::high_resolution_clock::now(); }

float Timer::elapsed()
{
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::high_resolution_clock::now() - start_)
             .count() *
         0.001f * 0.001f * 0.001f;
}

float Timer::elapsed_millis() { return elapsed() * 1000.0f; }

ScopedTimer::ScopedTimer(const std::string &name) : name_{name} {}

ScopedTimer::~ScopedTimer()
{
  const auto time = timer_.elapsed_millis();
  DC_LOG_DEBUG("[TIMER] {} - {}ms", name_, time);
}

void PerformanceProfiler::set_per_frame_timing(const std::string &name,
                                               float              time)
{
  if (per_frame_data_.find(name) == per_frame_data_.end())
  {
    per_frame_data_[name] = 0.0f;
  }

  per_frame_data_[name] += time;
}

void PerformanceProfiler::clear()
{
  last_per_frame_data_ = std::move(per_frame_data_);
  per_frame_data_      = {};
}

void PerformanceProfiler::for_each(
    const std::function<void(const std::string &name, float time_ms)> process)
{
  for (const auto &[name, time] : last_per_frame_data_)
  {
    process(name, time);
  }
}

ScopedPerformanceTimer::ScopedPerformanceTimer(const std::string   &name,
                                               PerformanceProfiler &profiler)
    : name_{name},
      profiler_{profiler}
{
}

ScopedPerformanceTimer::~ScopedPerformanceTimer()
{
  const auto time = timer_.elapsed_millis();
  profiler_.set_per_frame_timing(name_, time);
}

} // namespace dc
