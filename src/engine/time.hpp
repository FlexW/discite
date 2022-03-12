#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace dc
{
std::int64_t current_time_millis();

class Timer
{
public:
  Timer();

  void  reset();
  float elapsed();
  float elapsed_millis();

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class ScopedTimer
{
public:
  ScopedTimer(const std::string &name);
  ~ScopedTimer();

private:
  std::string name_;
  Timer       timer_;
};

class PerformanceProfiler
{
public:
  void set_per_frame_timing(const std::string &name, float time);
  void clear();

  void for_each(const std::function<void(const std::string &name,
                                         float              time_ms)> process);

private:
  std::unordered_map<std::string, float> per_frame_data_;
  std::unordered_map<std::string, float> last_per_frame_data_;
};

class ScopedPerformanceTimer
{
public:
  ScopedPerformanceTimer(const std::string   &name,
                         PerformanceProfiler &profiler);
  ~ScopedPerformanceTimer();

private:
  std::string          name_;
  Timer                timer_;
  PerformanceProfiler &profiler_;
};

#if defined(DC_ENABLE_TIMING)

#define DC_TIME_SCOPE_PERF(name)                                               \
  ScopedPerformanceTimer timer__LINE__(                                        \
      name,                                                                    \
      *dc::Engine::instance()->performance_profiler())

#else

#define DC_TIME_SCOPE_PERF(name) void(0)

#endif

#define DC_TIME_SCOPE(name) ScopedTimer timer__LINE__(name)

} // namespace dc
