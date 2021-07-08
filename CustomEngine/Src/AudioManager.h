#pragma once
#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <fmod.hpp>

class AudioClip;

class AudioManager
{
public:
  static void Init();
  static void Update();
  static void Exit();

  static FMOD::Sound* Open(const char* audioPath);
  static void Play(AudioClip *audio);
  static void Stop(AudioClip *audio);

private:
  AudioManager() {};
  ~AudioManager() {};

  static FMOD::System* m_system;
  static FMOD::ChannelGroup* m_channelGroup;
};

#endif