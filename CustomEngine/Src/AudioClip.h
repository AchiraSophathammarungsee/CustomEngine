#pragma once
#ifndef AUDIOCLIP_H
#define AUDIOCLIP_H

#include <fmod.hpp>
#include <string>

class Framebuffer;
class Shader;
class Transform;

class AudioClip
{
public:
  AudioClip(const char* filepath) : m_sound(nullptr) { Open(filepath); };
  ~AudioClip() { m_sound->release(); };

  void Open(const char* filepath);

  FMOD::Sound* Audio() const { return m_sound; };
  std::string Name() const { return m_name; };
  std::string FilePath() const { return m_filePath; };

  void DrawDebugMenu();

  static void GeneratePreview(AudioClip* texture, Framebuffer* preview, Shader* previewShader, Transform* transform) {};
  static void DrawResourceCreator(bool& openFlag);

private:
  std::string m_name;

  std::string m_filePath;
  FMOD::Sound* m_sound;
};

#endif