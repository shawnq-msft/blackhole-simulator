#version 430

// Inputs from vertex shader
in vec2 uv;
in vec3 world_position;

// Uniforms
uniform vec3 camera_position;
uniform vec3 black_hole_position;
uniform float black_hole_radius;
uniform float schwarzschild_radius;
uniform float spin;
uniform sampler2D accretion_disk_texture;
uniform sampler2D starfield_texture;
uniform float time;

// Output
out vec4 fragColor;

// Constants
const float PI = 3.14159265359;
const float LIGHT_SPEED = 299792458.0;
const int MAX_STEPS = 100;
const float STEP_SIZE = 0.1;
const float EPSILON = 0.001;

// Ray structure
struct Ray {
    vec3 origin;
    vec3 direction;
};

// Function to calculate gravitational deflection
vec3 calculate_deflection(Ray ray, vec3 rel_position) {
    // Calculate impact parameter
    float dot_product = dot(rel_position, ray.direction);
    vec3 closest_point = rel_position - dot_product * ray.direction;
    float impact_parameter = length(closest_point);
    
    // Calculate deflection angle (simplified)
    float deflection_angle = 4.0 * schwarzschild_radius / impact_parameter;
    
    // Calculate deflection vector
    vec3 deflection_vector = normalize(cross(ray.direction, cross(rel_position, ray.direction)));
    
    // Apply deflection based on angle and impact parameter
    float deflection_mask = float(impact_parameter > 1.5 * schwarzschild_radius);
    float deflection_factor = deflection_angle * deflection_mask;
    
    // Calculate new direction
    vec3 new_direction = normalize(ray.direction + deflection_vector * deflection_factor);
    
    return new_direction;
}

// Function to trace a ray through curved spacetime
bool trace_ray(inout Ray ray, out vec3 hit_position) {
    // Initialize ray
    ray.origin = camera_position;
    
    // Current state
    vec3 position = ray.origin;
    vec3 direction = ray.direction;
    
    // Ray tracing loop
    for (int step = 0; step < MAX_STEPS; step++) {
        // Calculate distance to black hole center
        vec3 rel_position = position - black_hole_position;
        float distance = length(rel_position);
        
        // Check for intersection with event horizon
        if (distance < schwarzschild_radius * 1.1) {
            hit_position = position;
            return true;
        }
        
        // Calculate deflection
        direction = calculate_deflection(Ray(position, direction), rel_position);
        
        // Update position
        position += direction * STEP_SIZE;
    }
    
    // No intersection found
    hit_position = position;
    return false;
}

// Function to calculate color from accretion disk
vec4 sample_accretion_disk(vec3 position) {
    // Calculate position relative to black hole
    vec3 rel_position = position - black_hole_position;
    
    // Project onto xy-plane
    vec2 disk_position = rel_position.xy;
    float radius = length(disk_position);
    
    // Check if within accretion disk bounds
    float inner_radius = 3.0 * schwarzschild_radius;
    float outer_radius = 20.0 * schwarzschild_radius;
    
    if (radius < inner_radius || radius > outer_radius) {
        return vec4(0.0);
    }
    
    // Calculate angle
    float angle = atan(disk_position.y, disk_position.x);
    
    // Calculate UV coordinates
    float u = (radius - inner_radius) / (outer_radius - inner_radius);
    float v = (angle + PI) / (2.0 * PI);
    
    // Add time-based rotation for disk movement
    v = fract(v + time * 0.01 * (1.0 - 0.5 * u)); // Keplerian rotation (faster near center)
    
    // Sample texture
    vec4 color = texture(accretion_disk_texture, vec2(u, v));
    
    // Apply temperature gradient (hotter near center)
    float temperature = 1.0 - u;
    vec3 hot_color = mix(vec3(1.0, 0.6, 0.1), vec3(0.1, 0.4, 1.0), u);
    
    // Apply relativistic beaming and Doppler shift
    // Simplified model: brighter on approaching side, dimmer on receding side
    float doppler_factor = 1.0 + 0.5 * sin(angle);
    
    // Combine effects
    color.rgb = mix(color.rgb, hot_color, 0.7) * doppler_factor;
    
    // Apply distance falloff
    float falloff = 1.0 - smoothstep(0.7, 1.0, u);
    color.a *= falloff;
    
    return color;
}

// Function to sample starfield background
vec4 sample_starfield(vec3 direction) {
    // Convert direction to spherical coordinates
    float theta = acos(direction.y);
    float phi = atan(direction.z, direction.x);
    
    // Calculate UV coordinates
    float u = (phi + PI) / (2.0 * PI);
    float v = theta / PI;
    
    // Sample texture
    return texture(starfield_texture, vec2(u, v));
}

void main() {
    // Calculate ray direction from camera to current fragment
    vec3 ray_direction = normalize(world_position - camera_position);
    
    // Initialize ray
    Ray ray;
    ray.origin = camera_position;
    ray.direction = ray_direction;
    
    // Trace ray
    vec3 hit_position;
    bool intersected = trace_ray(ray, hit_position);
    
    // Initialize color
    vec4 color = vec4(0.0);
    
    if (intersected) {
        // Ray hit the event horizon - render as black
        color = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        // Ray didn't hit the black hole
        
        // Sample accretion disk
        vec4 disk_color = sample_accretion_disk(hit_position);
        
        // Sample starfield background
        vec4 star_color = sample_starfield(ray.direction);
        
        // Blend colors
        color = mix(star_color, disk_color, disk_color.a);
    }
    
    // Output final color
    fragColor = color;
}