#include "AudioClip.h"
#include <imgui.h>
#include "AudioManager.h"
#include "ResourceManager.h"
#include "FileDialog.h"

void AudioClip::Open(const char* filepath)
{
  m_sound = AudioManager::Open(filepath);
  m_filePath = filepath;
}

void AudioClip::DrawDebugMenu()
{
  static char nameBuffer[256] = { 0 };
  if (ImGui::InputText("Name", nameBuffer, 256))
  {
    m_name = nameBuffer;
  }

  ImGui::Text("File path: %s", m_filePath);
}

void AudioClip::DrawResourceCreator(bool& openFlag)
{
  static char texNameBuffer[256] = "unnamed";
  static char texPathBuffer[270] = "../Common/audios/";

  ImGui::SetNextWindowSize(ImVec2(300, 300));
  ImGui::Begin("New Audio Clip", &openFlag);

  if (ImGui::Button("Open File..."))
  {
    std::string path = FileDialog::OpenFile("Audio Files\0*.aiff;*.asf;*asx;*.dls;*.flac;*.fsb;*.it;*.m3u;*.midi;*.mod;*.mp2;*.mp3;*.ogg;*.pls;*.s3m;*.vag;*.wav;*.wax;*.wma;*.xm;*.xma\0");
    strcpy_s(texPathBuffer, 270, path.c_str());
  }
  ImGui::InputText("audio name", texNameBuffer, 255);
  ImGui::InputText("audio path", texPathBuffer, 270);

  if (ImGui::Button("Create Audio Clip"))
  {
    ResourceManager::Add<AudioClip>(new AudioClip(texPathBuffer), texNameBuffer);
    openFlag = false;
  }

  ImGui::End();
}