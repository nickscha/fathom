#version 330 core

out vec4 FragColor;

/* Uniforms provided */
uniform vec3  iResolution;
uniform float iTime;
uniform float iTimeDelta;
uniform int   iFrame;
uniform float iFrameRate;
uniform vec4  iMouse;

uniform usampler3D uBrickMap;
uniform sampler3D  uAtlas;

uniform ivec3 uBrickGridDim;
uniform ivec3 uAtlasBrickDim;

uniform vec3  uGridStart;
uniform float uCellSize;
uniform float uTruncation;

const int BRICK_SIZE = 8;
const int PHYSICAL_BRICK_SIZE = 10;

float sampleSparseSDF(vec3 worldPos)
{
    vec3 gridPos = (worldPos - uGridStart) / uCellSize;
    ivec3 brickCoord = ivec3(floor(gridPos / float(BRICK_SIZE)));

    if(any(lessThan(brickCoord, ivec3(0))) || any(greaterThanEqual(brickCoord, uBrickGridDim))) {
        return uTruncation; 
    }

    uint stored = texelFetch(uBrickMap, brickCoord, 0).r;
    if(stored == 0u) return uTruncation; 

    uint atlasLinear = stored - 1u;
    uint sliceSize = uint(uAtlasBrickDim.x * uAtlasBrickDim.y);
    vec3 physicalAtlasOffset = vec3(
        float(atlasLinear % uint(uAtlasBrickDim.x)),
        float((atlasLinear / uint(uAtlasBrickDim.x)) % uint(uAtlasBrickDim.y)),
        float(atlasLinear / sliceSize)
    ) * float(PHYSICAL_BRICK_SIZE);

    vec3 localPos = gridPos - vec3(brickCoord * BRICK_SIZE);

    /* Add 0.5 to localPos to hit texel centers. 
       The +1.0 moves us past the left-side apron.
    */
    vec3 texelCoord = physicalAtlasOffset + 1.0 + localPos;

    /* Normalize using the PHYSICAL size */
    vec3 totalAtlasSize = vec3(uAtlasBrickDim) * float(PHYSICAL_BRICK_SIZE);
    
    // texture() expects [0, 1]
    float d = texture(uAtlas, texelCoord / totalAtlasSize).r;

    return (d * 2.0 - 1.0) * uTruncation;
}

float raymarch(vec3 ro, vec3 rd)
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
    for(int i = 0; i < 64; i++)
    {
        vec3 p = ro + rd * t;
        vec3 gridPos = (p - uGridStart) / uCellSize;
        ivec3 brickCoord = ivec3(floor(gridPos / float(BRICK_SIZE)));

        /* Fast Bounds Check */
        if(any(lessThan(brickCoord, ivec3(0))) || any(greaterThanEqual(brickCoord, uBrickGridDim))) {
            t += uCellSize; 
            if(t > exitT) break;
            continue;
        }

        uint stored = texelFetch(uBrickMap, brickCoord, 0).r;

        if(stored == 0u) {
            /* Skip the whole 8x8x8 block at once */
            vec3 brickMin = vec3(brickCoord * BRICK_SIZE);
            vec3 brickMax = brickMin + vec3(BRICK_SIZE);
            
            vec3 tb0 = (brickMin - gridPos) * invRd;
            vec3 tb1 = (brickMax - gridPos) * invRd;
            vec3 tMax = max(tb0, tb1);
            float skipDist = min(tMax.x, min(tMax.y, tMax.z));

            t += (skipDist + 0.01) * uCellSize;
        }
        else {
            float d = sampleSparseSDF(p);
            if(d < uCellSize * 0.1) return t;
            t += d;
        }

        if(t > exitT) break;
    }
    return -1.0;
}

vec3 calc_normal(vec3 p)
{
    float e = uCellSize * 0.5; 
    vec2 k = vec2(1.0, -1.0);
    
    return normalize(
        k.xyy * sampleSparseSDF(p + k.xyy * e) +
        k.yyx * sampleSparseSDF(p + k.yyx * e) +
        k.yxy * sampleSparseSDF(p + k.yxy * e) +
        k.xxx * sampleSparseSDF(p + k.xxx * e)
    );
}

float ambientOcclusion(vec3 pos, vec3 nor)
{
    float occ = 0.0;
    float sca = 1.0;

    for(int i = 0; i < 5; i++)
    {
        float h = uCellSize * (0.5 + 2.0 * float(i) / 4.0);

        float d = sampleSparseSDF(pos + nor * h);

        occ += (h - d) * sca;
        sca *= 0.85;
    }

    return clamp(1.0 - 2.5 * occ, 0.0, 1.0);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
  vec2 p = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
  
  float an = 0.1 * iTime; //10.0 * iMouse.x / iResolution.x;

  vec3 ro = vec3(0.0, 1.0, 2.0); // ray origin
  vec3 ta = vec3(0.0, 0.0, 0.0);

  vec3 ww = normalize(ta-ro);
  vec3 uu = normalize(cross(ww, vec3(0, 1, 0)));
  vec3 vv = normalize(cross(uu, ww));

  vec3 rd = normalize(p.x * uu + p.y * vv + 1.5 * ww); // ray direction

  vec3 col = vec3(0.4, 0.75, 1.0) - 0.7 * rd.y; // sky, darker the higher

  float t = raymarch(ro, rd);

  if (t > 0.0) 
  {
    vec3 pos = ro + t * rd;
    vec3 nor = calc_normal(pos);

    float ao = ambientOcclusion(pos, nor);

    vec3 sun_dir  = normalize(vec3(0.8, 0.4, 0.2));

    vec3 mate = vec3(0.18);
    vec3  lig = normalize( vec3(-0.1, 0.3, 0.6) );
    float sun_dif = clamp(dot(nor, sun_dir), 0.0, 1.0);
    float sky_dif = clamp(0.5 + 0.5 * dot(nor, vec3(0.0, 1.0, 0.0)), 0.0, 1.0);
    float bou_dif = clamp(0.5 + 0.5 * dot(nor, vec3(0.0, -1.0, 0.0)), 0.0, 1.0);
    col  = mate * vec3(7.0, 4.5, 3.0) * sun_dif;
    col += mate * vec3(0.5, 0.8, 0.9) * sky_dif * ao;
    col += mate * vec3(0.7, 0.3, 0.2) * bou_dif;
    col *= ao;

    //col = nor;
    //col = nor / 0.5 * pos;
  }

  col = pow(col, vec3(0.4545));  

  fragColor = vec4(col, 1.0);
}

void main()
{
  vec2 fragCoord = gl_FragCoord.xy;
  mainImage(FragColor, fragCoord);
}
