#include <cstdio>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <cstdlib>
#include <cstring>

#include <glad/glad.h>

#include "shader.hpp"
#include "EngineCore.h"
#include "Log.h"
#include "ResourceManager.h"
#include "FileDialog.h"
#include "ShaderParser.h"

Shader::Shader(const std::string& name, const std::string& vert, const std::string& frag) : Name(name), ID(0), VertDir(vert), FragDir(frag), usingCustomShader(false)
{
  ID = LoadShaders(vert.c_str(), frag.c_str());
  ExtractUniforms();
}

Shader::Shader(const char* name, const char* vertString, const char* fragString, const char* shaderDir) : Name(name), ID(-1), usingCustomShader(true), ShaderDir(shaderDir)
{
  ID = CompileShaders(name, vertString, fragString, nullptr);
  ExtractUniforms();
}

Shader::Shader(const char* name, const char* vertString, const char* fragString, const char* geoString, const char* shaderDir) : Name(name), ID(-1), usingCustomShader(true), ShaderDir(shaderDir)
{
  ID = CompileShaders(name, vertString, fragString, geoString);
  ExtractUniforms();
}

Shader::~Shader()
{
  glDeleteProgram(ID);
}

void Shader::use(void) { glUseProgram(ID); }

GLint Shader::findUniform(const std::string& name) const
{
  auto it = uniforms.find(name);
  if (it == uniforms.end())
  {
    LOG_ERROR_S("Shader", "[{}] trying to set invalid uniform {}", Name, name);
    return -1;
  }

  //GLint loc = glGetUniformLocation(ID, name.c_str());
  /*if (loc == -1)
  {
    printf("[%s]: Couldn't find uniform: %s\n", Name.c_str(), name.c_str());
  }*/

  return it->second.ID;
}

void Shader::ExtractUniforms()
{
  //GLint uniformCount;
  //GLenum type;
  //const GLsizei bufSize = 16; // max name length
  //GLsizei nameLength;
  //GLint varSize;

  //GLchar name[bufSize];

  //glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &uniformCount);

  //for (GLint i = 0; i < uniformCount; i++)
  //{
  //  glGetActiveUniform(ID, i, bufSize, &nameLength, &varSize, &type, name);

  //  UniformData uniform;
  //  uniform.ID = i;
  //  uniform.name = name;
  //  uniform.size = varSize;
  //  uniform.type = type;

  //  uniforms.push_back(uniform);
  //  //std::cout << "Uniform #" << i << " Type: " << type << " Name: " << name << std::endl;
  //}

  GLint numActiveAttribs = 0;
  GLint numActiveUniforms = 0;
  glGetProgramInterfaceiv(ID, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numActiveAttribs);
  glGetProgramInterfaceiv(ID, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numActiveUniforms);

  const GLenum properties[5] = { GL_BLOCK_INDEX, GL_TYPE, GL_NAME_LENGTH, GL_LOCATION, GL_ARRAY_SIZE };
  for (int unif = 0; unif < numActiveUniforms; ++unif)
  {
    GLint values[5];
    glGetProgramResourceiv(ID, GL_UNIFORM, unif, 5, properties, 5, NULL, values);

    // Skip any uniforms that are in a block.
    if (values[0] != -1)
      continue;

    // Get the name. Must use a std::vector rather than a std::string for C++03 standards issues.
    // C++11 would let you use a std::string directly.
    std::vector<char> nameData(values[2]);
    glGetProgramResourceName(ID, GL_UNIFORM, unif, nameData.size(), NULL, &nameData[0]);

    UniformData uniform;
    uniform.ID = values[3];
    uniform.name = std::string(nameData.begin(), nameData.end() - 1);
    uniform.size = values[4];
    uniform.type = values[1];

    uniforms[uniform.name] = uniform;
  }
}

