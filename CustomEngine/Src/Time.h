
#ifndef TIME_H
#define TIME_H

#include <chrono>
#include <string>

class MeasureClock
{
public:
  MeasureClock(const char *name);
  ~MeasureClock();

private:
  std::chrono::high_resolution_clock::time_point MeasureStart;
  const char* name;
};

class TimeMeasure
{
public:
  TimeMeasure() {};
  ~TimeMeasure() {};

  void Start();
  size_t Stop();

private:
  std::chrono::high_resolution_clock::time_point MeasureStart;
};

class Time
{
public:
  typedef std::chrono::duration<float, std::milli> millisec;
  typedef std::chrono::duration<float, std::nano> nanosec;

  static void Init();
  static void Update();
  static void UpdateTimeStepRatio();
  
  static void IncrementFrameCount() { FrameCounter++; };
  static void IncrementFixedFrameCount() { FixedUpdateCount++; };
  static void ResetFixedFrameCount() { FixedUpdateCount = 0; };
  static float DT();
  static std::chrono::nanoseconds &DeltaTime() { return dt; };
  static std::chrono::nanoseconds &FixedDeltaTime() { return fixed_dt; };
  static std::chrono::nanoseconds &AccumDT() { return accumDT; };
  static float FixedDT() { return fixedDT; };
  static int TargetFPS() { return targetFPS; };
  static void SetTargetFPS(int fps);
  static float FPS();
  static std::string GetDate();
  static float CurrTime();

  static void DrawDebugMenu();

private:
  Time() {};
  ~Time() {};

  static std::chrono::high_resolution_clock::time_point StartTime;
  static std::chrono::nanoseconds PrevTime;
  static std::chrono::nanoseconds dt;
  static std::chrono::nanoseconds fixed_dt;
  static std::chrono::nanoseconds accumDT;
  static std::chrono::nanoseconds accumSec;
  static float nextStepRatio;
  static float fixedDT;
  static size_t FrameCounter;
  static float FrameRate;
  static float fps;
  static int targetFPS;
  static int FixedUpdateCount;
};

#endif