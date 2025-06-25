#version 430

// Inputs from vertex shader
in vec2 uv;

// Uniforms
uniform float time;
uniform vec2 resolution;
uniform float inner_radius;
uniform float outer_radius;
uniform float spin;

// Output
out vec4 fragColor;

// Constants
const float PI = 3.14159265359;

// Noise functions
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    // Smoothstep interpolation
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    // Sample 4 corners
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    // Bilinear interpolation
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// Fractal Brownian Motion
float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    // Add multiple noise octaves
    for (int i = 0; i < 6; i++) {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

// Temperature to RGB conversion (blackbody radiation)
vec3 temperature_to_rgb(float temperature) {
    // Map temperature from 0-1 to 1000-10000K
    float temp = 1000.0 + temperature * 9000.0;
    
    // Approximate blackbody radiation colors
    vec3 color;
    
    if (temp < 1500.0) {
        // Deep red
        color = vec3(1.0, 0.0, 0.0);
    } else if (temp < 3000.0) {
        // Red to orange
        float t = (temp - 1500.0) / 1500.0;
        color = mix(vec3(1.0, 0.0, 0.0), vec3(1.0, 0.5, 0.0), t);
    } else if (temp < 6000.0) {
        // Orange to yellow to white
        float t = (temp - 3000.0) / 3000.0;
        color = mix(vec3(1.0, 0.5, 0.0), vec3(1.0, 1.0, 0.8), t);
    } else {
        // White to blue-white
        float t = (temp - 6000.0) / 4000.0;
        color = mix(vec3(1.0, 1.0, 0.8), vec3(0.8, 0.9, 1.0), t);
    }
    
    return color;
}

void main() {
    // Convert UV to polar coordinates
    vec2 centered_uv = uv * 2.0 - 1.0;
    float radius = length(centered_uv);
    float angle = atan(centered_uv.y, centered_uv.x);
    
    // Check if within disk bounds
    if (radius < inner_radius || radius > outer_radius) {
        discard;
    }
    
    // Normalize radius to 0-1 range
    float normalized_radius = (radius - inner_radius) / (outer_radius - inner_radius);
    
    // Calculate temperature based on radius (hotter near center)
    float temperature = pow(1.0 - normalized_radius, 0.75);
    
    // Calculate orbital velocity (Keplerian)
    float orbital_velocity = sqrt(1.0 / radius);
    
    // Rotate based on orbital velocity and time
    float rotated_angle = angle + time * orbital_velocity * 0.1;
    
    // Generate turbulent patterns
    vec2 noise_coord = vec2(
        normalized_radius * 10.0,
        rotated_angle * 5.0
    );
    float turbulence = fbm(noise_coord + vec2(0.0, time * 0.1));
    
    // Add spiral arms
    float spiral = 0.5 + 0.5 * sin(rotated_angle * 3.0 + normalized_radius * 20.0 - time * 0.2);
    
    // Combine patterns
    float pattern = mix(turbulence, spiral, 0.5);
    
    // Apply relativistic effects
    // Doppler shift: approaching side is blueshifted, receding side is redshifted
    float doppler = 0.2 * sin(angle);
    temperature += doppler;
    
    // Calculate color based on temperature
    vec3 color = temperature_to_rgb(temperature);
    
    // Modulate brightness with pattern
    color *= 0.5 + 0.8 * pattern;
    
    // Add glow
    float glow = 0.1 / (normalized_radius + 0.05);
    color += vec3(1.0, 0.6, 0.3) * glow * 0.2;
    
    // Output final color with alpha based on pattern
    float alpha = pattern * (1.0 - normalized_radius);
    fragColor = vec4(color, alpha);
}