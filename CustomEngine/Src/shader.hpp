#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "shaderUniform.h"
#include "SphericalHarmonics.h"

enum Primitive_Enum
{
    TriangleStrip = 0,
    Points,
    LineStrip
};

class Transform;
class Framebuffer;
class Material;

struct UniformData
{
  UniformData() : ID(-1), name(), type(GL_INVALID_ENUM), size() {};

  int ID;
  std::string name;
  unsigned int type;
  int size;
};

class Shader
{
public:
  Shader() {};
  Shader(const char* name, const char* vertString, const char* fragString, const char *shaderDir);
  Shader(const char* name, const char* vertString, const char* fragString, const char *geoString, const char *shaderDir);
  Shader(const std::string& name, const std::string& vert, const std::string& frag);

  ~Shader();

  void ExtractUniforms();

  GLuint GetID(void) { return ID; };
  const std::string& GetName(void) { return Name; };
  void SetName(const char* name) { Name = name; };

  void use(void);
  GLint findUniform(const std::string& name) const;
  void setBool(const std::string& name, bool value) const;
  void setInt(const std::string& name, int value) const;
  void setFloat(const std::string& name, float value) const;
  void setFloatArray(const std::string& name, const std::vector<float>& value) const;
  void setVec2(const std::string& name, const glm::vec2 &value) const;
  void setVec3(const std::string& name, const glm::vec3 &value) const;
  void setVec3Array(const std::string& name, const std::vector<glm::vec3>& value) const;
  void setVec4(const std::string& name, const glm::vec4 &value) const;
  void setMat3(const std::string& name, const glm::mat3 &value) const;
  void setMat4(const std::string& name, const glm::mat4 &value) const;
  void setMat4Array(const std::string& name, const std::vector<glm::mat4> &value) const;
  void setSH(const std::string& name, const SH::SH_coeff& sh) const;

  bool FindUniform(const char *name, UniformData& out, const char **outname) const;
  bool GetBool(const char* name, boolUniform&out) const;
  bool GetInt(const char* name, intUniform& out) const;
  bool GetFloat(const char* name, floatUniform& out) const;
  bool GetVec2(const char* name, vec2Uniform& out) const;
  bool GetVec3(const char* name, vec3Uniform& out) const;
  bool GetVec4(const char* name, vec4Uniform& out) const;
  bool GetMat3(const char* name, mat3Uniform&out) const;
  bool GetMat4(const char* name, mat4Uniform& out) const;
  bool GetMat4Array(const char* name, mat4Uniform& out) const;
  bool GetSampler1D(const char* name, tex1DUniform& out) const;
  bool GetSampler2D(const char* name, tex2DUniform& out) const;
  bool GetSampler3D(const char* name, tex3DUniform& out) const;
  bool GetSamplerCube(const char* name, texCubeUniform& out) const;

  void SetBool(const boolUniform& in);
  void SetInt(const intUniform& in);
  void SetFloat(const floatUniform& in);
  void SetVec2(const vec2Uniform& in);
  void SetVec3(const vec3Uniform& in);
  void SetVec4(const vec4Uniform& in);
  void SetMat3(const mat3Uniform& in);
  void SetMat4(const mat4Uniform& in);
  void SetMat4Array(const mat4Uniform& in);
  void SetSampler1D(const tex1DUniform& in);
  void SetSampler2D(const tex2DUniform& in);
  void SetSampler3D(const tex3DUniform& in);
  void SetSamplerCube(const texCubeUniform& in);

  void AddBoolProperty(const boolUniform& in);
  void AddIntProperty(const intUniform& in);
  void AddFloatProperty(const floatUniform& in);
  void AddVec2Property(const vec2Uniform& in);
  void AddVec3Property(const vec3Uniform& in);
  void AddVec4Property(const vec4Uniform& in);
  void AddSampler1DProperty(const tex1DUniform& in);
  void AddSampler2DProperty(const tex2DUniform& in);
  void AddSampler3DProperty(const tex3DUniform& in);
  void AddSamplerCubeProperty(const texCubeUniform& in);

  const std::vector<boolUniform>& GetBoolProperties() const { return boolProperties; };
  const std::vector<intUniform>& GetIntProperties() const { return intProperties; };
  const std::vector<floatUniform>& GetFloatProperties() const { return floatProperties; };
  const std::vector<vec2Uniform>& GetVec2Properties() const { return vec2Properties; };
  const std::vector<vec3Uniform>& GetVec3Properties() const { return vec3Properties; };
  const std::vector<vec4Uniform>& GetVec4Properties() const { return vec4Properties; };
  const std::vector<tex1DUniform>& GetSampler1DProperties() const { return sampler1DProperties; };
  const std::vector<tex2DUniform>& GetSampler2DProperties() const { return sampler2DProperties; };
  const std::vector<tex3DUniform>& GetSampler3DProperties() const { return sampler3DProperties; };
  const std::vector<texCubeUniform>& GetSamplerCubeProperties() const { return samplerCubeProperties; };

  const std::unordered_map<std::string, UniformData>& GetUniformTable() const { return uniforms; };

  void BindToUniformBlock(const std::string& name, GLuint bindingPoint) const;

  void Reload();
  void SetID(unsigned int id);

  void DrawDebugMenu();

  static void GeneratePreview(Shader* shader, Framebuffer* preview, Shader* previewShader, Transform *transform) {};
  static void DrawResourceCreator(bool& openFlag);

protected:
  GLuint ID;
  std::string Name;
  std::string VertDir;
  std::string FragDir;
  std::string ShaderDir;
  bool usingCustomShader;

  std::unordered_map<std::string, UniformData> uniforms;

  std::vector<boolUniform> boolProperties;
  std::vector<intUniform> intProperties;
  std::vector<floatUniform> floatProperties;
  std::vector<vec2Uniform> vec2Properties;
  std::vector<vec3Uniform> vec3Properties;
  std::vector<vec4Uniform> vec4Properties;
  std::vector<tex1DUniform> sampler1DProperties;
  std::vector<tex2DUniform> sampler2DProperties;
  std::vector<tex3DUniform> sampler3DProperties;
  std::vector<texCubeUniform> samplerCubeProperties;
};

//Compute shaders require a different constructor since they are simpler and stored elsewhere
class ComputeShader : public Shader 
{
public:
  ComputeShader(const char* filepath) { setup(filepath); };

  bool setup(const std::string &computePath);
  void dispatch(unsigned int x, unsigned int y = 1, unsigned int z = 1) const;
};

GLuint CompileShaders(const char *name, const char* vertexString, const char* fragmentString, const char* geoString = nullptr);

///////////////////
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

///////////////////
GLuint LoadPipeline(const char * vertex_file_path,const char * fragment_file_path, GLuint * programIDs );

// Load shaders where multiple shader files == one complete shader
// Show how code can be reused across shaders
GLuint LoadMultiShaders(const char *vertex_file_path, const char *fragment_file_path,
                        const char *geom_file_path, Primitive_Enum  out_primitive_type );


#endif
