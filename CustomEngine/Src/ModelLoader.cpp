/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ModelLoader.cpp
Purpose: Load model from obj file format
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/11/2019
End Header --------------------------------------------------------*/

#include "ModelLoader.h"
#include "Model.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <math.h>
#include "ModelLoader.h"

#define TINYPLY_IMPLEMENTATION
#include <tinyply.h>
#include "example-utils.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "SkinnedMesh.h"
#include "BoneData.h"
#include "SkeletalAnimation.h"
#include <map>

static float toFloat(std::string& floatString)
{
  return static_cast<float>(atof(floatString.c_str()));
}

static unsigned int toInt(std::string& intString)
{
  return static_cast<unsigned int>(atoi(intString.c_str()));
}

static bool isNotNumber(char c)
{
  return (c == ' ' || c == '\t');
}

static bool isEndOfLine(char c)
{
  return c == '\0';
}

static void updateBoundingBox(BoundingBox& out, const glm::vec3 &vertex)
{
  //out.InitVertices(vertex);
  //out.left = (vertex.x < out.left) ? vertex.x : out.left;
  //out.right = (vertex.x > out.right) ? vertex.x : out.right;
  //out.bottom = (vertex.y < out.bottom) ? vertex.y : out.bottom;
  //out.top = (vertex.y > out.top) ? vertex.y : out.top;
  //out.back = (vertex.z < out.back) ? vertex.z : out.back;
  //out.front = (vertex.z > out.front) ? vertex.z : out.front;
}

static BoundingBox processVertices(std::vector<std::string>& verticesTxt, std::vector<glm::vec3>& output)
{
  BoundingBox box;
  bool firstVertex = true;

  for (std::string& line : verticesTxt)
  {
    glm::vec3 pos;

    int i = 2;
    int index = 0;
    bool readingValue = false;
    std::string floatValue;
    while (!isEndOfLine(line[i]))
    {
      if (isNotNumber(line[i]))
      {
        if (readingValue)
        {
          pos[index] = toFloat(floatValue);

          index++;
          floatValue.clear();

          readingValue = false;
        }
      }
      else
      {
        readingValue = true;
        floatValue += line[i];
      }
      i++;
    }
    pos[index] = toFloat(floatValue);

    /*if (firstVertex)
    {
      box.left = pos.x;
      box.right = pos.x;
      box.top = pos.y;
      box.bottom = pos.y;
      box.back = pos.z;
      box.front = pos.z;
      firstVertex = false;
    }
    else
    {
      updateBoundingBox(box, pos);
    }*/
    
    output.push_back(pos);
  }

  box.UpdateVertices(output);

  return box;
}

static void processNormals(std::vector<std::string>& normalsTxt, std::vector<glm::vec3>& output)
{
  for (std::string& line : normalsTxt)
  {
    glm::vec3 norm;

    int i = 3;
    int index = 0;
    bool readingValue = false;
    std::string floatValue;
    while (!isEndOfLine(line[i]))
    {
      if (isNotNumber(line[i]))
      {
        if (readingValue)
        {
          norm[index] = toFloat(floatValue);

          index++;
          floatValue.clear();
          readingValue = false;
        }
      }
      else
      {
        readingValue = true;
        floatValue += line[i];
      }
      i++;
    }
    norm[index] = toFloat(floatValue);

    output.push_back(norm);
  }
}

static void processUVs(std::vector<std::string>& uvsTxt, std::vector<glm::vec2>& output)
{
  for (std::string& line : uvsTxt)
  {
    glm::vec2 uv;

    int i = 3;
    int index = 0;
    bool readingValue = false;
    std::string floatValue;
    while (!isEndOfLine(line[i]))
    {
      if (isNotNumber(line[i]))
      {
        if (readingValue)
        {
          uv[index] = toFloat(floatValue);

          index++;
          floatValue.clear();

          readingValue = false;
        }
      }
      else
      {
        readingValue = true;
        floatValue += line[i];
      }
      i++;
    }
    if(index < 2)
      uv[index] = toFloat(floatValue);

    output.push_back(uv);
  }
}

static void addIndice(int vertexType, std::string& intValue, PolyVertex& output)
{
  if (!intValue.empty())
  {
    switch (vertexType)
    {
    case(0):
    {
      output.vertexID = toInt(intValue) - 1;
      break;
    }
    case(1):
    {
      output.uvID = toInt(intValue) - 1;
      break;
    }
    case(2):
    {
      output.normalID = toInt(intValue) - 1;
      break;
    }
    }
  }
  intValue.clear();
}

static void processFaces(std::vector<std::string>& facesTxt, std::vector<Polygon> &output)
{
  for (std::string& line : facesTxt)
  {
    int i = 2;
    std::string intValue;
    int vertexType = 0;
    Polygon poly;
    PolyVertex vertex;
    while (!isEndOfLine(line[i]))
    {
      // face have vertex, uv, and normal
      if (line[i] == '\\')
      {
        addIndice(vertexType, intValue, vertex);

        vertexType++;
      }
      else if (isNotNumber(line[i]))
      {
        addIndice(vertexType, intValue, vertex);

        poly.vertices.push_back(vertex);
        vertex = PolyVertex();

        vertexType = 0;
      }
      else
      {
        intValue += line[i];
      }
      i++;
    }
    addIndice(vertexType, intValue, vertex);
    
    poly.vertices.push_back(vertex);
    output.push_back(poly);
  }
}

