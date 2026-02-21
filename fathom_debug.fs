#version 330 core

out vec4 FragColor;

uniform vec3  iResolution;
uniform float iTime;
uniform usampler3D uBrickMap;
uniform vec3  uGridStart;
uniform float uCellSize;
uniform ivec3 uBrickGridDim;

const int BRICK_SIZE = 8;

vec3 debugColor(uint idx) {
    if (idx == 0u) return vec3(0.05);
    // Unique color per brick index
    float hue = float(idx) * 0.123456;
    return 0.5 + 0.4 * cos(6.28318 * (hue + vec3(0.0, 0.33, 0.67)));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
    
    float camDist = 4.0;
    vec3 ro = vec3(camDist * sin(iTime * 0.2), 2.0, camDist * cos(iTime * 0.2));
    vec3 ta = vec3(0.0, 0.0, 0.0);
    vec3 ww = normalize(ta - ro);
    vec3 uu = normalize(cross(ww, vec3(0, 1, 0)));
    vec3 vv = normalize(cross(uu, ww));
    vec3 rd = normalize(uv.x * uu + uv.y * vv + 1.5 * ww);

    vec3 gridMin = uGridStart;
    vec3 gridMax = uGridStart + vec3(uBrickGridDim * BRICK_SIZE) * uCellSize;
    
    vec3 t0 = (gridMin - ro) / rd;
    vec3 t1 = (gridMax - ro) / rd;
    vec3 tmin_v = min(t0, t1);
    vec3 tmax_v = max(t0, t1);
    
    float t_entry = max(max(tmin_v.x, tmin_v.y), tmin_v.z);
    float t_exit  = min(min(tmax_v.x, tmax_v.y), tmax_v.z);

    vec3 col = vec3(0.02); // Background color

    if (t_entry < t_exit && t_exit > 0.0) {
        float t = max(0.0, t_entry);
        vec3 invRd = 1.0 / rd;
        
        for(int i = 0; i < 48; i++) {
            vec3 p = ro + rd * (t + 0.0001);
            vec3 pGrid = (p - uGridStart) / uCellSize;
            ivec3 brickCoord = ivec3(floor(pGrid / float(BRICK_SIZE)));

            if(any(lessThan(brickCoord, ivec3(0))) || any(greaterThanEqual(brickCoord, uBrickGridDim))) break;

            uint stored = texelFetch(uBrickMap, brickCoord, 0).r;

            // Math to find the distance to the exit of the CURRENT brick
            vec3 bMin = vec3(brickCoord * BRICK_SIZE);
            vec3 bMax = bMin + vec3(BRICK_SIZE);
            vec3 tM = max((bMin - pGrid) * invRd, (bMax - pGrid) * invRd);
            float dt_brick = min(tM.x, min(tM.y, tM.z)) * uCellSize;

            if (stored > 0u) {
                vec3 deltaDist = abs(1.0 / rd);
                ivec3 step = ivec3(sign(rd));
                ivec3 voxelCoord = ivec3(floor(pGrid));
                vec3 sideDist = (vec3(voxelCoord) + max(vec3(0.0), sign(rd)) - pGrid) * (1.0/rd);
                
                // Color for this specific brick
                vec3 brickCol = debugColor(stored);
                vec3 n = step * step;
                
                // Voxel edges logic
                vec3 localP = fract(pGrid);
                float edge = min(localP.x, min(localP.y, localP.z));
                float voxelWire = smoothstep(0.1, 0.0, edge);
                
                // Brick edges logic
                vec3 brickP = fract(pGrid / float(BRICK_SIZE));
                float bEdge = min(brickP.x, min(brickP.y, brickP.z));
                float brickWire = smoothstep(0.02, 0.0, bEdge);

                col = mix(brickCol, vec4(0,0,0,1).rgb, voxelWire * 0.5); // Black voxel lines
                col = mix(col, vec3(1.0), brickWire);                    // White brick lines
                
                // Simple shading
                col *= (0.8 + 0.2 * dot(rd, -rd)); 
                break; 
            }
            
            t += dt_brick + 0.001;
            if(t > t_exit) break;
        }
    }
    fragColor = vec4(pow(col, vec3(0.4545)), 1.0);
}

void main() { mainImage(FragColor, gl_FragCoord.xy); }