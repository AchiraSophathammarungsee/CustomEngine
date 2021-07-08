#pragma once
#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

class AudioClip;

class AudioSource
{
public:
  AudioSource(AudioClip* audio = nullptr) : m_audio(audio), m_priority(128), m_pitch(1.f), m_volume(1.f), m_stereoPan(0.f), PlayOnAwake(false), Loop(false), Mute(false), 
    IgnoreEffects(false), IgnoreReverb(false) {};
  ~AudioSource() {};

  void Init();

  const AudioClip* Audio() const { return m_audio; };
  void SetAudio(AudioClip* audio) { m_audio = audio; };

  int Priority() const { return m_priority; };
  float Pitch() const { return m_pitch; };
  float Volume() const { return m_volume; };
  float StereoPan() const { return m_stereoPan; };

  void SetPriority(int val) { m_priority = val; };
  void SetPitch(float val) { m_pitch = val; };
  void SetVolume(float val) { val = (val < 0) ? 0 : ((val > 1) ? 1 : val); m_volume = val; };
  void StereoPan(float val) { val = (val < -1) ? -1 : ((val > 1) ? 1 : val); m_stereoPan = val; };

  bool PlayOnAwake;
  bool Loop;
  bool Mute;

  bool IgnoreEffects;
  bool IgnoreReverb;

  void DrawDebugMenu();

private:
  AudioClip* m_audio;

  int m_priority;
  float m_pitch;
  float m_volume;
  float m_stereoPan;
};

#endif