#include "ShaderParser.h"
#include "shader.hpp"
#include <fstream>
#include <sstream>
#include "EngineCore.h"
#include <unordered_map>
#include "ResourceManager.h"

namespace ShaderParser
{
  enum uniformTypes { BOOL, INT, FLOAT, DOUBLE, VEC2, VEC3, VEC4, COLOR3, COLOR4, SAMPLER1D, SAMPLER2D, SAMPLER3D, SAMPLERCUBE, COUNT };
  static const char* uniformTypesStr[COUNT] = { "bool", "int", "float", "double", "vec2", "vec3", "vec4", "color3", "color4", "sampler1D", "sampler2D", "sampler3D", "samplerCube" };

  static std::unordered_map<std::string, uniformTypes> strToType = { {"bool", BOOL}, {"int", INT}, {"float",FLOAT}, {"double",DOUBLE}, {"vec2",VEC2}, {"vec3",VEC3}, {"vec4",VEC4}, 
                                                                     {"color3",COLOR3}, {"color4",COLOR4}, {"sampler1D",SAMPLER1D}, {"sampler2D",SAMPLER2D}, {"sampler3D",SAMPLER3D}, 
                                                                     {"samplerCube",SAMPLERCUBE} };

  static std::unordered_map<std::string, bool> defineList;

  void GlobalDefine(const char* define)
  {
    defineList[define] = true;
  }

  bool IsDefined(const char* define, const std::unordered_map<std::string, bool> &localDefines)
  {
    auto it = defineList.find(define);
    auto it2 = localDefines.find(define);
    return it != defineList.end() || it2 != localDefines.end();
  }

  bool convertToBool(const std::string& str)
  {
    if (str == "false") return false;
    else if (str == "true") return true;

    ASSERT(false, "Shader Parser", "{} is not a valid bool value", str);
    return false;
  }
  int convertToInt(const std::string& str)
  {
    return std::stoi(str);
  }
  float convertToFloat(const std::string& str)
  {
    return std::stof(str);
  }
  float convertToDouble(const std::string& str)
  {
    return std::stod(str);
  }
  glm::vec2 convertToVec2(const std::string& str)
  {
    std::string valStr;
    glm::vec2 val;
    int count = 0;
    for (const char& c : str)
    {
      if (c == ',')
      {
        ASSERT(count < 2, "Shader Parser", "{} is not a valid vec2 value, exceeds 2 floats", str);

        val[count] = std::stof(valStr);
        count++;
        valStr.clear();
      }
      else
      {
        valStr += c;
      }
    }
    val[count] = std::stof(valStr);
    ASSERT(count == 1, "Shader Parser", "{} is not a valid vec2 value, not enough values count", str);

    return val;
  }
  glm::vec3 convertToVec3(const std::string& str)
  {
    std::string valStr;
    glm::vec3 val;
    int count = 0;
    for (const char& c : str)
    {
      if (c == ',')
      {
        ASSERT(count < 3, "Shader Parser", "{} is not a valid vec3 value, exceeds 3 floats", str);

        val[count] = std::stof(valStr);
        count++;
        valStr.clear();
      }
      else
      {
        valStr += c;
      }
    }
    val[count] = std::stof(valStr);
    ASSERT(count == 2, "Shader Parser", "{} is not a valid vec3 value, not enough values count", str);
    return val;
  }
  glm::vec4 convertToVec4(const std::string& str)
  {
    std::string valStr;
    glm::vec4 val;
    int count = 0;
    for (const char& c : str)
    {
      if (c == ',')
      {
        ASSERT(count < 4, "Shader Parser", "{} is not a valid vec4 value, exceeds 4 floats", str);

        val[count] = std::stof(valStr);
        count++;
        valStr.clear();
      }
      else
      {
        valStr += c;
      }
    }
    val[count] = std::stof(valStr);
    ASSERT(count == 3, "Shader Parser", "{} is not a valid vec4 value, not enough values count", str);
    return val;
  }

