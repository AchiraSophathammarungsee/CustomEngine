#pragma once
#ifndef MESHENUM_H
#define MESHENUM_H

#include <glad/glad.h>

enum PrimitiveMode
{
  PM_Points = GL_POINTS,
  PM_Lines = GL_LINES,
  PM_Line_Loop = GL_LINE_LOOP,
  PM_Line_Strip = GL_LINE_STRIP,
  PM_Triangles = GL_TRIANGLES,
  PM_Triangle_Strip = GL_TRIANGLE_STRIP,
  PM_Triangle_Fan = GL_TRIANGLE_FAN,
  PM_COUNT
};

enum NormalBaseMode
{
  NBM_None, NBM_Vertex, NBM_Face
};

#endif