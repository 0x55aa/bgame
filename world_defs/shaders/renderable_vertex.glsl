#version 120

// Pass the matrices in as uniforms
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform vec3 camera_position;

// World position is part of the VBO
attribute vec3 world_position;
attribute vec3 normal;
attribute vec3 color;
attribute vec2 texture_position;
attribute vec3 flags;
attribute vec3 light_position;
attribute vec3 light_color;

// Outputs
varying vec3 tint;
varying vec3 world_pos;
varying vec4 interpolated_position;
varying vec3 flag_out;
varying vec3 light_pos;
varying vec3 light_col;
varying vec3 normal_out;

void main() {
    vec4 position = gl_Vertex;
    position.xyz += world_position.xzy;
    gl_Position = projection_matrix * (view_matrix * position);
    tint = color;
    if (world_position.z < camera_position.z) {
        float darken = (camera_position.z - world_position.z) * 0.05f;
        tint.rgb -= darken;
    }
    gl_TexCoord[0] = vec4(texture_position, 0.0, 0.0);
    world_pos = world_position;
    interpolated_position = position;

    flag_out = flags;
    light_pos = light_position;
    light_col = light_color;
    normal_out = normal;
}
