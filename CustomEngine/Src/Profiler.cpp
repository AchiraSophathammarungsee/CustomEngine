#include "Profiler.h"
#include <imgui.h>

ProfileFunc Profiler::Root;
ProfileFunc* Profiler::CurrPath;
int Profiler::frameCount = 0;
float Profiler::TotalMS = 0.f;

void Profiler::FrameBegin()
{
  Root.functionName = "Total frame";
  Root.clock.Start();
  Root.parent = nullptr;

  CurrPath = &Root;
}

void Profiler::FrameEnd()
{
  Root.ms += (float)Root.clock.Stop() / 1000.f;

  CurrPath = CurrPath->parent;

  frameCount++;
  if (frameCount >= 60)
  {
    RecurrProfileTimes(&Root);
    frameCount = 0;

    TotalMS = Root.avgMS;
  }
}

void Profiler::Begin(const char* functionName)
{
  auto &child = CurrPath->childs[functionName];

  child.clock.Start();
  child.parent = CurrPath;
  child.functionName = functionName;
  CurrPath = &child;
}

void Profiler::End()
{
  CurrPath->ms += (float)CurrPath->clock.Stop() / 1000.f;

  CurrPath = CurrPath->parent;
}

void Profiler::RecurrProfileTimes(ProfileFunc* curr)
{
  // micro -> milli (1/1000)
  curr->avgMS = (float)curr->ms / 60.f;
  curr->ms = 0;

  for (auto& child : curr->childs)
  {
    RecurrProfileTimes(&child.second);
  }
}

void Profiler::MeasureTimes()
{
  RecurrProfileTimes(&Root);
}

void Profiler::RecurrProfiles(ProfileFunc *curr)
{
  ImGui::Indent(10.f);

  //float percent = totalMS > 0 ? curr->ms / totalMS * 100.f : 0.f;

  ImGui::Text("%s: %f (%f)", curr->functionName, curr->avgMS, curr->avgMS / TotalMS);// , percent);

  for (auto &child : curr->childs)
  {
    RecurrProfiles(&child.second);
  }

  ImGui::Unindent(10.f);
}

void Profiler::DrawDebugMenu()
{
  static bool open = true;

  ImGui::SetNextWindowSize(ImVec2(500, 400));
  ImGui::Begin("Profiler", &open);

  //size_t totalMS = Root.ms;
  RecurrProfiles(&Root);

  ImGui::End();
}