void Shader::setBool(const std::string& name, bool value) const
{
	glUniform1i(findUniform(name), (int)value);
}
void Shader::setInt(const std::string& name, int value) const
{
	glUniform1i(findUniform(name), value);
}
void Shader::setFloat(const std::string& name, float value) const
{
	glUniform1f(findUniform(name), value);
}
void Shader::setFloatArray(const std::string& name, const std::vector<float>& value) const
{
  glUniform1fv(findUniform(name), value.size(), &value[0]);
}
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
  glUniform2fv(findUniform(name), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
  glUniform3fv(findUniform(name), 1, &value[0]);
}
void Shader::setVec3Array(const std::string& name, const std::vector<glm::vec3>& value) const
{
  glUniform3fv(findUniform(name), value.size(), &value[0][0]);
}
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
  glUniform4fv(findUniform(name), 1, &value[0]);
}
void Shader::setMat3(const std::string& name, const glm::mat3& value) const
{
  glUniformMatrix3fv(findUniform(name), 1, GL_FALSE, &value[0][0]);
}
void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
  glUniformMatrix4fv(findUniform(name), 1, GL_FALSE, &value[0][0]);
}
void Shader::setMat4Array(const std::string& name, const std::vector<glm::mat4>& value) const
{
  glUniformMatrix4fv(findUniform(name), value.size(), GL_FALSE, &value[0][0][0]);
}
void Shader::setSH(const std::string& name, const SH::SH_coeff& sh) const
{
  glUniform3fv(findUniform(name), 9, &sh[0][0]);
}

void Shader::BindToUniformBlock(const std::string& name, GLuint bindingPoint) const
{
  unsigned int index = glGetUniformBlockIndex(ID, name.c_str());
  glUniformBlockBinding(ID, index, bindingPoint);
}

bool Shader::FindUniform(const char* name, UniformData& out, const char** outname) const
{
  //UniformData val = uniformTable.Find(name);
  auto it = uniforms.find(name);
  if (it == uniforms.end())
  {
    return false;
  }

  out = it->second;
  *outname = it->first.c_str();
  return true;
}

bool Shader::GetBool(const char* name, boolUniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_BOOL)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetInt(const char* name, intUniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_INT)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetFloat(const char* name, floatUniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_FLOAT)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetVec2(const char* name, vec2Uniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_FLOAT_VEC2)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetVec3(const char* name, vec3Uniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_FLOAT_VEC3)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetVec4(const char* name, vec4Uniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_FLOAT_VEC4)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetMat3(const char* name, mat3Uniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_FLOAT_MAT3)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetMat4(const char* name, mat4Uniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_FLOAT_MAT4)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetMat4Array(const char* name, mat4Uniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) && val.type == GL_FLOAT_MAT4 && val.size > 1)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetSampler1D(const char* name, tex1DUniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) == false) return false;

  if (val.type == GL_SAMPLER_1D)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetSampler2D(const char* name, tex2DUniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) == false) return false;

  if (val.type == GL_SAMPLER_2D)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetSampler3D(const char* name, tex3DUniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) == false) return false;

  if (val.type == GL_SAMPLER_3D)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}
bool Shader::GetSamplerCube(const char* name, texCubeUniform& out) const
{
  UniformData val;
  if (FindUniform(name, val, &out.name) == false) return false;

  if (val.type == GL_SAMPLER_CUBE)
  {
    out.id = val.ID;
    return true;
  }
  return false;
}