  void convertToTypes(Shader* shader, const std::string& valueStr, const std::string& typeStr, const std::string& nameStr)
  {
    auto it = strToType.find(typeStr);
    if (it == strToType.end())
    {
      LOG_ERROR_S("Shader Parser", "{}'s type: {} is not a valid uniform type", nameStr,typeStr);
      return;
    }

    uniformTypes type = it->second;
    std::string propertyName = "property.";
    propertyName += nameStr;

    switch (type)
    {
    case(BOOL):
    {
      boolUniform u;
      u.value = convertToBool(valueStr);
      shader->GetBool(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddBoolProperty(u);
      }
      break;
    }
    case(INT):
    {
      intUniform u;
      u.value = convertToInt(valueStr);
      shader->GetInt(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddIntProperty(u);
      }
      break;
    }
    case(FLOAT):
    {
      floatUniform u;
      u.value = convertToFloat(valueStr);
      shader->GetFloat(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddFloatProperty(u);
      }
      break;
    }
    case(VEC2):
    {
      vec2Uniform u;
      u.value = convertToVec2(valueStr);
      shader->GetVec2(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddVec2Property(u);
      }
      break;
    }
    case(VEC3):
    {
      vec3Uniform u;
      u.value = convertToVec3(valueStr);
      shader->GetVec3(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddVec3Property(u);
      }
      break;
    }
    case(VEC4):
    {
      vec4Uniform u;
      u.value = convertToVec4(valueStr);
      shader->GetVec4(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddVec4Property(u);
      }
      break;
    }
    case(COLOR3):
    {
      vec3Uniform u;
      u.value = convertToVec3(valueStr);
      u.isColor = true;
      shader->GetVec3(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddVec3Property(u);
      }
      break;
    }
    case(COLOR4):
    {
      vec4Uniform u;
      u.value = convertToVec4(valueStr);
      u.isColor = true;
      shader->GetVec4(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddVec4Property(u);
      }
      break;
    }
    case(SAMPLER1D):
    {
      tex1DUniform u;
      u.value = nullptr;
      u.bindID = convertToInt(valueStr);
      shader->GetSampler1D(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddSampler1DProperty(u);
      }
      break;
    }
    case(SAMPLER2D):
    {
      tex2DUniform u;
      u.value = ResourceManager::Get<Texture>("default");
      u.bindID = convertToInt(valueStr);
      shader->GetSampler2D(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddSampler2DProperty(u);
      }
      break;
    }
    case(SAMPLER3D):
    {
      tex3DUniform u;
      u.value = nullptr;
      u.bindID = convertToInt(valueStr);
      shader->GetSampler3D(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddSampler3DProperty(u);
      }
      break;
    }
    case(SAMPLERCUBE):
    {
      texCubeUniform u;
      u.value = nullptr;
      u.bindID = convertToInt(valueStr);
      shader->GetSamplerCube(propertyName.c_str(), u);
      if (u.id < 0)
      {
        LOG_WARN_S("Shader Parser", "property {} {} got optimized out", typeStr, nameStr);
      }
      else
      {
        shader->AddSamplerCubeProperty(u);
      }
      break;
    }
    }
  }

  bool isSpaces(char c, size_t &lineNum)
  {
    if (c == '\n') lineNum++;

    // ignore if space, newline, or tab
    return c == ' ' || c == '\n' || c == 9;
  }

  void extractChar(size_t& i, const std::string& str, char& c, size_t& lineNum)
  {
    for (; i < str.size(); i++)
    {
      if (isSpaces(str[i], lineNum) == false)
      {
        c = str[i];
        i++;
        break;
      }
    }
  }

  void extractWord(size_t &i, const std::string &str, std::string &word, bool &end, size_t& lineNum)
  {
    word.clear();
    bool inWord = false;
    for (; i < str.size(); i++)
    {
      if (str[i] == ';')
      {
        end = true;
        break;
      }

      if (inWord)
      {
        if (isSpaces(str[i], lineNum) == false)
        {
          word += str[i];
        }
        else
        {
          break;
        }
      }
      else
      {
        if (isSpaces(str[i], lineNum) == false)
        {
          word += str[i];
          inWord = true;
        }
      }
    }
  }

  bool isValidType(const std::string &type)
  {
    bool found = false;

    for (const char* t : uniformTypesStr)
    {
      if (t == type)
      {
        found = true;
        break;
      }
    }

    return found;
  }

  void ProcessDefines(const char* shaderName, const std::string& text, std::unordered_map<std::string, bool> &localDefines, size_t &i, size_t &lineNum, int depth, std::string& outGLSL)
  {
    std::string line;
    bool end;
    while (i < text.size())
    {
      extractWord(i, text, line, end, lineNum);

      if (line == "#ifdef" || line == "#ifndef")
      {
        ProcessDefines(shaderName, text, localDefines, i, lineNum, depth + 1, outGLSL);
      }
      else
      {

      }
    }
  }

