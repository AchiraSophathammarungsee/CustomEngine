#include "Time.h"
#include "EngineCore.h"
#include <iomanip>
#include "Profiler.h"

#ifdef PLATFORM_WINDOWS
#include <ctime>
#endif

#define SEC_TO_NANO 1000000000

MeasureClock::MeasureClock(const char* _name)
{
  MeasureStart = std::chrono::high_resolution_clock::now();
  name = _name;
}

MeasureClock::~MeasureClock()
{
  LOG_TRACE_S("Time", "{} took {} ms to complete the task.", name, std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - MeasureStart).count());
}

void TimeMeasure::Start()
{
  MeasureStart = std::chrono::high_resolution_clock::now();
}

size_t TimeMeasure::Stop()
{
  return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - MeasureStart).count();
}

std::chrono::high_resolution_clock::time_point Time::StartTime;
std::chrono::nanoseconds Time::PrevTime;
std::chrono::nanoseconds Time::dt;
std::chrono::nanoseconds Time::fixed_dt;
std::chrono::nanoseconds Time::accumDT;
std::chrono::nanoseconds Time::accumSec;
float Time::fixedDT;
size_t Time::FrameCounter;
float Time::FrameRate;
float Time::fps;
float Time::nextStepRatio;
int Time::targetFPS;
int Time::FixedUpdateCount;

void Time::Init()
{
  LOG_TRACE("Time", "Initializing Time...");

  StartTime = std::chrono::high_resolution_clock::now();
  PrevTime = std::chrono::nanoseconds(0);
  accumDT = std::chrono::nanoseconds(0);
  FrameCounter = 0;
  FrameRate = 30;
  fps = 33.333f;
  targetFPS = 60;
  fixed_dt = std::chrono::nanoseconds(SEC_TO_NANO) / targetFPS;
  fixedDT = (float)fixed_dt.count() / (float)SEC_TO_NANO;
  FixedUpdateCount = 0;
  nextStepRatio = 0.f;
}

void Time::Update()
{
  std::chrono::nanoseconds currTime = std::chrono::high_resolution_clock::now() - StartTime;
  dt = currTime - PrevTime;
  PrevTime = currTime;

  accumSec += dt;
  FrameCounter++;

  std::chrono::nanoseconds sec(SEC_TO_NANO);
  if (accumSec >= sec)
  {
    FrameRate = (float)FrameCounter;
    FrameCounter = 0;
    accumSec -= sec;

    Profiler::MeasureTimes();
  }
}

void Time::UpdateTimeStepRatio()
{
  nextStepRatio = (float)accumDT.count() / (float)fixed_dt.count();
}

float Time::DT()
{
  std::chrono::seconds sec(1);
  return (float)dt.count() / (float)std::chrono::nanoseconds(sec).count();
}

float Time::FPS()
{ 
  return FrameRate; 
}

std::string Time::GetDate()
{
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);

  std::tm bt;
#ifdef PLATFORM_WINDOWS
  localtime_s(&bt, &time_t);
#elif PLATFORM_LINUX
  localtime_r(&time, &bt);
#endif

  char buffer[64];
  std::memset(buffer, 0, sizeof(buffer));
  std::strftime(buffer, sizeof(buffer), "%d-%m-%Y %X", &bt);

  return std::string(buffer);
}

float Time::CurrTime()
{
  auto now = std::chrono::system_clock::now();
  return now.time_since_epoch().count();
}

void Time::SetTargetFPS(int fps)
{ 
  targetFPS = (fps > 1) ? fps : 1; 
  fixed_dt = std::chrono::nanoseconds(SEC_TO_NANO) / targetFPS;
  fixedDT = (float)SEC_TO_NANO / (float)fixed_dt.count();
}

void Time::DrawDebugMenu()
{
  static bool open = true;

  ImGui::SetNextWindowSize(ImVec2(200, 300));
  ImGui::Begin("Time", &open);

  ImGui::Text("FPS: %f", FPS());
  ImGui::Text("DT: %f", DT());

  if (ImGui::InputInt("Target Fixed FPS", &targetFPS))
  {
    SetTargetFPS(targetFPS);
  }
  ImGui::Text("Fixed DT: %f", FixedDT);
  ImGui::Text("Fixed Update Count: %d", FixedUpdateCount);

  ImGui::End();
}