void Shader::SetBool(const boolUniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform1i(in.id, (int)in.value);
}
void Shader::SetInt(const intUniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform1i(in.id, in.value);
}
void Shader::SetFloat(const floatUniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform1f(in.id, in.value);
}
void Shader::SetVec2(const vec2Uniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform2fv(in.id, 1, &in.value[0]);
}
void Shader::SetVec3(const vec3Uniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform3fv(in.id, 1, &in.value[0]);
}
void Shader::SetVec4(const vec4Uniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform4fv(in.id, 1, &in.value[0]);
}
void Shader::SetMat3(const mat3Uniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniformMatrix3fv(in.id, 1, GL_FALSE, &in.value[0][0]);
}
void Shader::SetMat4(const mat4Uniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniformMatrix4fv(in.id, 1, GL_FALSE, &in.value[0][0]);
}
void Shader::SetMat4Array(const mat4Uniform& in)
{
#ifdef EDITOR_MODE
  LOG_WARN("Shader", "Cannot set mat4 array yet");
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  //glUniformMatrix3fv(in.id, 1, GL_FALSE, &in.value[0][0]);
  //glUniformMatrix4fv(in.id, value.size(), GL_FALSE, &value[0][0][0]);
}
void Shader::SetSampler1D(const tex1DUniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform1i(in.id, in.bindID);
}
void Shader::SetSampler2D(const tex2DUniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform1i(in.id, in.bindID);
}
void Shader::SetSampler3D(const tex3DUniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform1i(in.id, in.bindID);
}
void Shader::SetSamplerCube(const texCubeUniform& in)
{
#ifdef EDITOR_MODE
  if (in.id == -1)
  {
    LOG_ERROR_S("Shader", "[{}] Trying to set uniform {} with invalid id", Name, in.name);
    return;
  }
#endif
  glUniform1i(in.id, in.bindID);
}

void Shader::Reload()
{
  GLuint id = 0;
  if (usingCustomShader)
  {
    ShaderParser::ReloadShader(this, ShaderDir.c_str(), Name.c_str());
  }
  else
  {
    id = LoadShaders(VertDir.c_str(), FragDir.c_str());
    SetID(id);
  }
}

void Shader::SetID(unsigned int id)
{
  if (id)
  {
    glDeleteProgram(ID);
    ID = id;
  }
}

void Shader::AddBoolProperty(const boolUniform& in)
{
  boolProperties.push_back(in);
}
void Shader::AddIntProperty(const intUniform& in)
{
  intProperties.push_back(in);
}
void Shader::AddFloatProperty(const floatUniform& in)
{
  floatProperties.push_back(in);
}
void Shader::AddVec2Property(const vec2Uniform& in)
{
  vec2Properties.push_back(in);
}
void Shader::AddVec3Property(const vec3Uniform& in)
{
  vec3Properties.push_back(in);
}
void Shader::AddVec4Property(const vec4Uniform& in)
{
  vec4Properties.push_back(in);
}
void Shader::AddSampler1DProperty(const tex1DUniform& in)
{
  sampler1DProperties.push_back(in);
}
void Shader::AddSampler2DProperty(const tex2DUniform& in)
{
  sampler2DProperties.push_back(in);
}
void Shader::AddSampler3DProperty(const tex3DUniform& in)
{
  sampler3DProperties.push_back(in);
}
void Shader::AddSamplerCubeProperty(const texCubeUniform& in)
{
  samplerCubeProperties.push_back(in);
}

void Shader::DrawDebugMenu()
{
  ImGui::Text("Name: %s", Name.c_str());
  ImGui::Text("Vertex Path: %s", VertDir.c_str());
  ImGui::Text("Fragment Path: %s", FragDir.c_str());
  ImGui::Text("Shader Path: %s", ShaderDir.c_str());
  ImGui::Text("ID:% i", ID);
  if (ImGui::Button("Reload"))
  {
    Reload();
  }

  static const char* TypeNames[] = { "boolean", "int", "uint", "float", "double", "vec2f", "vec3f", "vec4f", "mat3f", "mat4f", "GL_SAMPLER_1D", "sampler2D", "sampler3D", "samplerCube" };
  static unsigned int TypeEnums[] = { GL_BOOL, GL_INT, GL_UNSIGNED_INT, GL_FLOAT, GL_DOUBLE, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_FLOAT_MAT3, GL_FLOAT_MAT4, GL_SAMPLER_1D, GL_SAMPLER_2D, GL_SAMPLER_3D, GL_SAMPLER_CUBE };
  if (ImGui::CollapsingHeader("Uniform list"))
  {
    ImGui::Indent(10.f);

    for (auto& it : uniforms)
    {
      std::string typeStr = "unknown";

      for (int i = 0; i < 14; i++)
      {
        if (it.second.type == TypeEnums[i])
        {
          typeStr = TypeNames[i];
          break;
        }
      }

      ImGui::Text("[%d] Name: %s, Type: %s (0x%x), Size: %d", it.second.ID, it.first.c_str(), typeStr.c_str(), it.second.type, it.second.size);
    }

    static bool validUni = false;
    static char uniName[256] = {0};
    static UniformData testu;
    ImGui::InputText("sample uniform name", uniName, 256);
    if (ImGui::Button("sample uniform"))
    {
      const char* dummy;
      validUni = FindUniform(uniName, testu, &dummy);
    }
    if (validUni)
    {
      ImGui::Text("[%d] Name: %s, Type: 0x%x, Size: %d", testu.ID, testu.name.c_str(), testu.type, testu.size);
    }
    else
    {
      ImGui::Text("Error: no uniform with name %s", uniName);
    }

    /*for (UniformData &u : uniforms)
    {
      std::string typeStr = "unknown";

      for (int i = 0; i < 14; i++)
      {
        if (u.type == TypeEnums[i])
        {
          typeStr = TypeNames[i];
          break;
        }
      }

      ImGui::Text("[%d] Name: %s, Type: %s (0x%x), Size: %d", u.ID, u.name.c_str(), typeStr.c_str(), u.type, u.size);
    }*/

    ImGui::Unindent(10.f);
  }
}

