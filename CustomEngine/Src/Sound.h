#ifndef SOUND_H
#define SOUND_H
#include "Object.h"
#include <fmod.h>

class Sound : public Object
{
public:
  Sound();
  ~Sound();

  void DrawDebugMenu() override;

private:

};

#endif