static void constructVertexData(std::vector<Polygon>& polygons, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, std::vector<glm::vec2>& uvs, std::vector<Vertex>& outVertices)
{
  for (glm::vec3& pos : positions)
    outVertices.push_back(Vertex(pos));

  for (Polygon& poly : polygons)
  {
    for (PolyVertex vertex : poly.vertices)
    {
      if(vertex.normalID > -1)
        outVertices[vertex.vertexID].normal = normals[vertex.normalID];
      if (vertex.uvID > -1)
        outVertices[vertex.vertexID].uv = uvs[vertex.uvID];
    }
  }
}

static void triangulatePolyToFace(std::vector<Polygon> &polys, std::vector<Face> &outFaces, std::vector<Vertex> &outVertices)
{
  for (Polygon& poly : polys)
  {
    // construct first triangle
    Face f;
    unsigned int faceID = outFaces.size();

    f.indices[0] = poly.vertices[0].vertexID;
    f.indices[1] = poly.vertices[1].vertexID;
    f.indices[2] = poly.vertices[2].vertexID;
    outFaces.push_back(f);

    outVertices[f.indices[0]].sharingFaces.push_back(faceID);
    outVertices[f.indices[1]].sharingFaces.push_back(faceID);
    outVertices[f.indices[2]].sharingFaces.push_back(faceID);

    // for polygon with more than 3 vertices, construct following triangles as triangle fan
    unsigned int vertexNum = poly.vertices.size();
    if (vertexNum > 3)
    {
      for (unsigned int j = 3; j < vertexNum; j++)
      {
        unsigned int faceID = outFaces.size();
        f.indices[0] = poly.vertices[j - 1].vertexID;
        f.indices[1] = poly.vertices[j].vertexID;
        f.indices[2] = poly.vertices[0].vertexID;
        outFaces.push_back(f);

        outVertices[f.indices[0]].sharingFaces.push_back(faceID);
        outVertices[f.indices[1]].sharingFaces.push_back(faceID);
        outVertices[f.indices[2]].sharingFaces.push_back(faceID);
      }
    }
  }
}

static void adjustModelTransform(std::vector<Vertex>& outVertices, BoundingBox &box)
{
  glm::vec3 boxDiff = box.InitVertices[7] - box.InitVertices[0];
  glm::vec3 center(boxDiff * 0.5f);
  float biggestSize = 0.f;
  float xScale = abs(boxDiff.x);
  float yScale = abs(boxDiff.y);
  float zScale = abs(boxDiff.z);
  biggestSize = (xScale > yScale) ? xScale : yScale;
  biggestSize = (zScale > biggestSize) ? zScale : biggestSize;

  center /= biggestSize;

  for (Vertex &vert : outVertices)
  {
    vert.position = vert.position / biggestSize;
    vert.position -= center;
  }

  float left = box.Left() / biggestSize - center.x;
  float right = box.Right() / biggestSize - center.x;
  float bottom = box.Bottom() / biggestSize - center.y;
  float top = box.Top() / biggestSize - center.y;
  float near = box.Near() / biggestSize - center.z;
  float far = box.Far() / biggestSize - center.z;
  box.UpdateVertices(left, right, bottom, top, near, far);
  
}

bool formatCheck(const char *fileName, bool &isPly)
{
  std::string filename(fileName);
  bool foundFormat = false;
  std::string fileFormat;
  for (int i = filename.size() - 1; i > 0; i--)
  {
    if (filename[i] == '.')
    {
      foundFormat = true;
      break;
    }
    fileFormat += filename[i];
  }
  if (foundFormat == false)
  {
    std::cout << "Model Loader[ERROR]: file name: " << fileName << " doesn't contain any format!" << std::endl;
    return false;
  }

  int fileFormatSize = fileFormat.size();
  for (int i = 0; i < fileFormatSize / 2; i++)
  {
    char tmp = fileFormat[i];
    fileFormat[i] = fileFormat[fileFormatSize - 1 - i];
    fileFormat[fileFormatSize - 1 - i] = tmp;
  }

  bool validFormat = false;
  if (fileFormat == "obj")
  {
    validFormat = true;
  }
  else if (fileFormat == "ply")
  {
    isPly = true;
    validFormat = true;
  }
  if (validFormat == false)
  {
    std::cout << "Model Loader[ERROR]: " << fileFormat << " is not a valid format!" << std::endl;
    return false;
  }
  return true;
}

