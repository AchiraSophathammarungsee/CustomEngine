layout(std140) uniform Matrices
{
  mat4 finalMatrix;
  mat4 projMatrix;
  mat4 viewMatrix;
  vec3 camPos;
  vec3 fogColor;
  vec3 globalAmbient;
  vec2 nearFar;
  vec2 fogPlane;
  vec2 screenParam;
};

uniform mat4 modelMatrix;

vec3 ObjectToWorld(vec3 objPos)
{
  return vec3(modelMatrix * (objPos, 1.0));
}

vec4 WorldToView(vec4 world)
{
  return viewMatrix * world;
}

vec4 ViewToScreenCoord(vec4 view)
{
  // to ndc coord
  vec4 frag = projMatrix * view;
  // perspective division
  frag.xyz /= frag.w;
  // convert to NDC space
  frag.xy = frag.xy * 0.5 + 0.5;
  // convert to screen space
  frag.xy *= screenParam;
  return frag;
}

vec3 GetViewDir(vec3 worldPos)
{
  return normalize(camPos - worldPos);
}

mat3 GetTBN(mat4 boneTransform, vec3 normal, vec3 tangent, vec3 bitangent)
{
  vec4 worldNorm = modelMatrix * boneTransform * vec4(normal, 0.0f);
  vec4 worldTang = modelMatrix * boneTransform * vec4(tangent, 0.0f);
  vec4 worldBitang = modelMatrix * boneTransform * vec4(bitangent, 0.0f);
  
  vec3 T = normalize(worldTang.xyz);
  vec3 B = normalize(worldBitang.xyz);
  vec3 N = normalize(worldNorm.xyz);
  return mat3(T, B, N);
}

vec3 NormalMapping(vec3 texNorm, mat3 TBN)
{
  vec3 normal = normalize(texNorm);// * 2.0 - 1.0);
  normal = normalize(TBN * normal);
  return normal;
}

float FogAttenuation(vec3 worldPos)
{
  return clamp((fogPlane.y - length(camPos - worldPos)) / (fogPlane.y - fogPlane.x), 0.0, 1.0);
}

float linearDepth(float depthSample)
{
  float depthRange = 2.0 * depthSample - 1.0;
  float linear = 2.0 * nearFar.x * nearFar.y / (nearFar.y + nearFar.x - depthRange * (nearFar.y - nearFar.x));
  return linear;
}

float GetTextureArrayDepth(float layerID, int layerCount)
{
  return max(0, min(float(layerCount) - 1, floor(layerID + 0.5)));
}