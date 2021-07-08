#include "FuzzyAISystem.h"
#include "Coordinator.h"
#include "FuzzyAI.h"
#include "Transform.h"

void FuzzyAISystem::Update()
{
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto& ai = Coordinator::GetComponent<FuzzyAI>(entity);
    
    // crisp input
    ai.distance = glm::distance(transform.GetPosition(), *ai.predatorPos);

    // calculate firing strength
    for (int i = 0; i < ai.antecedents.size(); i++)
    {
      ai.alphas[i] = ai.antecedents[i]->Interpolate(ai.distance);
    }

    // calculate B'(y)
    ai.centerOfGravity = 0.f;
    float centerOfGravityDivisor = 0.f;
    for (int i = 0; i < ai.sampleFrequency; i++)
    {
      // sample each point
      float step = (ai.end - ai.start) / ai.sampleFrequency;
      float samplePoint = ai.start + step * i;

      // calculate B'(y) using Mandani
      float height = 0.f;
      ai.result[i] = 0.f; // reset
      for (int j = 0; j < ai.antecedents.size(); j++)
      {
        float height = ai.consequences[j]->Interpolate(samplePoint);
        ai.result[i] = glm::max(ai.result[i], glm::min(height, ai.alphas[j]));
      }

      ai.centerOfGravity += ai.result[i] * samplePoint;
      centerOfGravityDivisor += ai.result[i];
    }

    // calculate center of gravity
    // crisp output
    ai.centerOfGravity /= centerOfGravityDivisor;

    // running away
    glm::vec3 awayFromPredator = glm::normalize(glm::vec3(transform.GetPosition().x, 0.f, transform.GetPosition().z) - glm::vec3(ai.predatorPos->x, 0.f, ai.predatorPos->z));
    ai.velocity = ai.centerOfGravity * awayFromPredator;

    transform.SetPosition(transform.GetPosition() + (ai.freeze ? glm::vec3(0.f) : ai.velocity) * ai.slowDown);
  }
}