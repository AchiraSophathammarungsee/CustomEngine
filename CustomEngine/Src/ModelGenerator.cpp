/* Start Header -------------------------------------------------------
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: ModelGenerator.cpp
Purpose: Procedurally generate sphere, ring, quad, and cube meshes
Language: C++, GLSL
Platform: Visual studio 32-bit debug/release, OpenGL 4.6+, Windows
Project: achira.s_CS300_1
Author: Achira Sophathammarungsee, achira.s, 18002517>
Creation date: 9/12/2019
End Header --------------------------------------------------------*/

#include "Model.h"
#include <glm/glm.hpp>
# define M_PI 3.14159265358979323846  /* pi */

namespace ModelGenerator
{
  Model* CreateSphere(float radius, int numDivisions)
  {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    // sector = horizontal stripe of sphere
    // stack = vertical stripe of sphere
    float sectorStep = 2 * (float)M_PI / (float)numDivisions;
    float stackStep = (float)M_PI / (float)numDivisions;

    // sectorAngle = horizontal angle
    // stackAngle = vertical angle
    float sectorAngle, stackAngle;

    for (int i = 0; i <= numDivisions; i++)
    {
      stackAngle = ((float)M_PI / 2.f) - (((float)i) * stackStep);

      glm::vec3 pos;
      pos.z = radius * sin(stackAngle);

      for (int j = 0; j <= numDivisions; j++)
      {
        sectorAngle = j * sectorStep;

        pos.x = radius * cos(stackAngle) * cos(sectorAngle);
        pos.y = radius * cos(stackAngle) * sin(sectorAngle);

        vertices.push_back(Vertex(pos));
      }
    }

    //Once all the sphere points are generated, create triangles
    int p1, p2;
    for (int i = 0; i < numDivisions; ++i)
    {
      // p1 = first vertex of ring
      // p2 = first vertex of next ring
      p1 = i * (numDivisions + 1);
      p2 = p1 + numDivisions + 1;

      for (int j = 0; j < numDivisions; ++j, ++p1, ++p2)
      {
        // create 2 triangles per quad of the ring, excluding first and last ring
        if (i != 0)
        {
          Face f;
          unsigned int faceID = faces.size();

          f.indices[0] = p1;
          f.indices[1] = p2;
          f.indices[2] = p1 + 1;
          faces.push_back(f);

          vertices[p1].sharingFaces.push_back(faceID);
          vertices[p2].sharingFaces.push_back(faceID);
          vertices[p1 + 1].sharingFaces.push_back(faceID);
        }

        if (i != (numDivisions - 1))
        {
          Face f;
          unsigned int faceID = faces.size();

          f.indices[0] = p1 + 1;
          f.indices[1] = p2;
          f.indices[2] = p2 + 1;
          faces.push_back(f);

          vertices[p1 + 1].sharingFaces.push_back(faceID);
          vertices[p2].sharingFaces.push_back(faceID);
          vertices[p2 + 1].sharingFaces.push_back(faceID);
        }
      }
    }

    // Create a new Mesh from the vertices and the indices
    Model* sphere = new Model(vertices, faces, PM_Triangles, NBM_Vertex);
    sphere->boundingBox.UpdateVertices(radius, -radius, radius, -radius, radius, -radius);

    return sphere;
  }

  Model* CreateCircle(float radius, int segments)
  {
    std::vector<Vertex> orbitVertices;
    std::vector<unsigned int> orbitIndices;
    for (int i = 0; i < segments; i++)
    {
      float step = (float)i / (float)segments * (2.f * (float)M_PI);
      orbitVertices.push_back(Vertex(glm::vec3(radius * cos(step), 0.f, radius * sin(step))));
    }
    for (int i = 0; i < segments; i++)
    {
      orbitIndices.push_back(i);
    }
    orbitIndices.push_back(0);

    return new Model(orbitVertices, orbitIndices, PM_Line_Strip);
  }

