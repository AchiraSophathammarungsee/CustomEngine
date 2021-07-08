#ifndef UNIFORMBUFFER_H
#define UNIFORMBUFFER_H

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Shader;

class UniformBuffer
{
public:
  UniformBuffer(GLuint blockSize, GLuint bindingPoint);
  ~UniformBuffer();

  void Bind() const;
  void setBool(bool value, GLuint &dataOffset) const;
  void setInt(int value, GLuint &dataOffset) const;
  void setFloat(float value, GLuint &dataOffset) const;
  void setVec2(const glm::vec2& value, GLuint &dataOffset) const;
  void setVec3(const glm::vec3& value, GLuint &dataOffset) const;
  void setVec4(const glm::vec4& value, GLuint &dataOffset) const;
  void setMat3(const glm::mat3& value, GLuint &dataOffset) const;
  void setMat4(const glm::mat4& value, GLuint &dataOffset) const;

private:
  GLuint UBO;
  GLuint bindingPoint;
  GLuint blockSize;
};

#endif