void Shader::DrawResourceCreator(bool& openFlag)
{
  static char shaderNameBuffer[256] = "unnamed";
  static char vertexPathBuffer[270] = "../Common/shaders/";
  static char fragmentPathBuffer[270] = "../Common/shaders/";

  ImGui::SetNextWindowSize(ImVec2(300, 200));
  ImGui::Begin("New shader", &openFlag);

  /*ImGui::InputText("Shader Name", shaderNameBuffer, 255);
  ImGui::InputText("Vertex Path", vertexPathBuffer, 255);
  ImGui::InputText("Fragment Path", shaderNameBuffer, 255);*/

  if (ImGui::Button("Open File..."))
  {
    std::string path = FileDialog::OpenFile("Shader Files\0*.shader\0");
    strcpy_s(vertexPathBuffer, 270, path.c_str());
  }
  ImGui::InputText("Shader name", shaderNameBuffer, 255);
  ImGui::InputText("Shader path", vertexPathBuffer, 270);

  if (ImGui::Button("Create Shader"))
  {
    ResourceManager::Add<Shader>(ShaderParser::ReadShader(vertexPathBuffer, shaderNameBuffer), shaderNameBuffer);
    openFlag = false;
  }

  ImGui::End();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

GLuint CompileShaders(const char* name, const char* vertexString, const char* fragmentString, const char *geoString)
{
  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint GeometryShaderID = 0;
  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  LOG_TRACE_S("Shader", "Compiling shader: {}'s vertex shader", name);

  char const* VertexSourcePointer = vertexString;
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);

    LOG_ERROR("Shader", &VertexShaderErrorMessage[0]);
  }

  // compile geometru shader if exist
  if (geoString != nullptr)
  {
    LOG_TRACE_S("Shader", "Compiling shader: {}'s geometry shader", name);

    GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(GeometryShaderID, 1, &geoString, nullptr);
    glCompileShader(GeometryShaderID);

    // Check Vertex Shader
    glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) 
    {
      std::vector<char> GeoShaderErrorMessage(InfoLogLength + 1);
      glGetShaderInfoLog(GeometryShaderID, InfoLogLength, nullptr, &GeoShaderErrorMessage[0]);

      LOG_ERROR("Shader", &GeoShaderErrorMessage[0]);
    }
  }

  // Compile Fragment Shader
  LOG_TRACE_S("Shader", "Compiling shader: {}'s fragment shader", name);

  char const* FragmentSourcePointer = fragmentString;
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) 
  {
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);

    LOG_ERROR("Shader", &FragmentShaderErrorMessage[0]);
  }

  // Link the program
  LOG_TRACE("Shader", "Linking program");

  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  if (geoString != nullptr)
    glAttachShader(ProgramID, GeometryShaderID);

  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  if (InfoLogLength > 0) {
    std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
    glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);

    LOG_ERROR("Shader", &ProgramErrorMessage[0]);
  }

  glDetachShader(ProgramID, VertexShaderID);
  glDetachShader(ProgramID, FragmentShaderID);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  if (geoString != nullptr)
  {
    glDetachShader(ProgramID, GeometryShaderID);
    glDeleteShader(GeometryShaderID);
  }

  return ProgramID;
}

GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
  {
		std::string Line;
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
  else
  {
    LOG_TRACE_S("Shader", "Couldn't open: {}", vertex_file_path);
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line;
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
  LOG_TRACE_S("Shader", "Compiling shader: {}", vertex_file_path);

	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);

    LOG_ERROR("Shader", &VertexShaderErrorMessage[0]);
	}


	// Compile Fragment Shader
  LOG_TRACE_S("Shader", "Compiling shader: {}", fragment_file_path);

	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);

    LOG_ERROR("Shader", &FragmentShaderErrorMessage[0]);
	}


	// Link the program
  LOG_TRACE("Shader", "Linking program");

	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);

    LOG_ERROR("Shader", &ProgramErrorMessage[0]);
	}

	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

	return ProgramID;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

GLuint LoadPipeline(const char * vertex_file_path,const char * fragment_file_path, GLuint * programIDs )
{
    // Hardcoding the values to 2
    
    // Vertex Shader + Program
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line;
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }else{
        LOG_ERROR_S("Shader", "Couldn't open: {}", vertex_file_path);
        return 0;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    LOG_TRACE_S("Shader", "Compiling shader: {}", vertex_file_path);

    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , nullptr);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);

        LOG_ERROR("Shader", &VertexShaderErrorMessage[0]);
    }

    // Link the program
    LOG_TRACE("Shader", "Linking program");
    programIDs[0] = glCreateProgram();
    glAttachShader(programIDs[0], VertexShaderID);
    
    // IMP: Do this BEFORE linking the progam
    glProgramParameteri(programIDs[0],
                        GL_PROGRAM_SEPARABLE,
                        GL_TRUE );
    
    glLinkProgram(programIDs[0]);
    
    // Check the program
    glGetProgramiv(programIDs[0], GL_LINK_STATUS, &Result);
    glGetProgramiv(programIDs[0], GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(programIDs[0], InfoLogLength, nullptr, &ProgramErrorMessage[0]);

        LOG_ERROR("Shader", &ProgramErrorMessage[0]);
    }
    
    // Fragment Shader + Program
    
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line;
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    
    Result = GL_FALSE;
    InfoLogLength = 0;
    
    // Compile Fragment Shader
    LOG_TRACE_S("Shader", "Compiling shader: {}", fragment_file_path);

    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , nullptr);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);

        LOG_ERROR("Shader", &FragmentShaderErrorMessage[0]);
    }
    
    // Link the program
    LOG_TRACE("Shader", "Linking program");

    programIDs[1] = glCreateProgram();
    glAttachShader(programIDs[1], FragmentShaderID);
    
    // IMP: Do this BEFORE linking the progam
    glProgramParameteri(programIDs[1],
                        GL_PROGRAM_SEPARABLE,
                        GL_TRUE );
    
    glLinkProgram(programIDs[1]);
    
    GLuint pipeLineID;
    glGenProgramPipelines( 1, &pipeLineID );

    glUseProgramStages( pipeLineID, GL_VERTEX_SHADER_BIT, programIDs[0]);
    glUseProgramStages( pipeLineID, GL_FRAGMENT_SHADER_BIT, programIDs[1]);

    return pipeLineID;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

