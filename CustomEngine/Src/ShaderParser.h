#pragma once
#ifndef SHADERPARSER_H
#define SHADERPARSER_H

#include <string>
#include <vector>

class Shader;

namespace ShaderParser
{
  Shader* ReadShader(const char *filePath, const char *shaderName);
  bool ReloadShader(Shader *shader, const char* filePath, const char* shaderName);

  void GlobalDefine(const char *define);

  bool ExtractShader(const char* filePath, const char* shaderName, std::string &vertOut, std::string &fragOut, std::string& geoOut, std::vector<std::string>& uniformsNames, std::vector<std::string>& uniformsTypes, std::vector<std::string>& uniformsValues);
}

#endif