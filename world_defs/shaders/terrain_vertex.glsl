#version 120

// Pass the matrices in as uniforms
uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform vec3 camera_position;

// World position is part of the VBO
attribute vec3 in_position;
attribute vec3 world_position;
attribute vec4 rotation;
attribute vec3 color;
attribute vec2 texture_position;
attribute vec3 flags;
attribute vec3 light_position;
attribute vec3 light_color;
attribute vec2 normal_position;
attribute vec2 specular_position;
attribute vec2 displacement_position;

// Outputs
varying vec3 tint;
varying vec3 world_pos;
varying mat3 TBN;
varying vec4 interpolated_position;
varying vec3 flag_out;
varying vec3 light_pos;
varying vec3 light_col;

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

mat4 translate(float x, float y, float z){
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(x,   y,   z,   1.0)
    );
}

void main() {
    // Transform the vertex to world-space
    mat4 rotation_matrix = rotationMatrix(rotation.xyz, rotation.w);
    mat4 translation_matrix = translate(world_position.x, world_position.z, world_position.y); // Note we're converting from world to GL co-ordinates
    vec4 position = vec4(in_position, 1.0);
    mat4 model_view_matrix = view_matrix * translation_matrix * rotation_matrix;
    gl_Position = projection_matrix * model_view_matrix * position;
    vec4 normal_v4 = vec4(0.0, 1.0, 0.0, 1.0) * rotation_matrix;
    vec3 normal = normal_v4.xyz;

    // Parallax
    vec3 tangent;
    vec3 bitangent;

    vec3 c1 = cross(normal, vec3(0.0, 0.0, 1.0));
    vec3 c2 = cross(normal, vec3(0.0, 1.0, 0.0));

    tangent = length(c1) > length(c2) ? c1 : c2;
    tangent = normalize(tangent);
    bitangent = normalize(cross(normal, tangent));

    vec3 T = tangent;
    vec3 B = bitangent;
    vec3 N = normalize(normal);
    TBN = mat3(T, B, N);

    tint = color;
    float darken = (camera_position.y - world_position.z) * 0.05;
    tint.r -= darken;
    tint.g -= darken;
    tint.b -= darken;
    gl_TexCoord[0] = vec4(texture_position, 0.0, 0.0);
    world_pos = world_position;
    interpolated_position = translation_matrix * position;

    flag_out = flags;
    light_pos = light_position;
    light_col = light_color;
}
