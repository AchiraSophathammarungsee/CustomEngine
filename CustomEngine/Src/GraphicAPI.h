#pragma once
#ifndef GRAPHICAPI_H
#define GRAPHICAPI_H

namespace GraphicAPI
{
  static unsigned int GL_VersionMajor;
  static unsigned int GL_VersionMinor;

#define DEFERRED_SHADING
#define TRANSPARENT_SHADOW
#ifdef TRANSPARENT_SHADOW
  #define COLOR_STOCHASTIC_SHADOW
#endif
//#define MSAA_ON
}

#endif