bool LoadModel(const char* fileName, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces, BoundingBox& outBox, Model** plyModel, bool normalizeScale = true)
{
  std::ifstream file(fileName);
  if (!file)
  {
    std::cout << "Model Loader[ERROR]: no such file name: " << fileName << std::endl;
    return false;
  }

  // get file format
  bool isPly = false;
  if (formatCheck(fileName, isPly) == false)
  {
    file.close();
    return false;
  }

  if (isPly)
  {
    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;

    bool preload_into_memory = true;
    std::string filepath(fileName);
    try
    {
      // For most files < 1gb, pre-loading the entire file upfront and wrapping it into a 
      // stream is a net win for parsing speed, about 40% faster. 
      if (preload_into_memory)
      {
        byte_buffer = read_file_binary(filepath);
        file_stream.reset(new memory_stream((char*)byte_buffer.data(), byte_buffer.size()));
      }
      else
      {
        file_stream.reset(new std::ifstream(filepath, std::ios::binary));
      }

      if (!file_stream || file_stream->fail()) throw std::runtime_error("file_stream failed to open " + filepath);

      file_stream->seekg(0, std::ios::end);
      const float size_mb = file_stream->tellg() * float(1e-6);
      file_stream->seekg(0, std::ios::beg);

      tinyply::PlyFile file;
      file.parse_header(*file_stream);

      std::cout << "\t[ply_header] Type: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;
      for (const auto& c : file.get_comments()) std::cout << "\t[ply_header] Comment: " << c << std::endl;
      for (const auto& c : file.get_info()) std::cout << "\t[ply_header] Info: " << c << std::endl;

      for (const auto& e : file.get_elements())
      {
        std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
        for (const auto& p : e.properties)
        {
          std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
          if (p.isList) std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
          std::cout << std::endl;
        }
      }

      // Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers. 
      // See examples below on how to marry your own application-specific data structures with this one. 
      std::shared_ptr<tinyply::PlyData> vertices, normals, colors, texcoords, faces, tripstrip;

      // The header information can be used to programmatically extract properties on elements
      // known to exist in the header prior to reading the data. For brevity of this sample, properties 
      // like vertex position are hard-coded: 
      try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
      catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

      try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
      catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

      try { colors = file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" }); }
      catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

      try { colors = file.request_properties_from_element("vertex", { "r", "g", "b", "a" }); }
      catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

      try { texcoords = file.request_properties_from_element("vertex", { "u", "v" }); }
      catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

      // Providing a list size hint (the last argument) is a 2x performance improvement. If you have 
      // arbitrary ply files, it is best to leave this 0. 
      try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 3); }
      catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

      // Tristrips must always be read with a 0 list size hint (unless you know exactly how many elements
      // are specifically in the file, which is unlikely); 
      try { tripstrip = file.request_properties_from_element("tristrips", { "vertex_indices" }, 0); }
      catch (const std::exception & e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

      manual_timer read_timer;

      read_timer.start();
      file.read(*file_stream);
      read_timer.stop();

      const float parsing_time = read_timer.get() / 1000.f;
      std::cout << "\tparsing " << size_mb << "mb in " << parsing_time << " seconds [" << (size_mb / parsing_time) << " MBps]" << std::endl;

      if (vertices)   std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
      if (normals)    std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
      if (colors)     std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
      if (texcoords)  std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
      if (faces)      std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
      if (tripstrip)  std::cout << "\tRead " << (tripstrip->buffer.size_bytes() / tinyply::PropertyTable[tripstrip->t].stride) << " total indicies (tristrip) " << std::endl;

      std::vector<Vertex> vertexData(vertices->count);
      if (vertices->t == tinyply::Type::FLOAT32 && normals->t == tinyply::Type::FLOAT32)
      {
        for (int i = 0; i < vertexData.size(); i++)
        {
          vertexData[i].position.x = reinterpret_cast<float*>(vertices->buffer.get())[i * 3];
          vertexData[i].position.y = reinterpret_cast<float*>(vertices->buffer.get())[i * 3 + 1];
          vertexData[i].position.z = reinterpret_cast<float*>(vertices->buffer.get())[i * 3 + 2];
          vertexData[i].normal.x = reinterpret_cast<float*>(normals->buffer.get())[i * 3];
          vertexData[i].normal.y = reinterpret_cast<float*>(normals->buffer.get())[i * 3 + 1];
          vertexData[i].normal.z = reinterpret_cast<float*>(normals->buffer.get())[i * 3 + 2];
        }
      }
      else if(vertices->t == tinyply::Type::FLOAT64 && normals->t == tinyply::Type::FLOAT64)
      {
        for (int i = 0; i < vertexData.size(); i++)
        {
          vertexData[i].position.x = reinterpret_cast<double*>(vertices->buffer.get())[i * 3];
          vertexData[i].position.y = reinterpret_cast<double*>(vertices->buffer.get())[i * 3 + 1];
          vertexData[i].position.z = reinterpret_cast<double*>(vertices->buffer.get())[i * 3 + 2];
          vertexData[i].normal.x = reinterpret_cast<double*>(normals->buffer.get())[i * 3];
          vertexData[i].normal.y = reinterpret_cast<double*>(normals->buffer.get())[i * 3 + 1];
          vertexData[i].normal.z = reinterpret_cast<double*>(normals->buffer.get())[i * 3 + 2];
        }
      }
      else if (vertices->t == tinyply::Type::FLOAT32 && normals->t == tinyply::Type::FLOAT64)
      {
        for (int i = 0; i < vertexData.size(); i++)
        {
          vertexData[i].position.x = reinterpret_cast<float*>(vertices->buffer.get())[i * 3];
          vertexData[i].position.y = reinterpret_cast<float*>(vertices->buffer.get())[i * 3 + 1];
          vertexData[i].position.z = reinterpret_cast<float*>(vertices->buffer.get())[i * 3 + 2];
          vertexData[i].normal.x = reinterpret_cast<double*>(normals->buffer.get())[i * 3];
          vertexData[i].normal.y = reinterpret_cast<double*>(normals->buffer.get())[i * 3 + 1];
          vertexData[i].normal.z = reinterpret_cast<double*>(normals->buffer.get())[i * 3 + 2];
        }
      }
      else if (vertices->t == tinyply::Type::FLOAT64 && normals->t == tinyply::Type::FLOAT32)
      {
        for (int i = 0; i < vertexData.size(); i++)
        {
          vertexData[i].position.x = reinterpret_cast<double*>(vertices->buffer.get())[i * 3];
          vertexData[i].position.y = reinterpret_cast<double*>(vertices->buffer.get())[i * 3 + 1];
          vertexData[i].position.z = reinterpret_cast<double*>(vertices->buffer.get())[i * 3 + 2];
          vertexData[i].normal.x = reinterpret_cast<float*>(normals->buffer.get())[i * 3];
          vertexData[i].normal.y = reinterpret_cast<float*>(normals->buffer.get())[i * 3 + 1];
          vertexData[i].normal.z = reinterpret_cast<float*>(normals->buffer.get())[i * 3 + 2];
        }
      }

      std::vector<unsigned int> indices(faces->count * 3);
      for (int i = 0; i < indices.size(); i++)
      {
        indices[i] = reinterpret_cast<unsigned int*>(faces->buffer.get())[i];
      }

      BoundingBox box;
      box.UpdateVertices(vertexData);

      if(normalizeScale)
        adjustModelTransform(vertexData, box);

      *plyModel = new Model(vertexData, indices, PM_Triangles);
      (*plyModel)->SetFilePath(fileName);
    }
    catch (const std::exception & e)
    {
      std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
  }
  else
  {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<Polygon> polygons;

    std::vector<std::string> verticesTxt, normalsTxt, uvsTxt, facesTxt;
    std::string line;

    // extract all lines related to vertex. normal, uv, and face
    while (std::getline(file, line))
    {
      std::string dataType;
      for (char c : line)
      {
        if (isNotNumber(c))
          break;

        dataType += c;
      }

      if (dataType == "v")
      {
        verticesTxt.push_back(line);
      }
      else if (dataType == "f")
      {
        facesTxt.push_back(line);
      }
      else if (dataType == "vt")
      {
        uvsTxt.push_back(line);
      }
      else if (dataType == "vn")
      {
        normalsTxt.push_back(line);
      }
    }

    // process all lines into array
    outBox = processVertices(verticesTxt, positions);
    processNormals(normalsTxt, normals);
    processUVs(uvsTxt, uvs);
    processFaces(facesTxt, polygons);

    constructVertexData(polygons, positions, normals, uvs, outVertices);

    // triangulate vertices according to faces
    triangulatePolyToFace(polygons, outFaces, outVertices);

    // scale model into -1 to 1 scale, move model center to position 0,0
    if(normalizeScale)
      adjustModelTransform(outVertices, outBox);
  }

  file.close();
  return true;
}

Model* LoadModelFile(const char* fileName, NormalBaseMode nbm, bool normalizeScale)
{
  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  BoundingBox box;
  Model* model = nullptr;
  if (LoadModel(fileName, vertices, faces, box, &model, normalizeScale) == false)
  {
    return nullptr;
  }

  if (model == nullptr)
  {
    model = new Model(vertices, faces, PM_Triangles, nbm);
    model->SetFilePath(fileName);

    model->boundingBox = box;
  }

  return model;
}

bool ReloadModel(const char* fileName, std::vector<Vertex>& outVertices, std::vector<Face>& outFaces, BoundingBox &outBox)
{
  Model* model = nullptr;
  if (LoadModel(fileName, outVertices, outFaces, outBox, &model) == false)
  {
    return false;
  }

  if (model)
  {
    delete model;
  }

  return true;
}

const int MAX = 3; // Loops will print at most MAX entries followed by an elipsis.

// Prints a mesh's info; A mesh contains vertices, faces, normals and
// more as needed for graphics.  Vertices are tied to bones with
// weights.
void showMesh(aiMesh* mesh)
{
  // Mesh name and some counts
  printf("Mesh %s: %d vertices, %d faces,  %d bones\n", mesh->mName.C_Str(),
    mesh->mNumVertices, mesh->mNumFaces, mesh->mNumBones);

  // Mesh's bones and weights of all connected vertices.
  for (int i = 0; i < mesh->mNumBones && i < MAX; i++) {
    aiBone* bone = mesh->mBones[i];
    printf("  %s:  %d weights;  OffsetMatrix:[%f, ...]\n", bone->mName.C_Str(),
      bone->mNumWeights, bone->mOffsetMatrix[0][0]);
    for (int i = 0; i < bone->mNumWeights && i < MAX; i++) {
      printf("    %d %f\n", bone->mWeights[i].mVertexId, bone->mWeights[i].mWeight);
    }
    if (bone->mNumWeights > MAX) printf("    ...\n");
  }
  if (mesh->mNumBones > MAX) printf("  ...\n");
}

// Prints an animation.  An animation contains a few timing parameters
// and then channels for a number of animated bones.  Each channel
// contains a V, Q, and S keyframe sequences.
void showAnimation(aiAnimation* anim)
{
  printf("Animation: %s\n  duration (in ticks): %f\n  tickspersecond: %f\n  numchannels: %d\n",
    anim->mName.C_Str(),
    anim->mDuration,
    anim->mTicksPerSecond,
    anim->mNumChannels);

  // The animations channels
  for (int i = 0; i < anim->mNumChannels && i < MAX; i++) {
    aiNodeAnim* chan = anim->mChannels[i];

    // Prints the bone name followed by the numbers of each key type
    printf("\n");
    printf("    %-15s VQS keys:  %d %d %d\n",
      chan->mNodeName.C_Str(),
      chan->mNumPositionKeys,
      chan->mNumRotationKeys,
      chan->mNumScalingKeys);

    // The position (V) keys
    printf("\n");
    for (int i = 0; i < chan->mNumPositionKeys && i < MAX; i++) {
      aiVectorKey key = chan->mPositionKeys[i];
      printf("      V[%d]: %f : (%f %f %f)\n", i, key.mTime,
        key.mValue[0], key.mValue[1], key.mValue[2]);
    }
    if (chan->mNumPositionKeys > MAX) printf("      ...\n");

    // The rotation (Q) keys
    printf("\n");
    for (int i = 0; i < chan->mNumRotationKeys && i < MAX; i++) {
      aiQuatKey key = chan->mRotationKeys[i];
      printf("      Q[%d]: %f : (%f %f %f %f)\n", i, key.mTime,
        key.mValue.w, key.mValue.x, key.mValue.y, key.mValue.z);
    }
    if (chan->mNumRotationKeys > MAX) printf("      ...\n");

    // the scaling (S) keys
    printf("\n");
    for (int i = 0; i < chan->mNumScalingKeys && i < MAX; i++) {
      aiVectorKey key = chan->mScalingKeys[i];
      printf("      S[%d]: %f : (%f %f %f)\n", i, key.mTime,
        key.mValue[0], key.mValue[1], key.mValue[2]);
    }
    if (chan->mNumScalingKeys > MAX) printf("      ...\n");
  }

  if (anim->mNumChannels > MAX) printf("    ...\n");
}

// Prints the bone hierarchy and relevant info with a graphical
// representation of the hierarchy.
void showBoneHierarchy(const aiScene* scene, const aiNode* node, const int level = 0)
{
  // Print indentation to show this node's level in the hierarchy
  for (int i = 0; i < level; i++)
    printf(" |");

  // Print node name and transformation to parent's node
  printf("%s                    Transformation:[%f, ...]\n", node->mName.C_Str(),
    node->mTransformation[0][0]);

  // Recurse onto this node's children
  for (unsigned int i = 0; i < node->mNumChildren; ++i)
    showBoneHierarchy(scene, node->mChildren[i], level + 1);
}

aiMatrix4x4 GLMMat4ToAi(glm::mat4 mat)
{
  return aiMatrix4x4(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
    mat[1][0], mat[1][1], mat[1][2], mat[1][3],
    mat[2][0], mat[2][1], mat[2][2], mat[2][3],
    mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
}

glm::mat4 AiToGLMMat4(const aiMatrix4x4& atMat)
{
  glm::mat4 tmp;
  tmp[0][0] = atMat.a1;
  tmp[0][1] = atMat.b1;
  tmp[0][2] = atMat.c1;
  tmp[0][3] = atMat.d1;

  tmp[1][0] = atMat.a2;
  tmp[1][1] = atMat.b2;
  tmp[1][2] = atMat.c2;
  tmp[1][3] = atMat.d2;

  tmp[2][0] = atMat.a3;
  tmp[2][1] = atMat.b3;
  tmp[2][2] = atMat.c3;
  tmp[2][3] = atMat.d3;

  tmp[3][0] = atMat.a4;
  tmp[3][1] = atMat.b4;
  tmp[3][2] = atMat.c4;
  tmp[3][3] = atMat.d4;
  return tmp;
}

aiNode* FindRootBone(std::vector<aiNode*> nodeList)
{
  for (aiNode* node : nodeList)
  {
    if (std::string(node->mName.C_Str()) == "root" && node->mParent)
      return node;
  }
  return nullptr;
}

void ProcessBoneHierarchy(aiNode* currNode, std::vector<aiNode*> &nodes)
{
  nodes.push_back(currNode);

  for (unsigned int i = 0; i < currNode->mNumChildren; i++)
  {
    ProcessBoneHierarchy(currNode->mChildren[i], nodes);
  }
}

glm::mat4 CalculateNodeTransform(const aiNode* root)
{
  glm::mat4 ret(1.f);

  const aiNode* node = root;
  while (node != nullptr)
  {
    glm::mat4 mat = AiToGLMMat4(node->mTransformation);
    ret = glm::transpose(mat) * ret;
    node = node->mParent;
  }
  return ret;
}

void ProcessNodes(const aiNode* currNode, std::vector<std::string> &nodeNames, std::map<std::string, std::string>& nodeParents, std::map<std::string, std::vector<std::string>> &nodeChildrens, std::map<std::string, glm::mat4> &nodeTransforms)
{
  if (currNode == nullptr) return;

  std::string nodeName(currNode->mName.C_Str());
  nodeNames.push_back(nodeName);

  if (currNode->mParent != nullptr)
  {
    nodeParents[nodeName] = std::string(currNode->mParent->mName.C_Str());
  }

  std::vector<std::string> childNames;
  for (unsigned int i = 0; i < currNode->mNumChildren; i++)
  {
    childNames.push_back(std::string(currNode->mChildren[i]->mName.C_Str()));
  }
  nodeChildrens[nodeName] = childNames;

  nodeTransforms[nodeName] = AiToGLMMat4(currNode->mTransformation);//CalculateNodeTransform(currNode);

  for (unsigned int i = 0; i < currNode->mNumChildren; i++)
  {
    ProcessNodes(currNode->mChildren[i], nodeNames, nodeParents, nodeChildrens, nodeTransforms);
  }
}

void ProcessMesh(aiMesh *mesh, unsigned int meshIndex, const aiScene *scene, const std::vector<std::string> &nodeNames, std::vector<SkinnedVertex> &vertices, std::vector<unsigned int> &indices, std::map<std::string, glm::mat4> &boneOffsets)
{
  unsigned int VertexStartIndex = vertices.size();

  if (mesh->HasTextureCoords(0))
  {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      aiVector3D position = mesh->mVertices[i];
      aiVector3D normal = mesh->mNormals[i];
      aiVector3D uv = mesh->mTextureCoords[0][i];

      vertices.push_back(SkinnedVertex(glm::vec3(position.x, position.y, position.z), glm::vec3(normal.x, normal.y, normal.z), glm::vec2(uv.x, uv.y)));
    }
  }
  else
  {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      aiVector3D position = mesh->mVertices[i];
      aiVector3D normal = mesh->mNormals[i];

      vertices.push_back(SkinnedVertex(glm::vec3(position.x, position.y, position.z), glm::vec3(normal.x, normal.y, normal.z), glm::vec2()));
    }
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
    {
      indices.push_back(face.mIndices[j]);
    }
  }

  // see if mesh has bones or not
  if (mesh->HasBones())
  {
    std::vector<unsigned int> weightCounts(mesh->mNumVertices, 0);

    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
      const aiBone* bone = mesh->mBones[i];
      std::string boneName = std::string(bone->mName.C_Str());

      std::map<std::string, glm::mat4>::const_iterator offset_it = boneOffsets.find(boneName);
      if (offset_it == boneOffsets.end())
      {
        glm::mat4 mat = AiToGLMMat4(bone->mOffsetMatrix);
        boneOffsets[boneName] = mat;
      }
      
      std::vector<std::string>::const_iterator node_it = std::find(nodeNames.begin(), nodeNames.end(), boneName);
      ASSERT(node_it != nodeNames.end(), "[ProcessMesh] Bone {} not found in sceneNode!", boneName);

      unsigned int boneID = (unsigned int)(node_it - nodeNames.begin());

      for (unsigned int w = 0; w < bone->mNumWeights; w++)
      {
        const struct aiVertexWeight* weight = &bone->mWeights[w];
        ASSERT(weightCounts[weight->mVertexId] < 4, "[ProcessMesh] mesh has over 4 bones!");

        unsigned int vertexID = weight->mVertexId + VertexStartIndex;
        vertices[vertexID].boneIDs[weightCounts[weight->mVertexId]] = boneID;
        vertices[vertexID].weights[weightCounts[weight->mVertexId]] = weight->mWeight;
        weightCounts[weight->mVertexId]++;
      }
    }

    //// for each bone, find vertices it associates with then write weight data to the vertices.
    //for (unsigned int i = 0; i < mesh->mNumBones; i++)
    //{
    //  BoneData bone;
    //  bone.ID = i;
    //  bone.Name = mesh->mBones[i]->mName.C_Str();
    //  bone.StaticTransform = AiToGLMMat4(mesh->mBones[i]->mOffsetMatrix) * invRootMat;

    //  bones.push_back(bone);

    //  for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
    //  {
    //    unsigned int vertexID = mesh->mBones[i]->mWeights[j].mVertexId;
    //    vertices[vertexID].AddWeight(i, mesh->mBones[i]->mWeights[j].mWeight);
    //  }
    //}
  }
}

