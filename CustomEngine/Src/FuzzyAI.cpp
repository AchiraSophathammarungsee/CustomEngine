#include "FuzzyAI.h"
#include <imgui.h>
#include "EngineCore.h"

float TriangularFuzzyMember::Interpolate(float input)
{
  if (input >= start && input < end)
  {
    if (input > middle)
    {
      return (end - input) / (end - middle);
    }
    else
    {
      return (input - start) / (middle - start);
    }
  }
  return 0.f;
}

float TrapezoidFuzzyMember::Interpolate(float input)
{
  if (left)
  {
    if (input <= high)
      return 1.f;
    else if (input < low)
      return (low - input) / (low - high);
    else
      return 0.f;
  }
  else
  {
    if (input >= high)
      return 1.f;
    else if (input > low)
      return (input - low) / (high - low);
    else
      return 0.f;
  }
}

float getGraphValue(void* data, int id)
{
  return (reinterpret_cast<float*>(data))[id];
}

void TriangularFuzzyMember::DrawDebugMenu(int id, int offset, FuzzyAI* parent)
{
  ImGui::PushID(id + offset);
  ImGui::Text("[#%d Triangular]:", id);
  ImGui::Indent(10.f);
  if (ImGui::InputFloat("start", &start))
  {
    if (start > middle)
      start = middle;
  }
  if (ImGui::InputFloat("middle", &middle))
  {
    if (middle > end)
      middle = end;
    else if (middle < start)
      middle = start;
  }
  if (ImGui::InputFloat("end", &end))
  {
    if (end < middle)
      end = middle;
  }
  ImGui::Unindent(10.f);
  //ImGui::Text("[#%d Triangular]: (%f, %f, %f)", id, start, middle, end);
  ImGui::PopID();
}

void TrapezoidFuzzyMember::DrawDebugMenu(int id, int offset, FuzzyAI* parent)
{
  ImGui::PushID(id + offset);
  if (left)
  {
    ImGui::Text("[#%d Trapezoid]:", id);
    ImGui::Indent(10.f);
    if (ImGui::InputFloat("start", &high))
    {
      if (high < low)
        high = low;

      if (high > parent->end) parent->end = high;
    }
    if (ImGui::InputFloat("end", &low))
    {
      if (low < high)
        low = high;

      if (low < parent->start) parent->start = low;
    }
    ImGui::Text("Sided: %s", left ? "left" : "right");
    ImGui::Unindent(10.f);
    //ImGui::Text("[#%d Trapezoid]: (%f, %f)", id, high, low);
  }
  else
  {
    ImGui::Text("[#%d Trapezoid]:", id);
    ImGui::Indent(10.f);
    if (ImGui::InputFloat("start", &low))
    {
      if (low < high)
        low = high;
    }
    if (ImGui::InputFloat("end", &high))
    {
      if (high < low)
        high = low;
    }
    ImGui::Text("Sided: %s", left ? "left" : "right");
    ImGui::Unindent(10.f);
    //ImGui::Text("[#%d Trapezoid]: (%f, %f)", id, low, high);
  }
  ImGui::PopID();
}

FuzzyAI::FuzzyAI(float start, float end, const glm::vec3* targetPredator, int antecedentsNum, int sampleFreq) : 
  predatorPos(targetPredator), sampleFrequency(sampleFreq), start(start), end(end), freeze(true), slowDown(0.5f)
{
  for (int i = 0; i < antecedentsNum; i++)
  {
    antecedents.push_back(nullptr);
    consequences.push_back(nullptr);
    alphas.push_back(0.f);
  }
  for(int i = 0; i < sampleFrequency; i++)
    result.push_back(0.f);
}

FuzzyAI::~FuzzyAI()
{
  /*for (int i = 0; i < antecedents.size(); i++)
  {
    delete antecedents[i];
  }

  for (int i = 0; i < consequences.size(); i++)
  {
    delete consequences[i];
  }*/
}

void FuzzyAI::AddAntecedents(int id, FuzzyMember* f)
{
  antecedents[id] = f;
}

void FuzzyAI::AddConsequences(int id, FuzzyMember* f)
{
  consequences[id] = f;
}

void FuzzyAI::DrawDebugMenu()
{
  if (ImGui::CollapsingHeader("Fuzzy AI"))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    ImGui::Text("Start: %f, End: %f", start, end);
    ImGui::Text("Predator Pos: (%f,%f,%f)", predatorPos->x, predatorPos->y, predatorPos->z);
    ImGui::Text("Distance: %f", distance);

    ImGui::Text("Antecedents");
    ImGui::Indent(10.f);
    for (int i = 0; i < antecedents.size(); i++)
    {
      antecedents[i]->DrawDebugMenu(i, 0, this);
    }
    ImGui::Unindent(10.f);

    ImGui::Text("Consequences");
    ImGui::Indent(10.f);
    for (int i = 0; i < consequences.size(); i++)
    {
      consequences[i]->DrawDebugMenu(i, consequences.size(), this);
    }
    ImGui::Unindent(10.f);

    ImGui::Text("firing strength:");
    ImGui::Indent(10.f);
    for (int i = 0; i < alphas.size(); i++)
    {
      ImGui::Text("alpha%d: %f", i, alphas[i]);
    }
    ImGui::Unindent(10.f);

    ImGui::PlotLines("B'(y)", getGraphValue, &result[0], result.size());
    ImGui::Text("%f                                                       %f", start, end);

    if (ImGui::DragInt("Sample frequency", &sampleFrequency, 1.f, 2, 50))
    {
      result.clear();
      for (int i = 0; i < sampleFrequency; i++)
        result.push_back(0.f);
    }
    ImGui::Text("Center of gravity: %f", centerOfGravity);
    ImGui::Text("Velocity: (%f,%f,%f)", velocity.x, velocity.y, velocity.z);
    ImGui::Checkbox("Freeze", &freeze);
    ImGui::DragFloat("Slow Down Rate", &slowDown, 0.1F, 0.f, 1.f);

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}