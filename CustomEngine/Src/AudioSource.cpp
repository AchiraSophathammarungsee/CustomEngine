#include "AudioSource.h"
#include <imgui.h>
#include "AudioClip.h"
#include "ResourceManager.h"
#include "AudioManager.h"

void AudioSource::Init()
{
  //if(PlayOnAwake)
    AudioManager::Play(m_audio);
}

void AudioSource::DrawDebugMenu()
{
  ImGuiTreeNodeFlags headerFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;
  if (ImGui::CollapsingHeader("Audio Source", headerFlags))
  {
    ImGui::Separator();
    ImGui::Indent(10.f);

    static bool selectAudio = false;
    if (m_audio)
    {
      if (ImGui::Button(m_audio->Name().c_str()))
      {
        selectAudio = true;
      }
    }
    else
    {
      if (ImGui::Button("Select audio clip"))
      {
        selectAudio = true;
      }
    }
    
    if (selectAudio)
    {
      ResourceManager::DrawResourceSelector<AudioClip>(&m_audio, &selectAudio);
    }

    if (ImGui::Button("Play"))
    {
      Init();
    }

    if (ImGui::Button("Stop"))
    {
      //m_audio->Audio()->setMode();
    }

    ImGui::Checkbox("Play On Awake", &PlayOnAwake);
    ImGui::Checkbox("Loop", &Loop);
    ImGui::Checkbox("Mute", &Mute);
    ImGui::Checkbox("Ignore Effects", &IgnoreEffects);
    ImGui::Checkbox("Ignore Reverb", &IgnoreReverb);

    ImGui::SliderFloat("Volume", &m_volume, 0.f, 1.f);
    ImGui::SliderFloat("Pitch", &m_pitch, 0.f, 2.f);
    ImGui::SliderFloat("Stereo Pan", &m_stereoPan, -1.f, 1.f);

    ImGui::Unindent(10.f);
    ImGui::Separator();
  }
}