void CreateBones(std::vector<Bone>& bones, std::map<std::string, Bone*> &boneMap, const std::vector<std::string> &nodeNames, const std::map<std::string, std::string>& nodeParents, const std::map<std::string, std::vector<std::string>>& nodeChildrens, const std::map<std::string, glm::mat4> &boneOffsets, const std::map<std::string, glm::mat4>& nodeTransforms)
{
  bones.reserve(nodeNames.size());

  unsigned int id = 0;
  for (const std::string& name : nodeNames)
  {
    Bone bone;
    bone.ID = id;
    bone.Name = name;
    bone.OffsetMatrix = glm::mat4(1.f);
    bone.BindposeMatrix = glm::mat4(1.f);
    bone.TransformMatrix = glm::mat4(1.f);

    auto offsetIT = boneOffsets.find(name);
    if (offsetIT != boneOffsets.end())
    {
      bone.OffsetMatrix = offsetIT->second;
    }

    auto transformIT = nodeTransforms.find(name);
    if (transformIT != nodeTransforms.end())
    {
      bone.BindposeMatrix = transformIT->second;
    }

    bones.push_back(bone);
    boneMap[name] = &bones.back();
    id++;
  }

  for (Bone& bone : bones)
  {
    auto parentIT = nodeParents.find(bone.Name);
    if (parentIT == nodeParents.end())
    {
      bone.Parent = -1;
    }
    else
    {
      auto parent = boneMap.find(parentIT->second);
      if (parent != boneMap.end())
      {
        bone.Parent = parent->second->ID;
      }
      else
      {
        LOG_WARN_S("ModelLoader", "Parent of bone {} not found.", bone.Name);
        bone.Parent = -1;
      }
    }

    auto childIT = nodeChildrens.find(bone.Name);
    if (childIT != nodeChildrens.end())
    {
      for (const std::string& childName : childIT->second)
      {
        auto child = boneMap.find(childName);
        if (child != boneMap.end())
        {
          bone.Childrens.push_back(child->second->ID);
        }
        else
        {
          LOG_WARN_S("ModelLoader", "Child {} of bone {} not found.", childName, bone.Name);
        }
      }
    }
  }
}

