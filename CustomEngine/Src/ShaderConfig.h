#pragma once
#ifndef SHADERCONFIG_H
#define SHADERCONFIG_H

#include <glad/glad.h>

namespace ShaderConfig
{
  enum CullModes { CULL_Back, CULL_Front, CULL_Off, CULL_COUNT };
  enum PolygonMode { Fill = GL_FILL, Line = GL_LINE };
  enum BlendMode { BLEND_Opaque, BLEND_Masked, BLEND_Translucent, BLEND_Additive, BLEND_Multiply, BLEND_Modulate, BLEND_COUNT };
  enum DepthDrawMode { DEPTHDRAW_ON, DEPTHDRAW_OFF };
  enum DepthTestMode { DEPTHTEST_ON, DEPTHTEST_OFF };

  static const char* CullModeString[] = { "Back", "Front", "Off" };
  static const char* PolygonModeString[] = { "Fill", "Line" };
  static const char* BlendModeString[] = { "Opaque", "Masked", "Translucent", "Additive", "Multiply", "Modulate" };
}

#endif
