#pragma once
#ifndef IBLMAP_H
#define IBLMAP_H

class CubeMap;

class IBLMap
{
public:
  IBLMap();

  void Bind(int index);

private:
  CubeMap *Irradiance;
  CubeMap *Prefilter;
};

#endif