void ProcessAnimations(const aiScene* scene, const std::map<std::string, Bone*> &boneMap, const std::vector<Bone> &bones, std::vector<SkeletalAnimation>& animations)
{
  for (unsigned int i = 0; i < scene->mNumAnimations; i++)
  {
    SkeletalAnimation animation;
    animation.Duration = scene->mAnimations[i]->mDuration;
    animation.TickPerSec = scene->mAnimations[i]->mTicksPerSecond;
    animation.Name = std::string(scene->mAnimations[i]->mName.C_Str());

    std::vector<unsigned int> usedBones;

    for (unsigned int n = 0; n < scene->mAnimations[i]->mNumChannels; n++)
    {
      const aiNodeAnim* nodeAnim = scene->mAnimations[i]->mChannels[n];
      //nodeAnim->mNodeName;
      BoneAnimation boneAnim;
      boneAnim.BoneName = std::string(nodeAnim->mNodeName.C_Str());
      auto boneMapIT = boneMap.find(boneAnim.BoneName);
      ASSERT(boneMapIT != boneMap.end(), "[ModelLoader] Can't find corresponding bone for animation!");
      boneAnim.BoneID = boneMapIT->second->ID;

      usedBones.push_back(boneAnim.BoneID);

      for (unsigned int j = 0; j < nodeAnim->mNumPositionKeys; j++)
      {
        const aiVector3D translation = nodeAnim->mPositionKeys[j].mValue;
        glm::vec3 position(translation.x, translation.y, translation.z);
        double posTime = nodeAnim->mPositionKeys[j].mTime;

        boneAnim.Keyframes.push_back(Keyframe<VQS>(VQS(position), posTime));
      }

      for (unsigned int j = 0; j < nodeAnim->mNumScalingKeys; j++)
      {
        const aiVector3D scl = nodeAnim->mScalingKeys[j].mValue;
        glm::vec3 scale(scl.x, scl.y, scl.z);
        double sclTime = nodeAnim->mScalingKeys[j].mTime;

        boneAnim.Keyframes[j].value.SetScale(scale);
        //boneAnim.Scales.push_back(Keyframe<glm::vec3>(scale, sclTime));
      }

      for (unsigned int j = 0; j < nodeAnim->mNumRotationKeys; j++)
      {
        const aiQuaternion rot = nodeAnim->mRotationKeys[j].mValue;
        Quaternion rotation(rot.x, rot.y, rot.z, rot.w);
        double rotTime = nodeAnim->mRotationKeys[j].mTime;

        boneAnim.Keyframes[j].value.SetRotation(rotation);
        //boneAnim.Rotations.push_back(Keyframe<Quaternion>(rotation, rotTime));
      }

      animation.BoneAnimations.push_back(boneAnim);
    }

    for (const Bone& b : bones)
    {
      bool found = false;
      for (unsigned int id : usedBones)
      {
        if (id == b.ID)
        {
          found = true;
          break;
        }
      }

      if (found == false)
        animation.UnunsedBones.push_back(b.ID);
    }

    animations.push_back(animation);
  }
}