GLuint LoadMultiShaders(const char *vertex_file_path, const char *fragment_file_path,
                        const char *geom_file_path,
                        Primitive_Enum out_primitive_type)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;

    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open())
    {
        std::string Line;
        while (getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    } else
    {
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n",
               vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open())
    {
        std::string Line;
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }


    // array of char * to hold the strings
    std::vector<const char *> shaderStrings;
    shaderStrings.clear();

    // First string
    std::string geometryShaderHeader("#version 450 core\n"
                                     "\n"
                                     "const int maxVerts = 4;");
    shaderStrings.push_back(geometryShaderHeader.c_str());

    // Second string
    std::string geometryInOut = "// Type of primitive input\n  // from VS (CPU)\n layout (points) in;";

    std::string temp;
    switch (out_primitive_type)
    {
        case TriangleStrip:
            temp = "triangle_strip";
            break;

        case LineStrip:
            temp = "line_strip";
            break;

        case Points:
            temp = "points";
            break;
    };

    geometryInOut += "layout (" + temp + ", max_vertices = maxVerts) out;\n";
    shaderStrings.push_back(geometryInOut.c_str());

    // Third string
    // Read the Geometry Shader from the file
    std::string GeometryShaderCode;
    std::ifstream GeometryShaderStream(geom_file_path, std::ios::in);
    if (GeometryShaderStream.is_open())
    {
        std::string Line;
        while (getline(GeometryShaderStream, Line))
            GeometryShaderCode += "\n" + Line;
        GeometryShaderStream.close();
    }

    shaderStrings.push_back(GeometryShaderCode.c_str());
    //////////////////////////////////////////////////////////


    GLint Result = GL_FALSE;
    int InfoLogLength;


    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const *VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> VertexShaderErrorMessage(static_cast<unsigned long>(InfoLogLength + 1));
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }


    // Compile Geometry Shader
    printf("Compiling shader : %s\n", geom_file_path);
    char const **GeometrySourcePointer = shaderStrings.data(); //GeometryShaderCode.c_str();
    glShaderSource(GeometryShaderID, 3, GeometrySourcePointer, nullptr);
    glCompileShader(GeometryShaderID);

    // Check Fragment Shader
    glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> GeometryShaderErrorMessage((unsigned long) (InfoLogLength + 1));
        glGetShaderInfoLog(GeometryShaderID, InfoLogLength, nullptr, &GeometryShaderErrorMessage[0]);
        printf("%s\n", &GeometryShaderErrorMessage[0]);
    }


    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const *FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> FragmentShaderErrorMessage(static_cast<unsigned long>(InfoLogLength + 1));
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, GeometryShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0)
    {
        std::vector<char> ProgramErrorMessage(static_cast<unsigned long>(InfoLogLength + 1));
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }


    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, GeometryShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(GeometryShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

bool ComputeShader::setup(const std::string &computePath) 
{
  //Getting the compute shader code from the text file at file path
  std::string shaderFolderPath = "../assets/shaders/ComputeShaders/";
  std::string computeCode;
  std::ifstream cShaderFile(computePath);
  std::stringstream cShaderStream;

  Name = computePath;

  //Check if shader files exist
  if (!cShaderFile.good()) 
  {
    LOG_ERROR_S("Computer shader", "Couldn't find compute shader file: {} in shaders folder.", computePath);
    return false;
  }
  else 
  { //Compute Shader Exists
    cShaderStream << cShaderFile.rdbuf();

    //Close Files
    cShaderFile.close();

    //Passing code from string stream to string
    computeCode = cShaderStream.str();
    const char* cShaderCode = computeCode.c_str();

    //OpenGL initialization
    int computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &cShaderCode, NULL);
    glCompileShader(computeShader);
    int success;
    char infoLog[512];
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
      LOG_ERROR_S("Computer shader", "[{}]: {}", Name, infoLog);
      return false;
    }

    //Linking shaders
    ID = glCreateProgram();
    glAttachShader(ID, computeShader);
    glLinkProgram(ID);

    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) 
    {
      glGetProgramInfoLog(ID, 512, NULL, infoLog);
      LOG_ERROR_S("Computer shader", "[{}]: {}", Name, infoLog);
      return false;
    }

    //Deleting shaders
    glDeleteShader(computeShader);

    ExtractUniforms();

    return true;
  }
}

//Shorthand for dispatch compute with some default parameter values
void ComputeShader::dispatch(unsigned int x, unsigned int y, unsigned int z) const 
{
  glDispatchCompute(x, y, z);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}