  void ProcessDefines(const char *shaderName, const std::string& text, std::string& outGLSL)
  {
    size_t i = 0;
    std::string line;
    bool end;
    size_t lineNum = 0;
    int defineDepth = 0;
    std::unordered_map<std::string, bool> localDefines;
    bool expectElse = false;

    while (i < text.size())
    {
      extractWord(i, text, line, end, lineNum);

      if (expectElse)
      {
        bool foundElse = false;
        bool foundEnd = false;
        while (i < text.size())
        {
          extractWord(i, text, line, end, lineNum);

          if (line == "#elif" || line == "#else")
          {
            foundElse = true;
            break;
          }
          else if (line == "#endif")
          {
            foundEnd = true;
            break;
          }


        }
      }
      else
      {
        if (line == "#define")
        {
          extractWord(i, text, line, end, lineNum);

          localDefines[line] = true;
        }
        else if (line == "#ifdef")
        {
          defineDepth++;

          extractWord(i, text, line, end, lineNum);
          if (IsDefined(line.c_str(), localDefines))
          {

          }
          else
          {
            expectElse = true;
          }
        }
        else if (line == "#ifndef")
        {
          defineDepth++;

          extractWord(i, text, line, end, lineNum);
          if (IsDefined(line.c_str(), localDefines))
          {

          }
          else
          {
            expectElse = true;
          } 
        }
        else if (line == "#else")
        {
          LOG_ERROR_S("Shader Parser", "{}: #else without any #ifdef beforehand at line {}", shaderName, lineNum);
        }
        else if (line == "#elif")
        {
          LOG_ERROR_S("Shader Parser", "{}: #elif without any #ifdef beforehand at line {}", shaderName, lineNum);
        }
        else if (line == "#endif")
        {
          if (defineDepth == 0)
          {
            LOG_ERROR_S("Shader Parser", "{}: #endif without any #ifdef beforehand at line {}", shaderName, lineNum);
          }
          else
          {
            defineDepth--;
          }
        }
        else
        {
          outGLSL += line;
        }
      }
    }

    if (defineDepth > 0)
    {
      LOG_ERROR_S("Shader Parser", "{}: expected #endif before file end", shaderName);
    }
  }

  bool LoadGLSL(const char *path, std::string &out)
  {
    std::ifstream file(path);
    if (file.good() == false)
    {
      return false;
    }

    out = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return true;
  }

