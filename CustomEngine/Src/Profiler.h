#pragma once
#ifndef PROFILER_H
#define PROFILER_H

#include <map>
#include <vector>
#include <string>
#include "Time.h"

#define PROFILER_SAMPLE_AVERAGE 60

struct ProfileFunc
{
  ProfileFunc() : parent(nullptr), functionName(nullptr), ms(0), avgMS(0) {};

  ProfileFunc* parent;
  const char* functionName;
  TimeMeasure clock;
  float ms;
  float avgMS;
  std::map<const char*, ProfileFunc> childs;
};

class Profiler
{
public:
  static void FrameBegin();
  static void FrameEnd();
  static void Begin(const char *functionName);
  static void End();

  static void MeasureTimes();

  static void DrawDebugMenu();

private:
  Profiler() {};
  ~Profiler() {};

  static int frameCount;
  static void RecurrProfileTimes(ProfileFunc* curr);
  static void RecurrProfiles(ProfileFunc* curr);

  static ProfileFunc Root;
  static ProfileFunc *CurrPath;
  static float TotalMS;
};

#endif