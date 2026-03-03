#version 330 core

out vec4 FragColor;

uniform vec3  iResolution;
uniform usampler3D uBrickMap;
uniform sampler3D  uAtlas;
uniform sampler3D uMaterial;
uniform sampler1D uPalette;

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
const float fPHYSICAL_BRICK_SIZE = 10.0;
const float EPS = 0.01;

vec3 getAtlasOffset(uint stored) {
    uint atlasLinear = stored - 1u;
    uint bricksPerRow = uint(uAtlasBrickDim.x);
    vec2 offset;
    offset.y = float(atlasLinear / bricksPerRow);
    offset.x = float(atlasLinear - (offset.y * bricksPerRow));
    return vec3(offset * fPHYSICAL_BRICK_SIZE, 0.0) + 1.0;
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

/*
vec3 sampleMaterial(vec3 gridPos, vec3 atlasOffset, ivec3 brickCoord) {
    vec3 localPos = gridPos - vec3(brickCoord * BRICK_SIZE);
    vec3 texelCoord = atlasOffset + localPos;
    float matID = texture(uMaterial, texelCoord * uInvAtlasSize).r;
    return texture(uPalette, matID).rgb;
}
*/

vec3 sampleMaterial(vec3 gridPos, vec3 atlasOffset, ivec3 brickCoord) {
    vec3 localPos = gridPos - vec3(brickCoord * BRICK_SIZE);
    vec3 texelCoord = atlasOffset + localPos;
    float matIDNormalized = texture(uMaterial, texelCoord * uInvAtlasSize).r;
    float id = matIDNormalized * 255.0;
    float paletteCoord = (id + 0.5) / 256.0;
    return texture(uPalette, paletteCoord).rgb;
}

vec3 debugColor(ivec3 p) {
    uvec3 v = uvec3(p);
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.z; v.y += v.z * v.x; v.z += v.x * v.y;
    v ^= v >> 16u;
    v.x += v.y * v.z; v.y += v.z * v.x; v.z += v.x * v.y;
    return vec3(v & 255u) / 255.0;
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

            if (hitStored == 65535u) // Solid
            {
                hitT = t; break;
            } 
            else if (hitStored > 0u) // SDF Occupied
            {
                vec3 atlasOff = getAtlasOffset(hitStored);
                float localT = t;
                float brickExitT = min(min(tMax.x, tMax.y), tMax.z);
                
                // Inner Loop: Sphere Tracing inside one brick
                vec3 pStart = (ro + rd * localT - uGridStart) * invCell;
                vec3 rdStep = (rd * invCell);

                for(int j = 0; j < 32; j++) {
                    float d = sampleAtlas(pStart, atlasOff, brickCoord);
                    if (d < EPS) { hitT = localT; break; }
                    localT += d;
                    pStart += rdStep * d;
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
            float e = 0.1;

            vec3 normal = normalize(
                k.xyy * sampleAtlas(gP + k.xyy*e, atlasOff, brickCoord) +
                k.yyx * sampleAtlas(gP + k.yyx*e, atlasOff, brickCoord) +
                k.yxy * sampleAtlas(gP + k.yxy*e, atlasOff, brickCoord) +
                k.xxx * sampleAtlas(gP + k.xxx*e, atlasOff, brickCoord)
            );

            vec3 material = sampleMaterial(gP, atlasOff, brickCoord);
            float diffuse = clamp(dot(normal, normalize(vec3(0.7, 0.9, 0.3))), 0.0, 1.0);
            vec3 ambient  = vec3(0.2, 0.3, 0.4);
            vec3 sun      = vec3(0.8, 0.7, 0.5);

            //col = material * (dif + 0.15);
            //col = vec3(0.2, 0.3, 0.4) + dif * vec3(0.8, 0.7, 0.5);           
            col = material * (ambient + diffuse * sun);
            //col = ambient + diffuse * sun;
            
            /*
            ivec3 voxelCoord = ivec3(floor((pos - uGridStart) * invCell));
            vec3 brick_color = debugColor(brickCoord);
            vec3 voxel_color = debugColor(voxelCoord);
            col = brick_color * (0.7 + 0.3 * voxel_color);
            */
        }
    }

    fragColor = vec4(pow(col, vec3(0.4545)), 1.0);
}

void main()
{
  vec2 fragCoord = gl_FragCoord.xy;
  mainImage(FragColor, fragCoord);
}
