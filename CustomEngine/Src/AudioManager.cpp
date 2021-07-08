#include "AudioManager.h"
#include <fmod_errors.h>
#include <string>
#include "Log.h"
#include "AudioClip.h"

FMOD::System* AudioManager::m_system = nullptr;
FMOD::ChannelGroup* AudioManager::m_channelGroup = nullptr;

bool succeededOrWarn(const char *msg, FMOD_RESULT result)
{
  if (result != FMOD_OK) 
  {
    std::string message = msg;
    message += ": ";
    message += FMOD_ErrorString(result);
    LOG_ERROR("Audio Manager", message.c_str());
    return false;
  }
  return true;
}

void AudioManager::Init()
{
  FMOD_RESULT result;

  // Create the main system object.
  result = FMOD::System_Create(&m_system);
  if (!succeededOrWarn("FMOD: Failed to create system object", result))
    return;

  int driverCount = 0;
  m_system->getNumDrivers(&driverCount);
  if (driverCount == 0)
  {
    LOG_ERROR("Audio Manager", "No audio driver detected");
    return;
  }

  // Initialize FMOD.
  result = m_system->init(512, FMOD_INIT_NORMAL, nullptr);
  if (!succeededOrWarn("FMOD: Failed to initialise system object", result))
    return;

  // Create the channel group.
  result = m_system->createChannelGroup("inGameSoundEffects", &m_channelGroup);
  if (!succeededOrWarn("FMOD: Failed to create in-game sound effects channel group", result))
    return;
}

void AudioManager::Update()
{
  m_system->update();
}

void AudioManager::Exit()
{
  // Clean up.
  m_channelGroup->release();
  m_system->release();
}

FMOD::Sound* AudioManager::Open(const char* audioPath)
{
  FMOD::Sound* sound = nullptr;
  m_system->createSound(audioPath, FMOD_DEFAULT, nullptr, &sound);
  return sound;
}

void AudioManager::Play(AudioClip* audio)
{
  FMOD_RESULT result;
  FMOD::Channel* channel = nullptr;
  result = m_system->playSound(audio->Audio(), nullptr, false, &channel);
  if (!succeededOrWarn("FMOD: Failed to play sound", result))
    return;

  // Assign the channel to the group.
  result = channel->setChannelGroup(m_channelGroup);
  if (!succeededOrWarn("FMOD: Failed to set channel group on", result))
    return;
}

void AudioManager::Stop(AudioClip* audio)
{
}