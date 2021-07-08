#include "SplinePathSystem.h"
#include "shader.hpp"
#include "Coordinator.h"
#include "BSpline.h"
#include "Transform.h"
#include "Renderer.h"
#include "Time.h"
#include "SkeletalAnimation.h"
#include <numeric>

void SplinePathSystem::Update()
{
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    auto& spline = Coordinator::GetComponent<BSpline>(entity);

    // make transform follow spline
  }
}

void SplinePathSystem::Draw(Shader* shader)
{
  shader->use();
  for (EntityID entity : Entities)
  {
    auto& transform = Coordinator::GetComponent<Transform>(entity);
    BSpline& spline = Coordinator::GetComponent<BSpline>(entity);
    SkeletalAnimationSet& animations = Coordinator::GetComponent<SkeletalAnimationSet>(entity);

    if (spline.IsPlaying())
    {
      if (animations.Playing == false)
      {
        if (animations.CurrAnimation == -1)
        {
          animations.SetAnimation(2);
        }

        animations.Playing = true;
        animations.Start();

        animations.UseCustomTime = (spline.Easing) ? true : false;
      }

      float samplingT = spline.SamplingDistance(spline.Distance);

      glm::vec3 pos = spline.SampleSpline(samplingT);
      glm::vec3 nextPos = spline.SampleSpline(samplingT + 0.00001f);
      glm::vec3 dir = glm::normalize(nextPos - pos);

      transform.SetPosition(pos);

      transform.SetRotationAxis(glm::vec3(0.f, 1.f, 0.f));
      transform.SetRotationAngle(glm::degrees(-glm::atan(dir.z, dir.x)) + 90.f);

      float speed = 0.f;
      if (spline.Easing)
      {
        float animSpeed = spline.Speed;
        float animDur = spline.EaseDur;

        float t1 = spline.RampUpTime;
        float t3 = animDur;
        float t2 = t3 - spline.RampDownTime;

        if (spline.Time <= t1)
        {
          speed = spline.Time * animSpeed / t1;
          spline.Distance = animSpeed * spline.Time * spline.Time / 2.f;
        }
        else if (spline.Time <= t2)
        {
          speed = animSpeed;
          float s1 = animSpeed * t1 * t1 / 2.f;
          spline.Distance = s1 + animSpeed * (spline.Time - t1);
        }
        else
        {
          speed = (t3 - spline.Time) * animSpeed / (t3 - t2);
          float s1 = animSpeed * t1 * t1 / 2.f;
          float s2 = s1 + animSpeed * (t2 - t1);
          spline.Distance = s2 + (animSpeed / (2.f * (t3 - t2))) * ((2.f * t3 * spline.Time) - (spline.Time * spline.Time) - (2.f * t3 * t2) + (t2 * t2));
        }

        float keyframeT = animations.PlaySpeed * spline.Distance * animSpeed;
        float overflowT = keyframeT / animations.Animations[animations.CurrAnimation].Duration;
        overflowT = floorf(overflowT);
        animations.CustomTime = keyframeT - (overflowT * animations.Animations[animations.CurrAnimation].Duration);

        spline.Time += Time::DT();
        if (spline.Time > t3)
        {
          spline.Time = t3;
          spline.Stop();
          animations.Stop();
          animations.UseCustomTime = false;
        }
      }
      else
      {
        speed = spline.Speed * Time::DT();
        spline.Distance += speed;
      }
      
      if (spline.Distance >= spline.GetTotalDist())
      {
        spline.Distance = spline.GetTotalDist();
        spline.Stop();
        animations.Stop();
        animations.UseCustomTime = false;
      }
    }

    Renderer::DebugBSpline(&spline);
  }
}