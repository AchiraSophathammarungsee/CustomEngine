Property
{
}

#VERT
{
  // Input vertex data, different for all executions of this shader.
  layout(location = 0) in vec3 vPosition;
  layout(location = 4) in vec2 vUV;

  out vec2 UV;

  void main()
  {
    UV = vUV;
    gl_Position = vec4(2.f * vPosition, 1.0f);
  }
}

#FRAG
#include ../Common/shaderlib/mathutil.glsl
#include ../Common/shaderlib/common.glsl
{
  in vec2 UV;
  
  uniform sampler2D gPos;
  uniform sampler2D gNorm;
  uniform sampler2D opaqueTex;
  
  uniform float maxDist;
  uniform float resolution;
  uniform float steps;
  uniform float thickness;
  
  out vec3 SSR;
  
  bool traceScreenSpaceRay1(vec3 vsOrig, vec3 vsDir, out vec2 hitPixel, out vec3 hitPoint)
  {
    // clip to near plane
    float nearPlane = nearFar.x;
    float rayLength = ((vsOrig.z + vsDir * maxDist) < nearPlane) ? (vsOrig.z - nearPlane) / vsDir.z : maxDist;
    vec3 vsEndPoint = vsOrig + vsDir * rayLength;
    
    // Project into homogeneous clip space
    vec4 fragStart = projMatrix * vec4(vsOrig, 1.0);
    vec4 fragEnd = projMatrix * vec4(vsEndPoint, 1.0);
    float k0 = 1.0 / fragStart.w;
    float k1 = 1.0 / fragEnd.w;
    
    // The interpolated homogeneous version of the camera-space points 
    vec3 Q0 = vsOrig * k0;
    vec3 Q1 = vsEndPoint * k1;
    
    // Screen-space endpoints
    vec2 P0 = fragStart.xy * k0;
    vec2 P1 = fragEnd.xy * k1;
    
    // If the line is degenerate, make it cover at least one pixel
    // to avoid handling zero-pixel extent as a special case later
    P1 += vec2((DistSquared(P0, P1) < 0.0001) ? 0.01 : 0.0);
    vec2 delta = P1 - P0;
    
    // Permute so that the primary iteration is in x to collapse
    // all quadrant-specific DDA cases later
    bool permute = false;
    if (abs(delta.x) < abs(delta.y))
    { 
      // This is a more-vertical line
      permute = true;
      delta = delta.yx;
      P0 = P0.yx;
      P1 = P1.yx; 
    }
    
    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;
    
    // Track the derivatives of Q and k
    vec3  dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2  dP = vec2(stepDir, delta.y * invdx);
    
    // Scale derivatives by the desired pixel stride and then
    // offset the starting values by the jitter fraction
    dP *= stride; 
    dQ *= stride; 
    dk *= stride;
    P0 += dP * jitter; 
    Q0 += dQ * jitter; 
    k0 += dk * jitter;
    
    // Slide P from P0 to P1, (now-homogeneous) Q from Q0 to Q1, k from k0 to k1
    vec3 Q = Q0;
    
    // Adjust end condition for iteration direction
    float end = P1.x * stepDir;
    
    float k = k0, stepCount = 0.0, prevZMaxEstimate = vsOrig.z;
    float rayZMin = prevZMaxEstimate, rayZMax = prevZMaxEstimate;
    float sceneZMax = rayZMax + 100;
    
    for (vec2 P = P0; 
         ((P.x * stepDir) <= end) && (stepCount < steps) &&
         ((rayZMax < sceneZMax - thickness) || (rayZMin > sceneZMax)) &&
          (sceneZMax != 0); 
         P += dP, Q.z += dQ.z, k += dk, ++stepCount)
    {
      rayZMin = prevZMaxEstimate;
      rayZMax = (dQ.z * 0.5 + Q.z) / (dk * 0.5 + k);
      prevZMaxEstimate = rayZMax;
      if (rayZMin > rayZMax)
      { 
        float t = rayZMin; rayZMin = rayZMax; rayZMax = t;
      }
      
      hitPixel = permute ? P.yx : P;
      // You may need hitPixel.y = csZBufferSize.y - hitPixel.y; here if your vertical axis
      // is different than ours in screen space
      //hitPixel.y = csZBufferSize.y - hitPixel.y;
      
      sceneZMax = texelFetch(csZBuffer, ivec2(hitPixel), 0);
    }
    
    // Advance Q based on the number of steps
    Q.xy += dQ.xy * stepCount;
    hitPoint = Q * (1.0 / k);
    return (rayZMax >= sceneZMax - thickness) && (rayZMin < sceneZMax);
  }

  void main()
  {
    vec4 pos = WorldToView(vec4(texture(gPos, UV).rgb, 1.0));
    
    vec3 normalizedPos = normalize(pos.xyz);
    
    vec3 normal = normalize(WorldToView(vec4(texture(gNorm, UV).rgb, 0.0)).rgb);
    vec3 rayDir = normalize(reflect(normal, normalizedPos));
    
    // view space
    vec4 startVS = vec4(pos.xyz, 1);
    vec4 endVS = vec4(pos.xyz + rayDir * maxDist, 1);
    
    // screen space
    vec4 startFrag = ViewToScreenCoord(startVS);
    vec4 endFrag = ViewToScreenCoord(endVS);
    
    vec2 frag = startFrag.xy;
    vec2 uv = frag / screenParam;
    
    // calculate difference between the X and Y coordinates of the end and start fragments. 
    // This will be how many pixels the ray line occupies in the X and Y dimension of the screen.
    vec2 screenDiff = endFrag.xy - startFrag.xy;
    
    // The larger difference will help you determine how much to travel in the X and Y direction each iteration, 
    // how many iterations are needed to travel the entire line, and what percentage of the line does the current position represent.
    float useX = abs(screenDiff.x) >= abs(screenDiff.y) ? 1 : 0;
    float delta     = mix(abs(screenDiff.y), abs(screenDiff.x), useX) * clamp(resolution, 0, 1);
    
    // increment ray by
    vec2 increment = screenDiff / max(delta, 0.001);
    
    // search0 remember prev pos
    float search0 = 0;
    float search1 = 0;
    int hit0 = 0; // indicates there was an intersection during the first pass
    int hit1 = 0; // indicates there was an intersection during the second pass.
    
    float viewDist = startVS.z;
    
    SSR = vec3(viewDist);
    return;
    
    // view distance difference between the current ray point and scene position.
    // It tells you how far behind or in front of the scene the ray currently is.
    float depth = thickness;
    
    vec4 finalPos = pos;
    
    // first pass
    for(int i = 0; i < int(delta); i++)
    {
      frag += increment;
      uv = frag / screenParam;
      finalPos = WorldToView(vec4(texture(gPos, uv).rgb, 1.0));
      
      // Calculate the percentage or portion of the line the current fragment represents. 
      // If useX is zero, use the Y dimension of the line. If useX is one, use the X dimension of the line.
      // search1 is the percentage or portion of the line the current position represents. 
      // You'll need this to interpolate between the ray's view-space start and end distances from the camera.
      search1 = mix((frag.y - startFrag.y) / screenDiff.y, (frag.x - startFrag.x) / screenDiff.x, useX);
      search1 = clamp(search1, 0.0, 1.0);
      
      // (startVS.z * endVS.z): need to perform perspective-correct interpolation which you see here.
      viewDist = (startVS.z * endVS.z) / mix(endVS.z, startVS.z, search1);
      depth = finalPos.z - viewDist;
      
      // If the difference is between zero and the thickness, this is a hit. Set hit0 to one and exit the first pass. 
      // If the difference is not between zero and the thickness, this is a miss. 
      // Set search0 to equal search1 to remember this position as the last known miss. 
      // Continue marching the ray towards the end fragment.
      if(depth > 0 && depth < thickness)
      {
        hit0 = 1;
        break;
      }
      else
      {
        search0 = search1;
      }
    }
    
    // Set the search1 position to be halfway between the position of the last miss and the position of the last hit.
    search1 = search0 + ((search1 - search0) / 2.0);
    
    // second pass
    float passsteps = steps * hit0;
    for(int i = 0; i < passsteps; i++)
    {
      frag = mix(startFrag.xy, endFrag.xy, search1);
      uv = frag / screenParam;
      finalPos = WorldToView(vec4(texture(gPos, uv).rgb, 1.0));
      
      viewDist = (startVS.z * endVS.z) / mix(endVS.z, startVS.z, search1);
      depth = finalPos.z - viewDist;
      
      // If the depth is within bounds, this is a hit.
      // Set hit1 to one and set search1 to be halfway between the last known miss position and this current hit position.
      // If the depth is not within bounds, this is a miss.
      // Set search1 to be halfway between this current miss position and the last known hit position. 
      // Move search0 to this current miss position. Continue this back and forth search while i is less than steps.
      if (depth > 0 && depth < thickness)
      {
        hit1 = 1;
        search1 = search0 + ((search1 - search0) / 2);
      }
      else
      {
        float temp = search1;
        search1 = search1 + ((search1 - search0) / 2);
        search0 = temp;
      }
    }
    
    // You're now done with the second and final pass but before you can output the reflected UV coordinates, 
    // you'll need to calculate the visibility of the reflection. The visibility ranges from zero to one. 
    // If there wasn't a hit in the second pass, the visibility is zero.
    
    // If the reflected scene position's alpha or w component is zero, the visibility is zero. 
    // Note that if w is zero, there was no scene position at that point.
    float visibility = hit1 * finalPos.w;
    
    // One of the ways in which screen space reflection can fail is when the reflection ray points in the general direction of the camera.
    // If the reflection ray points towards the camera and hits something, it's most likely hitting the back side of something facing away from the camera.
    
    // To handle this failure case, you'll need to gradually fade out the reflection based on how much the reflection vector points to the camera's position. 
    // If the reflection vector points directly in the opposite direction of the position vector, the visibility is zero. 
    // Any other direction results in the visibility being greater than zero.
    visibility *= (1 - max( dot(normalizedPos, rayDir), 0));
    
    // As you sample scene positions along the reflection ray, 
    // you're hoping to find the exact point where the reflection ray first intersects with the scene's geometry. 
    // Unfortunately, you may not find this particular point. 
    // Fade out the reflection the further it is from the intersection point you did find.
    visibility *= (1 - clamp(depth / thickness, 0, 1));
    
    // Fade out the reflection based on how far way the reflected point is from the initial starting point. 
    // This will fade out the reflection instead of it ending abruptly as it reaches maxDistance.
    visibility *= (1 - clamp(length(finalPos.xyz - pos.xyz) / maxDist, 0, 1));
    
    // If the reflected UV coordinates are out of bounds, set the visibility to zero. 
    // This occurs when the reflection ray travels outside the camera's frustum.
    visibility *= (uv.x < 0 || uv.x > 1 ? 0 : 1) * (uv.y < 0 || uv.y > 1 ? 0 : 1);
    
    visibility = clamp(visibility, 0, 1);
    
    SSR = texture(opaqueTex, uv).rgb * visibility;
  }
}