  Model* CreateLineCircleSphere(float radius, int segments)
  {
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    unsigned int id = 0;

    // horizontal circle
    for (int i = 0; i < segments; i++)
    {
      float step = (float)i / (float)segments * (2.f * (float)M_PI);
      verts.push_back(Vertex(glm::vec3(radius * cos(step), 0.f, radius * sin(step))));
      indices.push_back(id);
      id++;
    }
    indices.push_back(0);

    // vertical circle
    unsigned int begin = id;
    for (int i = 0; i < segments; i++)
    {
      float step = (float)i / (float)segments * (2.f * (float)M_PI);
      verts.push_back(Vertex(glm::vec3(radius * cos(step), radius * sin(step), 0.f)));
      indices.push_back(id);
      id++;
    }
    indices.push_back(begin);

    // connection to next circle
    for (int i = 1; i < segments / 4; i++)
    {
      indices.push_back(i);
    }

    // vertical circle
    begin = id;
    for (int i = 0; i < segments; i++)
    {
      float step = (float)i / (float)segments * (2.f * (float)M_PI);
      verts.push_back(Vertex(glm::vec3(0.f, radius * sin(step), radius * cos(step))));
      indices.push_back(id);
      id++;
    }
    indices.push_back(begin);

    return new Model(verts, indices, PM_Line_Strip);
  }

  Model* CreateLine()
  {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    vertices.push_back(glm::vec3(0.f));
    vertices.push_back(glm::vec3(1.f, 0.f, 0.f));
    indices.push_back(0);
    indices.push_back(1);

    return new Model(vertices, indices, PM_Line_Strip);
  }

  Model* CreateLineTriangle()
  {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.push_back(glm::vec3(-0.5f, -0.5f, -0.5f));
    vertices.push_back(glm::vec3(0.5f, 0.5f, -0.5f));
    vertices.push_back(glm::vec3(0.f, 0.f, 0.5f));
    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);

