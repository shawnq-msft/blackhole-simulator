#version 430

// Vertex attributes
in vec3 in_position;
in vec2 in_texcoord_0;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Outputs to fragment shader
out vec2 uv;
out vec3 world_position;

void main() {
    // Transform vertex position to clip space
    gl_Position = projection * view * model * vec4(in_position, 1.0);
    
    // Pass texture coordinates to fragment shader
    uv = in_texcoord_0;
    
    // Pass world position to fragment shader
    world_position = (model * vec4(in_position, 1.0)).xyz;
}