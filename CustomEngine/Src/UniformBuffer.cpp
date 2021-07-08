#include "UniformBuffer.h"
#include "shader.hpp"

UniformBuffer::UniformBuffer(GLuint _blockSize, GLuint _bindingPoint) : blockSize(_blockSize), bindingPoint(_bindingPoint)
{
  glGenBuffers(1, &UBO);
  glBindBuffer(GL_UNIFORM_BUFFER, UBO); 

  glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_UNIFORM_BUFFER, bindingPoint, UBO, 0, blockSize);

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

UniformBuffer::~UniformBuffer()
{
  glDeleteBuffers(1, &UBO);
}

void UniformBuffer::Bind() const
{
  glBindBuffer(GL_UNIFORM_BUFFER, UBO);
}

void UniformBuffer::setBool(bool value, GLuint &dataOffset) const
{
  int b = value; // bools in GLSL are represented as 4 bytes, so we store it in an integer
  glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, 4, &b);
  dataOffset += 4;
}
void UniformBuffer::setInt(int value, GLuint &dataOffset) const
{
  glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, 4, &value);
  dataOffset += 4;
}
void UniformBuffer::setFloat(float value, GLuint &dataOffset) const
{
  glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, 4, &value);
  dataOffset += 4;
}
void UniformBuffer::setVec2(const glm::vec2& value, GLuint &dataOffset) const
{
  glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, 8, &value[0]);
  dataOffset += 8;
}
void UniformBuffer::setVec3(const glm::vec3& value, GLuint &dataOffset) const
{
  glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, 16, &value[0]);
  dataOffset += 16;
}
void UniformBuffer::setVec4(const glm::vec4& value, GLuint &dataOffset) const
{
  glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, 16, &value[0]);
  dataOffset += 16;
}
void UniformBuffer::setMat3(const glm::mat3& value, GLuint &dataOffset) const
{
  glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, 16*4, &value[0][0]);
  dataOffset += 16*3;
}
void UniformBuffer::setMat4(const glm::mat4& value, GLuint &dataOffset) const
{
  glBufferSubData(GL_UNIFORM_BUFFER, dataOffset, 16*4, &value[0][0]);
  dataOffset += 16 * 4;
}