SkinnedMesh* ReadAssimpFile(const char* fileName)
{
  printf("Reading %s\n", fileName);
  Assimp::Importer importer;

  // A single call returning a single structure for the complete file.
  const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenNormals);

  printf("  %d animations\n", scene->mNumAnimations); // This is what 460/560 is all about
  printf("  %d meshes\n", scene->mNumMeshes);         // Verts and faces for the skin.
  printf("  %d materials\n", scene->mNumMaterials);   // More graphics info
  printf("  %d textures\n", scene->mNumTextures);     // More graphics info
  printf("\n");


  // Prints a graphical representation of the bone hierarchy.
  //showBoneHierarchy(scene, scene->mRootNode);

  //std::vector<aiNode*> boneNodes;
  //ProcessBoneHierarchy(scene->mRootNode, boneNodes);

  //aiNode* rootBone = FindRootBone(boneNodes);

  std::vector<std::string> nodeNames;
  std::map<std::string, std::string> nodeParents;
  std::map<std::string, glm::mat4> nodeTransforms;
  std::map<std::string, std::vector<std::string>> nodeChildrens;
  ProcessNodes(scene->mRootNode, nodeNames, nodeParents, nodeChildrens, nodeTransforms);

  // Prints all the animation info for each animation in the file
  /*printf("\n");
  for (int i = 0; i < scene->mNumAnimations; i++)
    showAnimation(scene->mAnimations[i]);*/

  std::vector<SkinnedVertex> vertices;
  std::vector<unsigned int> indices;
  std::map<std::string, glm::mat4> boneOffsets;

  // Prints all the mesh info for each mesh in the file
  //printf("\n");
  for (unsigned int i = 0; i < scene->mNumMeshes; i++)
  {
    //showMesh(scene->mMeshes[i]);
    
    ProcessMesh(scene->mMeshes[i], i, scene, nodeNames, vertices, indices, boneOffsets);
  }

  glm::mat4 rootMat = AiToGLMMat4(scene->mRootNode->mTransformation);
  glm::mat4 invRootMat = glm::inverse(rootMat);

  std::map<std::string, Bone*> boneMap;
  std::vector<Bone> bones;
  CreateBones(bones, boneMap, nodeNames, nodeParents, nodeChildrens, boneOffsets, nodeTransforms);

  std::vector<SkeletalAnimation> animations;
  ProcessAnimations(scene, boneMap, bones, animations);

  SkinnedMesh* newModel = new SkinnedMesh(vertices, indices, bones, invRootMat, PM_Triangles);
  newModel->SetFilePath(fileName);
  newModel->SetAnimations(animations);

  return newModel;
}

