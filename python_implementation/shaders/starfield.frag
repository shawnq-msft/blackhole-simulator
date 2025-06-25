#version 430

// Inputs from vertex shader
in vec2 uv;

// Uniforms
uniform float time;
uniform vec2 resolution;

// Output
out vec4 fragColor;

// Constants
const float PI = 3.14159265359;

// Hash function for pseudo-random numbers
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// Star field generation
vec3 starfield(vec2 uv) {
    // Scale UV coordinates
    uv *= 100.0;
    
    // Initialize color
    vec3 color = vec3(0.0);
    
    // Grid-based stars
    vec2 grid = floor(uv);
    vec2 gv = fract(uv) - 0.5;
    
    // For each grid cell
    for (float y = -1.0; y <= 1.0; y++) {
        for (float x = -1.0; x <= 1.0; x++) {
            // Neighbor grid cell
            vec2 offset = vec2(x, y);
            
            // Random position within the cell
            vec2 id = grid + offset;
            float n = hash(id);
            vec2 rp = offset + vec2(n, fract(n * 34.0)) - gv;
            
            // Star intensity based on distance
            float star = 0.01 / length(rp);
            
            // Random star color
            float hue = hash(id + 0.241);
            vec3 star_color = 0.5 + 0.5 * cos(vec3(0.0, 1.0, 2.0) + hue * 2.0 * PI);
            
            // Twinkle effect
            float twinkle = sin(time * (n * 5.0) + n * PI * 2.0) * 0.5 + 0.5;
            star *= 0.5 + 0.5 * twinkle;
            
            // Add star to color
            color += star * star_color;
        }
    }
    
    // Add subtle blue nebula
    float nebula = max(0.0, hash(uv * 0.01) - 0.95) * 20.0;
    color += vec3(0.0, 0.1, 0.2) * nebula;
    
    return color;
}

void main() {
    // Calculate UV coordinates centered at (0,0)
    vec2 uv = uv * 2.0 - 1.0;
    
    // Correct aspect ratio
    uv.x *= resolution.x / resolution.y;
    
    // Generate star field
    vec3 color = starfield(uv);
    
    // Output final color
    fragColor = vec4(color, 1.0);
}