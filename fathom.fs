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
uniform float uCellDiagonal;

const int BRICK_SIZE = 8;

uint fetchBrickIndex(ivec3 b)
{
    return texelFetch(uBrickMap, b, 0).r;
}

ivec3 atlasBrickXYZ(uint idx)
{
    uint linear = idx; // already -1 handled outside

    uint w = uint(uAtlasBrickDim.x);
    uint h = uint(uAtlasBrickDim.y);

    uint bx = linear % w;
    uint by = (linear / w) % h;
    uint bz = linear / (w*h);

    return ivec3(bx,by,bz);
}

/* 1. Safely fetches a SINGLE voxel, handling indirection and boundaries */
float fetchVoxelSDF(ivec3 gridCoord)
{
    /* Out of bounds check */
    if(any(lessThan(gridCoord, ivec3(0))) || any(greaterThanEqual(gridCoord, uBrickGridDim * BRICK_SIZE)))
        return 1.0; /* Safe max distance */

    ivec3 brick = gridCoord / BRICK_SIZE;
    ivec3 voxel = gridCoord % BRICK_SIZE;

    uint stored = texelFetch(uBrickMap, brick, 0).r;
    
    /* Empty brick */
    if(stored == 0u)
        return 1.0; 

    /* Found a brick, calculate exact atlas position */
    uint atlasLinear = stored - 1u;
    uint w = uint(uAtlasBrickDim.x);
    uint h = uint(uAtlasBrickDim.y);

    ivec3 atlasOffset = ivec3(
        int(atlasLinear % w),
        int((atlasLinear / w) % h),
        int(atlasLinear / (w * h))
    ) * BRICK_SIZE;

    /* Fetch exact texel using integer coordinates (No hardware bleeding!) */
    float d = texelFetch(uAtlas, atlasOffset + voxel, 0).r;
    
    /* Decode from [0, 1] to [-1, 1] */
    return d * 2.0 - 1.0;
}

/* 2. Manual Trilinear Interpolation */
float sampleSparseSDF(vec3 worldPos)
{
    vec3 gridPos = (worldPos - uGridStart) / uCellSize;
    
    /* Voxel centers are at +0.5, so we shift by -0.5 to find the 8 surrounding grid corners */
    vec3 p = gridPos - 0.5;
    
    ivec3 i = ivec3(floor(p));
    vec3 f = fract(p);

    /* Fetch the 8 corners of the interpolation cube. 
       This flawlessly crosses brick boundaries because of fetchVoxelSDF! */
    float v000 = fetchVoxelSDF(i + ivec3(0, 0, 0));
    float v100 = fetchVoxelSDF(i + ivec3(1, 0, 0));
    float v010 = fetchVoxelSDF(i + ivec3(0, 1, 0));
    float v110 = fetchVoxelSDF(i + ivec3(1, 1, 0));
    float v001 = fetchVoxelSDF(i + ivec3(0, 0, 1));
    float v101 = fetchVoxelSDF(i + ivec3(1, 0, 1));
    float v011 = fetchVoxelSDF(i + ivec3(0, 1, 1));
    float v111 = fetchVoxelSDF(i + ivec3(1, 1, 1));

    /* Interpolate along X */
    float v00 = mix(v000, v100, f.x);
    float v10 = mix(v010, v110, f.x);
    float v01 = mix(v001, v101, f.x);
    float v11 = mix(v011, v111, f.x);

    /* Interpolate along Y */
    float v0 = mix(v00, v10, f.y);
    float v1 = mix(v01, v11, f.y);

    /* Interpolate along Z */
    float d = mix(v0, v1, f.z);

    return d * uCellDiagonal;
}

float raymarch(vec3 ro, vec3 rd)
{
    float t = 0.0;
    vec3 invRd = 1.0 / rd;

    for(int i = 0; i < 64; i++)
    {
        vec3 p = ro + rd * t;
        vec3 gridPos = (p - uGridStart) / uCellSize;
        ivec3 brickCoord = ivec3(floor(gridPos / float(BRICK_SIZE)));

        /* 1. Bounds Check */
        if(any(lessThan(brickCoord, ivec3(0))) || any(greaterThanEqual(brickCoord, uBrickGridDim))) {
            /* If we are outside, we need to step to the grid entry point or just exit */
            t += 1.0; 
            if(t > 200.0) break;
            continue;
        }

        /* 2. Check Indirection Map */
        uint stored = texelFetch(uBrickMap, brickCoord, 0).r;

        if(stored == 0u) {
            /* EMPTY BRICK: Perform DDA Skip */
            /* Find distance to the next brick boundary along the ray */
            vec3 brickMin = vec3(brickCoord * BRICK_SIZE);
            vec3 brickMax = brickMin + vec3(BRICK_SIZE);
            
            /* Intersection with Axis Aligned Bounding Box (AABB) */
            vec3 t0 = (brickMin - gridPos) * invRd;
            vec3 t1 = (brickMax - gridPos) * invRd;
            vec3 tMax = max(t0, t1);
            
            /* The shortest distance to a boundary */
            float skipDist = min(tMax.x, min(tMax.y, tMax.z));
            
            /* Jump to the boundary + a tiny epsilon to enter the next brick */
            t += (skipDist + 0.01) * uCellSize;
        }
        else {
            /* OCCUPIED BRICK: Normal Sphere Tracing */
            float d = sampleSparseSDF(p);
            if(abs(d) < 0.01) return t;
            t += d;
        }

        if(t > 128.0) break;
    }
    return -1.0;
}

vec3 calc_normal(vec3 p)
{
    // orig: float e = uCellSize * 0.1; 
    float e = uCellSize * 0.1; 
    vec2 k = vec2(1.0, -1.0);
    
    return normalize(
        k.xyy * sampleSparseSDF(p + k.xyy * e) +
        k.yyx * sampleSparseSDF(p + k.yyx * e) +
        k.yxy * sampleSparseSDF(p + k.yxy * e) +
        k.xxx * sampleSparseSDF(p + k.xxx * e)
    );
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

    vec3 mate = vec3(0.18);
    vec3  lig = normalize( vec3(-0.1, 0.3, 0.6) );
    vec3 sun_dir  = normalize(vec3(0.8, 0.4, 0.2));
    float sun_dif = clamp(dot(nor, sun_dir), 0.0, 1.0);
    float sky_dif = clamp(0.5 + 0.5 * dot(nor, vec3(0.0, 1.0, 0.0)), 0.0, 1.0);
    float bou_dif = clamp(0.5 + 0.5 * dot(nor, vec3(0.0, -1.0, 0.0)), 0.0, 1.0);
    col  = mate * vec3(7.0, 4.5, 3.0) * sun_dif;
    col += mate * vec3(0.5, 0.8, 0.9) * sky_dif;
    col += mate * vec3(0.7, 0.3, 0.2) * bou_dif;

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
