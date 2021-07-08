#pragma once
#ifndef GAMESTATE_H
#define GAMESTATE_H

class GameState
{
public:
  void Init();
  void Exit();

  void Pause();
  void Resume();

  void Update();
  void FixedUpdate();
  void Draw();

private:

};

#endif