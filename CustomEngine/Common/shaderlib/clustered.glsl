// Cluster shading structs and buffers
struct PointLight
{
  vec4 position;
  vec4 color;
  bool enabled;
  float intensity;
  float range;
  float haveShadow;
};
struct LightGrid
{
  uint offset;
  uint count;
};
struct VolumeTileAABB
{
  vec4 minPoint;
  vec4 maxPoint;
};

layout(std430, binding = 1) buffer clusterAABB
{
  VolumeTileAABB cluster[];
};
layout(std430, binding = 2) buffer screenToView
{
  mat4 inverseProjection;
  uvec4 tileSizes;
  uvec2 screenDimensions;
  float scale;
  float bias;
};

layout(std430, binding = 3) buffer lightSSBO
{
  PointLight pointLight[];
};
layout(std430, binding = 4) buffer lightIndexSSBO
{
  uint globalLightIndexList[];
};
layout(std430, binding = 5) buffer lightGridSSBO
{
  LightGrid lightGrid[];
};

vec3 debugColors[8] = vec3[]
(
  vec3(0, 0, 0),    vec3(0,  0,  1), vec3(0, 1, 0),  vec3(0, 1,  1),
  vec3(1,  0,  0),  vec3(1,  0,  1), vec3(1, 1, 0),  vec3(1, 1, 1)
);

uint CurrentCluster(vec3 fragCoord)
{
  uint zTile = uint(max(log2(linearDepth(fragCoord.z)) * scale + bias, 0.0));
  uvec3 tiles = uvec3(uvec2(fragCoord.xy / tileSizes[3]), zTile);
  uint tileIndex = tiles.x +
                   tileSizes.x * tiles.y +
                   (tileSizes.x * tileSizes.y) * tiles.z;
  return tileIndex;
}

uint CurrentTile(vec3 fragCoord)
{
  uvec2 tiles = uvec2(fragCoord.xy / tileSizes[3]);
  uint tileIndex = tiles.x +
                   tileSizes.x * tiles.y;
  return tileIndex;
}

uint CurrentClusterZ(vec3 fragCoord)
{
  uint zTile = uint(max(log2(linearDepth(fragCoord.z)) * scale + bias, 0.0));
  return zTile;
}

vec3 DebugCluster(uint clusterID)
{
  uint clrID = uint(clusterID - (7 * floor(clusterID/7)));
  return debugColors[clrID];
}