  bool ParseIncludes(std::string &glslIncludes, char &operant, size_t &i, const std::string &fileStr, size_t &lineNum, const char *shaderType, const char *filePath)
  {
    bool end;
    std::string line;
    while (true)
    {
      extractWord(i, fileStr, line, end, lineNum);
      if (line != "include")
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] Expected 'include' after '#' of {}", filePath, lineNum, shaderType);
        return false;
      }
      extractWord(i, fileStr, line, end, lineNum);
      std::string glslout;
      if (LoadGLSL(line.c_str(), glslout) == false)
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] Couldn't include {}", filePath, lineNum, line);
        return false;
      }
      glslIncludes += glslout;

      extractChar(i, fileStr, operant, lineNum);
      if (operant == '{')
        break;
      else if (operant == '#')
        continue;
      else
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] Expecting '#' or '{' while parsing includes {}", filePath, lineNum, line);
        return false;
      }
    }
    return true;
  }

  bool ParseGLSLFile(std::string &shaderOut, size_t& i, char &operant, size_t &lineNum, const std::string &propertyStruct, const std::string &fileStr, const char *filePath, const char *shaderType)
  {
    std::string GLSLincludes;
    extractChar(i, fileStr, operant, lineNum);
    if (operant == '#')
    {
      if (ParseIncludes(GLSLincludes, operant, i, fileStr, lineNum, "#VERT", filePath) == false)
      {
        return false;
      }
    }
    if (operant != '{')
    {
      LOG_ERROR_S("Shader Parser", "[{}, {}] Expecting '{' after #VERT", filePath, lineNum);
      return false;
    }

    shaderOut = "#version 430 core\n";
    shaderOut += propertyStruct;
    shaderOut += GLSLincludes;
    bool foundVertEnd = false;
    int bracketScope = 1;
    for (; i < fileStr.size(); i++)
    {
      if (fileStr[i] == '}')
      {
        bracketScope--;

        if (bracketScope < 1)
        {
          i++;
          foundVertEnd = true;
          break;
        }
      }
      else if (fileStr[i] == '{')
      {
        bracketScope++;
      }

      shaderOut += fileStr[i];
    }
    if (foundVertEnd == false)
    {
      LOG_ERROR_S("Shader Parser", "Expecting '}' for {}", shaderType);
      return false;
    }
    return true;
  }

  bool ExtractShader(const char* filePath, const char* shaderName, std::string& vertOut, std::string& fragOut, std::string &geoOut, std::vector<std::string> &uniformsNames, std::vector<std::string> &uniformsTypes, std::vector<std::string> &uniformsValues)
  {
    std::fstream file(filePath);
    std::string matProperty;
    size_t lineNum = 0;

    if (file.good() == false)
    {
      LOG_ERROR_S("Shader Parser", "Failed to open file: {}", filePath);
      return false;
    }

    std::string fileStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // read Property block {}
    std::string line;
    size_t i = 0;
    bool e;
    extractWord(i, fileStr, line, e, lineNum);
    if (line != "Property")
    {
      LOG_ERROR_S("Shader Parser", "[{}, {}] Expected 'Property' on the file line", filePath, lineNum);
      file.close();
      return false;
    }

    char operant;
    extractChar(i, fileStr, operant, lineNum);
    if (operant != '{')
    {
      LOG_ERROR_S("Shader Parser", "[{}, {}] Expecting '{' after Property", filePath, lineNum);
      file.close();
      return false;
    }

    std::string propType;
    std::string propName;
    std::string propInit;
    std::string propVal;
    bool firstVar = true;
    bool noStruct = false;
    while (fileStr[i] != '}' && i < fileStr.size())
    {
      if (firstVar)
      {
        char c;
        size_t tmpI = i;
        extractChar(tmpI, fileStr, c, lineNum);
        if (c == '}')
        {
          i = tmpI;
          noStruct = true;
          break;
        }
      }

      propType.clear();
      propName.clear();
      propInit.clear();
      propVal.clear();
      bool end = false;

      extractWord(i, fileStr, propType, end, lineNum);
      if (end)
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] Found ';' after property type {}", filePath, lineNum, propType);
        file.close();
        return false;
      }
      // check type
      if (isValidType(propType) == false)
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] {} is not a valid property type", filePath, lineNum, propType);
        file.close();
        return false;
      }

      extractWord(i, fileStr, propName, end, lineNum);
      if (end)
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] Found ';' after property name {}", filePath, lineNum, propName);
        file.close();
        return false;
      }

      char opr;
      extractChar(i, fileStr, opr, lineNum);
      if (opr != '=')
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] Expecting '=' after {} not {}", filePath, lineNum, propName, opr);
        file.close();
        return false;
      }

      extractChar(i, fileStr, opr, lineNum);
      if (opr != '(')
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] Expecting '(' before {}'s initialization", filePath, lineNum, propName);
        file.close();
        return false;
      }

      bool foundInitEnd = false;
      for (; i < fileStr.size(); i++)
      {
        if (fileStr[i] == ')')
        {
          i++;
          foundInitEnd = true;
          break;
        }

        propInit += fileStr[i];
      }
      if (foundInitEnd == false)
      {
        LOG_ERROR_S("Shader Parser", "[{}, {}] Expecting ')' after {}'s initialization", filePath, lineNum, propName);
        file.close();
        return false;
      }

      uniformsNames.push_back(propName);
      uniformsTypes.push_back(propType);
      uniformsValues.push_back(propInit);

      for (; i < fileStr.size(); i++)
      {
        if (isSpaces(fileStr[i], lineNum) == false)
        {
          break;
        }
      }

      firstVar = false;
    }

    i++;

    std::string propertyStruct;
    if (noStruct == false)
    {
      // create PropertyStruct and uniform
      propertyStruct = "struct Property{";
      for (int n = 0; n < uniformsNames.size(); n++)
      {
        if (uniformsTypes[n] == "color3")
        {
          propertyStruct += "vec3";
        }
        else if (uniformsTypes[n] == "color4")
        {
          propertyStruct += "vec4";
        }
        else
        {
          propertyStruct += uniformsTypes[n];
        }

        propertyStruct += ' ';
        propertyStruct += uniformsNames[n];
        propertyStruct += ';';
      }
      propertyStruct += "};\n";
      propertyStruct += "uniform Property property;\n";
    }

    // read VERT block
    bool end;
    extractWord(i, fileStr, line, end, lineNum);
    if (line != "#VERT")
    {
      LOG_ERROR_S("Shader Parser", "[{}, {}] Couldn't find #VERT after property block", filePath, lineNum);
      file.close();
      return false;
    }
    if (ParseGLSLFile(vertOut, i, operant, lineNum, propertyStruct, fileStr, filePath, "#VERT") == false)
    {
      file.close();
      return false;
    }

    // find #GEO or #FRAG
    extractWord(i, fileStr, line, end, lineNum);
    if (line != "#FRAG" && line != "#GEO")
    {
      LOG_ERROR_S("Shader Parser", "{} isn't #FRAG nor #GEO", line);
      file.close();
      return false;
    }
    // have geometry shader
    if (line == "#GEO")
    {
      if (ParseGLSLFile(geoOut, i, operant, lineNum, propertyStruct, fileStr, filePath, "#GEO") == false)
      {
        file.close();
        return false;
      }
      // find #FRAG
      extractWord(i, fileStr, line, end, lineNum);
      if (line != "#FRAG")
      {
        LOG_ERROR("Shader Parser", "Expecting #FRAG after #GEO");
        file.close();
        return false;
      }
      if (ParseGLSLFile(fragOut, i, operant, lineNum, propertyStruct, fileStr, filePath, "#FRAG") == false)
      {
        file.close();
        return false;
      }
    }
    // parse FRAG shader
    else
    {
      if (ParseGLSLFile(fragOut, i, operant, lineNum, propertyStruct, fileStr, filePath, "#FRAG") == false)
      {
        file.close();
        return false;
      }
    }

    // Close Files
    file.close();
    return true;
  }

  Shader* ReadShader(const char* filePath, const char* shaderName)
  {
    std::string vertFile, fragFile, geoFile;

    std::vector<std::string> uniformsNames;
    std::vector<std::string> uniformsTypes;
    std::vector<std::string> uniformsValues;

    if (ExtractShader(filePath, shaderName, vertFile, fragFile, geoFile, uniformsNames, uniformsTypes, uniformsValues) == false) return nullptr;

    // compile vert and frag program
    Shader* newShader = new Shader(shaderName, vertFile.c_str(), fragFile.c_str(), geoFile.empty() ? nullptr : geoFile.c_str(), filePath);
    
    // create property uniforms
    for (int n = 0; n < uniformsNames.size(); n++)
    {
      // convert str to value
      convertToTypes(newShader, uniformsValues[n], uniformsTypes[n], uniformsNames[n]);
    }

#ifdef EDITOR_MODE
    std::string outFileName = "../Log/";
    outFileName += shaderName;

    std::string vertFileName = outFileName;
    vertFileName += ".vert";
    std::string fragFileName = outFileName;
    fragFileName += ".frag";

    std::ofstream voutput(vertFileName);
    if (voutput.is_open() == true)
    {
      voutput << vertFile;
      voutput.close();
    }

    std::ofstream foutput(fragFileName);
    if (foutput.is_open() == true)
    {
      foutput << fragFile;
      foutput.close();
    }
#endif

    return newShader;
  }

  bool ReloadShader(Shader* shader, const char* filePath, const char* shaderName)
  {
    std::string vertFile, fragFile, geoFile;

    std::vector<std::string> uniformsNames;
    std::vector<std::string> uniformsTypes;
    std::vector<std::string> uniformsValues;

    if (ExtractShader(filePath, shaderName, vertFile, fragFile, geoFile, uniformsNames, uniformsTypes, uniformsValues) == false) return false;

    GLuint id = CompileShaders(shaderName, vertFile.c_str(), fragFile.c_str(), geoFile.empty() ? nullptr : geoFile.c_str());
    if (id)
    {
      shader->SetID(id);
      shader->ExtractUniforms();

      // create property uniforms
      for (int n = 0; n < uniformsNames.size(); n++)
      {
        // convert str to value
        convertToTypes(shader, uniformsValues[n], uniformsTypes[n], uniformsNames[n]);
      }
      return true;
    }
    return false;
  }
}