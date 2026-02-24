#version 330 core

out vec4 FragColor;

/* Uniforms provided */
uniform vec3  iResolution;

uniform usampler3D uBrickMap;
uniform sampler3D  uAtlas;

uniform ivec3 uBrickGridDim;
uniform ivec3 uAtlasBrickDim;

uniform vec3  uInvAtlasSize;
uniform vec3  uGridStart;
uniform float uCellSize;
uniform float uTruncation;

/* Camera */
uniform vec3  camera_position;
uniform vec3  camera_forward;
uniform vec3  camera_right;
uniform vec3  camera_up;
uniform float camera_fov;

const int BRICK_SIZE = 8;
const int PHYSICAL_BRICK_SIZE = 10;
const float EPS = 1e-3;

// 2D -> 1D hash
float hash12(vec2 p)
{
    vec3 p3  = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float sampleAtlasOnly(vec3 gridPos, uint stored, ivec3 brickCoord) {
    uint atlasLinear = stored - 1u;
    
    uint bricksPerRow = uint(uAtlasBrickDim.x);

    uint bx = atlasLinear % bricksPerRow;
    uint by = atlasLinear / bricksPerRow;

    vec3 physicalAtlasOffset = vec3(
        float(bx * uint(PHYSICAL_BRICK_SIZE)),
        float(by * uint(PHYSICAL_BRICK_SIZE)),
        0.0
    );

    vec3 localPos = gridPos - vec3(brickCoord * BRICK_SIZE);
    vec3 texelCoord = physicalAtlasOffset + 1.0 + localPos;
    
    float d = textureLod(uAtlas, texelCoord * uInvAtlasSize, 0.0).r;

    return (d * 2.0 - 1.0) * uTruncation;
}

float raymarch(vec3 ro, vec3 rd, out uint outStored, out ivec3 outBrick)
{
    vec3 gridMin = uGridStart;
    vec3 gridMax = uGridStart + vec3(uBrickGridDim * BRICK_SIZE) * uCellSize;
    
    vec3 t0 = (gridMin - ro) / rd;
    vec3 t1 = (gridMax - ro) / rd;
    vec3 tNear = min(t0, t1);
    vec3 tFar = max(t0, t1);
    float entryT = max(max(tNear.x, tNear.y), tNear.z);
    float exitT = min(min(tFar.x, tFar.y), tFar.z);

    if (entryT > exitT || exitT < 0.0) return -1.0;

    float t = max(0.0, entryT);
    vec3 invRd = 1.0 / rd;

    // Fast Brick Skip
    for(int i = 0; i < 80; i++) {
        vec3 p = ro + rd * t;
        vec3 gridPos = (p - uGridStart) / uCellSize;
        ivec3 brickCoord = ivec3(floor(gridPos / float(BRICK_SIZE)));
        uint stored = texelFetch(uBrickMap, brickCoord, 0).r;

        if(stored == 0u) {
            // Empty Brick: Skip
            vec3 brickMin = vec3(brickCoord * BRICK_SIZE);
            vec3 brickMax = brickMin + vec3(BRICK_SIZE);
            vec3 tMax = max((brickMin - gridPos) * invRd, (brickMax - gridPos) * invRd);
            float skipDist = min(tMax.x, min(tMax.y, tMax.z));
            t += (skipDist + 0.01) * uCellSize;
        } else if (stored == 65535u) {
            // Solid Brick: Immediate hit!
            outStored = stored;
            outBrick = brickCoord;
            return t; 
        } else {
            // Occupied
            float d = sampleAtlasOnly(gridPos, stored, brickCoord);
            if (d < uCellSize * 0.1) {
               outStored = stored;
               outBrick = brickCoord;
               return t;
            }
            t += d;
        }

        if(t > exitT) break;
    }
    return -1.0;
}

vec3 calc_normal(vec3 pos, uint stored, ivec3 brickCoord)
{
    float e = uCellSize * 0.5; 
    
    vec3 gridPos = (pos - uGridStart) / uCellSize;

    float g = e / uCellSize;
    
    float dx = sampleAtlasOnly(gridPos + vec3(g, 0, 0), stored, brickCoord) - 
               sampleAtlasOnly(gridPos - vec3(g, 0, 0), stored, brickCoord);
    float dy = sampleAtlasOnly(gridPos + vec3(0, g, 0), stored, brickCoord) - 
               sampleAtlasOnly(gridPos - vec3(0, g, 0), stored, brickCoord);
    float dz = sampleAtlasOnly(gridPos + vec3(0, 0, g), stored, brickCoord) - 
               sampleAtlasOnly(gridPos - vec3(0, 0, g), stored, brickCoord);

    return normalize(vec3(dx, dy, dz));
}

vec3 visualize_grid(vec3 worldPos)
{
    vec3 gridPos = (worldPos - uGridStart) / uCellSize;
    vec3 voxelDist = abs(fract(gridPos) - 0.0);

    voxelDist = min(voxelDist, 1.0 - voxelDist);

    float voxelEdge = min(min(voxelDist.x, voxelDist.y), voxelDist.z);
    float voxelLine = 1.0 - smoothstep(0.0, 0.03, voxelEdge);
    vec3 brickPos = gridPos / float(BRICK_SIZE);
    vec3 brickDist = abs(fract(brickPos) - 0.0);
    
    brickDist = min(brickDist, 1.0 - brickDist);

    float brickEdge = min(min(brickDist.x, brickDist.y), brickDist.z);
    float brickLine = 1.0 - smoothstep(0.0, 0.02, brickEdge);

    // Colors
    vec3 voxelColor = vec3(1.0);
    vec3 brickColor = vec3(1.0, 0.0, 0.0);

    // Brick lines override voxel lines
    vec3 col = voxelColor * voxelLine * 0.5;
    col = mix(col, brickColor, brickLine);

    return col;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
  vec2 p = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
  
  vec3 ro = camera_position; // ray origin
  vec3 rd = normalize(p.x * camera_right + p.y * camera_up + camera_fov * camera_forward); // ray direction

  vec3 col = vec3(0.4, 0.75, 1.0) - 0.7 * rd.y; // sky, darker the higher

  uint hitStored;
  ivec3 hitBrick;
  float t = raymarch(ro, rd, hitStored, hitBrick);

  if (t > 0.0) 
  {
    vec3 pos = ro + t * rd;
    vec3 nor = calc_normal(pos, hitStored, hitBrick);

    vec3 sun_dir  = normalize(vec3(0.8, 0.4, 0.2));

    vec3 mate = vec3(0.18);
    vec3  lig = normalize( vec3(-0.1, 0.3, 0.6) );
    float sun_dif = clamp(dot(nor, sun_dir), 0.0, 1.0);
    float sky_dif = clamp(0.5 + 0.5 * dot(nor, vec3(0.0, 1.0, 0.0)), 0.0, 1.0);
    float bou_dif = clamp(0.5 + 0.5 * dot(nor, vec3(0.0, -1.0, 0.0)), 0.0, 1.0);
    
    col  = mate * vec3(7.0, 4.5, 3.0) * sun_dif;
    col += mate * vec3(0.5, 0.8, 0.9) * sky_dif;
    col += mate * vec3(0.7, 0.3, 0.2) * bou_dif;

    //col = nor;
    //col = nor / 0.5 * pos;

    // Show debug grid
    //vec3 gridVis = visualize_grid(pos);
    //col += gridVis;
  }

  col = pow(col, vec3(0.4545));  
  col += (hash12(fragCoord) - 0.5) / 255.0; // dithering
  fragColor = vec4(col, 1.0);
}

void main()
{
  vec2 fragCoord = gl_FragCoord.xy;
  mainImage(FragColor, fragCoord);
}