    return new Model(vertices, indices, PM_Line_Strip, true);
  }

  Model* CreateSquare()
  {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(0);

    return new Model(vertices, indices, PM_Line_Strip);
  }

  Model* CreateLineCube()
  {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));

    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));

    indices.push_back(0);
    indices.push_back(4);
    indices.push_back(5);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(6);
    indices.push_back(7);
    indices.push_back(3);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(5);
    indices.push_back(6);
    indices.push_back(2);
    indices.push_back(3);
    indices.push_back(7);
    indices.push_back(4);

    return new Model(vertices, indices, PM_Line_Strip);
  }

  Model* CreateLineSphere(int numDivisions)
  {
    float radius = 0.5f;
    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;

    // sector = horizontal stripe of sphere
    // stack = vertical stripe of sphere
    float sectorStep = 2 * (float)M_PI / (float)numDivisions;
    float stackStep = (float)M_PI / (float)numDivisions;

    // sectorAngle = horizontal angle
    // stackAngle = vertical angle
    float sectorAngle, stackAngle;

    int index = 0;
    for (int i = 0; i <= numDivisions; i++)
    {
      stackAngle = ((float)M_PI / 2.f) - (((float)i) * stackStep);

      glm::vec3 pos;
      pos.z = radius * sin(stackAngle);

      for (int j = 0; j <= numDivisions; j++)
      {
        sectorAngle = j * sectorStep;

        pos.x = radius * cos(stackAngle) * cos(sectorAngle);
        pos.y = radius * cos(stackAngle) * sin(sectorAngle);

        vertices.push_back(Vertex(pos));
        indices.push_back(index);
        index++;
      }
    }

    for (int i = 1; i <= numDivisions - 1; i++)
    {
      int beginIndex = (i % 2) ? vertices.size() - 1 : 0;
      int sign = (i % 2) ? -1 : 1;
      for (int j = 0; j <= numDivisions; j++)
      {
        indices.push_back(beginIndex + sign * (j * numDivisions + i + j));
      }
    }

    // Create a new Mesh from the vertices and the indices
    return new Model(vertices, indices, PM_Line_Strip);
  }

  Model* CreateQuad()
  {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, 0.f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, 0.f), glm::vec3(0.f), glm::vec2(1.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, 0.f), glm::vec3(0.f), glm::vec2(0.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, 0.f), glm::vec3(0.f), glm::vec2(1.f, 0.f)));

    faces.push_back(Face(glm::vec3(2, 0, 1)));
    faces.push_back(Face(glm::vec3(1, 0, 3)));

    vertices[0].sharingFaces.push_back(0);
    vertices[1].sharingFaces.push_back(0);
    vertices[2].sharingFaces.push_back(0);

    vertices[1].sharingFaces.push_back(1);
    vertices[0].sharingFaces.push_back(1);
    vertices[3].sharingFaces.push_back(1);

    Model* quad = new Model(vertices, faces, PM_Triangles, NBM_Face);
    quad->boundingBox.UpdateVertices(0.5f, -0.5f, 0.5f, -0.5f, 0.f, -0.f);

    return quad;
  }

  Model * CreateCube()
  {
    std::vector<Vertex> vertices;
    std::vector<Face> faces;

    // front
    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.f), glm::vec2(1.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(1.f, 0.f)));
    // back
    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.f), glm::vec2(1.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f), glm::vec2(1.f, 0.f)));
    // top
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.f), glm::vec2(1.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.f), glm::vec2(1.f, 0.f)));
    // bottom
    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(1.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f), glm::vec2(1.f, 0.f)));
    // right
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.f), glm::vec2(1.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.f), glm::vec2(1.f, 0.f)));
    // left
    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.f), glm::vec2(0.f, 0.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.f), glm::vec2(1.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.f), glm::vec2(0.f, 1.f)));
    vertices.push_back(Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.f), glm::vec2(1.f, 0.f)));

    // front
    faces.push_back(Face(glm::vec3(2, 0, 1)));
    faces.push_back(Face(glm::vec3(1, 0, 3)));
    // back
    faces.push_back(Face(glm::vec3(6, 5, 4)));
    faces.push_back(Face(glm::vec3(4, 5, 7)));
    // top
    faces.push_back(Face(glm::vec3(9, 8, 10)));
    faces.push_back(Face(glm::vec3(11, 8, 9)));
    // bottom
    faces.push_back(Face(glm::vec3(14, 12, 13)));
    faces.push_back(Face(glm::vec3(13, 12, 15)));
    // right
    faces.push_back(Face(glm::vec3(18, 16, 17)));
    faces.push_back(Face(glm::vec3(17, 16, 19)));
    // left
    faces.push_back(Face(glm::vec3(21, 20, 22)));
    faces.push_back(Face(glm::vec3(23, 20, 21)));

    // front face
    vertices[2].sharingFaces.push_back(0);
    vertices[0].sharingFaces.push_back(0);
    vertices[1].sharingFaces.push_back(0);
    vertices[1].sharingFaces.push_back(1);
    vertices[0].sharingFaces.push_back(1);
    vertices[3].sharingFaces.push_back(1);
    // back face
    vertices[6].sharingFaces.push_back(2);
    vertices[5].sharingFaces.push_back(2);
    vertices[4].sharingFaces.push_back(2);
    vertices[4].sharingFaces.push_back(3);
    vertices[5].sharingFaces.push_back(3);
    vertices[7].sharingFaces.push_back(3);
    // top face
    vertices[9].sharingFaces.push_back(4);
    vertices[8].sharingFaces.push_back(4);
    vertices[10].sharingFaces.push_back(4);
    vertices[11].sharingFaces.push_back(5);
    vertices[8].sharingFaces.push_back(5);
    vertices[9].sharingFaces.push_back(5);
    // bottom face
    vertices[14].sharingFaces.push_back(6);
    vertices[12].sharingFaces.push_back(6);
    vertices[13].sharingFaces.push_back(6);
    vertices[13].sharingFaces.push_back(7);
    vertices[12].sharingFaces.push_back(7);
    vertices[15].sharingFaces.push_back(7);
    // right face
    vertices[18].sharingFaces.push_back(8);
    vertices[16].sharingFaces.push_back(8);
    vertices[17].sharingFaces.push_back(8);
    vertices[17].sharingFaces.push_back(9);
    vertices[16].sharingFaces.push_back(9);
    vertices[19].sharingFaces.push_back(9);
    // left face
    vertices[21].sharingFaces.push_back(10);
    vertices[20].sharingFaces.push_back(10);
    vertices[22].sharingFaces.push_back(10);
    vertices[23].sharingFaces.push_back(11);
    vertices[20].sharingFaces.push_back(11);
    vertices[21].sharingFaces.push_back(11);

    Model * cube = new Model(vertices, faces, PM_Triangles, NBM_Vertex);
    cube->boundingBox.UpdateVertices(0.5f, -0.5f, 0.5f, -0.5f, 0.5f, -0.5f);

    return cube;
  }
}