void ProcessMesh(aiMesh* mesh, unsigned int meshIndex, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<unsigned int>& indices)
{
  unsigned int VertexStartIndex = vertices.size();

  if (mesh->HasTextureCoords(0))
  {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      aiVector3D position = mesh->mVertices[i];
      aiVector3D normal = mesh->mNormals[i];
      aiVector3D uv = mesh->mTextureCoords[0][i];

      vertices.push_back(Vertex(glm::vec3(position.x, position.y, position.z), glm::vec3(normal.x, normal.y, normal.z), glm::vec2(uv.x, uv.y)));
    }
  }
  else
  {
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
      aiVector3D position = mesh->mVertices[i];
      aiVector3D normal = mesh->mNormals[i];

      vertices.push_back(Vertex(glm::vec3(position.x, position.y, position.z), glm::vec3(normal.x, normal.y, normal.z), glm::vec2()));
    }
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
    {
      indices.push_back(face.mIndices[j]);
    }
  }
}

Model* ReadObjFile(const char* fileName)
{
  Assimp::Importer importer;

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::map<std::string, glm::mat4> boneOffsets;

  const aiScene* scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenNormals);
  // Prints all the mesh info for each mesh in the file
  //printf("\n");
  for (unsigned int i = 0; i < scene->mNumMeshes; i++)
  {
    //showMesh(scene->mMeshes[i]);

    ProcessMesh(scene->mMeshes[i], i, scene, vertices, indices);
  }

  Model* newModel = new Model(vertices, indices, PM_Triangles);
  newModel->SetFilePath(fileName);

  return newModel;
}