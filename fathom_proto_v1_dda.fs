#version 330 core

out vec4 FragColor;

uniform vec3  iResolution;
uniform usampler3D uBrickMap;
uniform sampler3D  uAtlas;
uniform usampler3D uMaterial;

uniform ivec3 uAtlasBrickDim;
uniform vec3  uInvAtlasSize;
uniform vec3  uGridStart;
uniform float uCellSize;
uniform float uTruncation;

uniform vec3  camera_position;
uniform vec3  camera_forward;
uniform vec3  camera_right;
uniform vec3  camera_up;
uniform vec3  camera_forward_scaled;

const int   BRICK_SIZE = 8;
const float fBRICK_SIZE = 8.0;
const int   PHYSICAL_BRICK_SIZE = 10;
const float EPS = 0.01;

vec3 getAtlasOffset(uint stored) {
    uint atlasLinear = stored - 1u;
    uint bricksPerRow = uint(uAtlasBrickDim.x);
    vec2 offset;
    offset.x = float(atlasLinear % bricksPerRow);
    offset.y = float(atlasLinear / bricksPerRow);
    return vec3(offset * float(PHYSICAL_BRICK_SIZE), 0.0) + 1.0;
}

float sampleAtlas(vec3 gridPos, vec3 atlasOffset, ivec3 brickCoord) {
    vec3 localPos = gridPos - vec3(brickCoord * BRICK_SIZE);
    vec3 texelCoord = atlasOffset + localPos;
    float d = texture(uAtlas, texelCoord * uInvAtlasSize).r;
    
    #ifdef FATHOM_SPARSE_GRID_QUANTIZE_U8
        return (d * 2.0 - 1.0) * uTruncation;
    #else 
        return d * uTruncation;
    #endif
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    vec3 ro = camera_position; // ray origin
    vec3 rd = normalize(uv.x * camera_right + uv.y * camera_up + camera_forward_scaled); // ray direction

    vec3 gridMin = uGridStart;
    vec3 gridMax = uGridStart + vec3(textureSize(uBrickMap, 0) * BRICK_SIZE) * uCellSize;
    
    vec3 invRd = 1.0 / rd;
    vec3 t0 = (gridMin - ro) * invRd;
    vec3 t1 = (gridMax - ro) * invRd;
    float tNear = max(max(min(t0.x, t1.x), min(t0.y, t1.y)), min(t0.z, t1.z));
    float tFar  = min(min(max(t0.x, t1.x), max(t0.y, t1.y)), max(t0.z, t1.z));

    vec3 col = vec3(0.4, 0.75, 1.0) - 0.7 * rd.y; // Sky

    if (tNear < tFar && tFar > 0.0) {
        float t = max(0.0, tNear) + EPS;
        float hitT = -1.0;
        uint hitStored = 0u;
        ivec3 hitBrick;

        // DDA Setup for Brick Skipping
        vec3 invCell = vec3(1.0 / uCellSize);
        vec3 gridP = (ro + rd * t - uGridStart) * invCell;
        vec3 brickP = gridP / fBRICK_SIZE;
        ivec3 brickCoord = ivec3(floor(brickP));
        
        vec3 rdSign = sign(rd);
        vec3 tDelta = abs((fBRICK_SIZE * uCellSize) * invRd);
        vec3 tMax = ((vec3(brickCoord) + max(rdSign, 0.0)) * fBRICK_SIZE - gridP) * uCellSize * invRd + t;

        // Brick Traversal (DDA)
        for(int i = 0; i < 64; i++) {
            hitStored = texelFetch(uBrickMap, brickCoord, 0).r;

            if (hitStored == 65535u) { // Solid
                hitT = t; break;
            } 
            else if (hitStored > 0u) { // SDF Occupied
                vec3 atlasOff = getAtlasOffset(hitStored);
                float localT = t;
                float brickExitT = min(min(tMax.x, tMax.y), tMax.z);
                
                // Inner Loop: Sphere Tracing inside one brick
                for(int j = 0; j < 32; j++) {
                    vec3 p = (ro + rd * localT - uGridStart) * invCell;
                    float d = sampleAtlas(p, atlasOff, brickCoord);
                    if (d < EPS) { hitT = localT; break; }
                    localT += d;
                    if (localT > brickExitT) break;
                }
                if (hitT > 0.0) break;
            }

            // Standard DDA Step to next brick
            if (tMax.x < tMax.y) {
                if (tMax.x < tMax.z) { t = tMax.x; tMax.x += tDelta.x; brickCoord.x += int(rdSign.x); }
                else { t = tMax.z; tMax.z += tDelta.z; brickCoord.z += int(rdSign.z); }
            } else {
                if (tMax.y < tMax.z) { t = tMax.y; tMax.y += tDelta.y; brickCoord.y += int(rdSign.y); }
                else { t = tMax.z; tMax.z += tDelta.z; brickCoord.z += int(rdSign.z); }
            }
            if (t > tFar) break;
        }

        if (hitT > 0.0) {
            vec3 pos = ro + rd * hitT;
            vec3 gP = (pos - uGridStart) * invCell;
            vec3 atlasOff = getAtlasOffset(hitStored);
            
            vec2 k = vec2(1.0, -1.0);
            vec3 nor = normalize(
                k.xyy * sampleAtlas(gP + k.xyy*0.1, atlasOff, brickCoord) +
                k.yyx * sampleAtlas(gP + k.yyx*0.1, atlasOff, brickCoord) +
                k.yxy * sampleAtlas(gP + k.yxy*0.1, atlasOff, brickCoord) +
                k.xxx * sampleAtlas(gP + k.xxx*0.1, atlasOff, brickCoord)
            );

            // Simple lighting
            float dif = clamp(dot(nor, normalize(vec3(0.7, 0.9, 0.3))), 0.0, 1.0);
            col = vec3(0.2, 0.3, 0.4) + dif * vec3(0.8, 0.7, 0.5);
        }
    }

    fragColor = vec4(pow(col, vec3(0.4545)), 1.0);
}

void main()
{
  vec2 fragCoord = gl_FragCoord.xy;
  mainImage(FragColor, fragCoord);
}
