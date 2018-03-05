#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 texture_info;
layout (location = 2) in float aNormal;

layout (std140) uniform camera_data
{
	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 proj_view_matrix;
	vec3 camera_position;
};

out VS_OUT {
    vec3 tex_pos;
    vec3 world_pos;
    vec3 base_normal;
    mat3 TBN; // Used to transform tangent space to world space for per-pixel normals
} vs_out;

const vec3 normal_lut[6] = vec3[6](
        vec3(0.0, 1.0, 0.0), // Top
        vec3(0.0f,  0.0f, -1.0f), // Back
        vec3(0.0f,  0.0f, 1.0f), // Front
        vec3(-1.0f,  0.0f,  0.0f), // Left
        vec3(1.0f,  0.0f,  0.0f), // Right
        vec3(0.0f, -1.0f,  0.0f) // Bottom
    );

const vec3 tangent_lut[6] = vec3[6](
        vec3(1.0, 0.0, 0.0), // Top
        vec3(1.0f,  0.0f, 0.0f), // Back
        vec3(1.0f,  0.0f, 0.0f), // Front
        vec3(0.0f,  1.0f,  0.0f), // Left
        vec3(0.0f,  1.0f,  0.0f), // Right
        vec3(1.0f, 0.0f,  0.0f) // Bottom
    );

const vec3 bitangent_lut[6] = vec3[6](
        vec3(0.0, 0.0, 1.0), // Top
        vec3(0.0f,  1.0f, 0.0f), // Back
        vec3(0.0f,  1.0f, 0.0f), // Front
        vec3(0.0f,  0.0f,  1.0f), // Left
        vec3(0.0f,  0.0f,  1.0f), // Right
        vec3(0.0f, 0.0f,  1.0f) // Bottom
    );

const mat3 TBN[6] = mat3[6](
		mat3(vec4(tangent_lut[0], 0.0), vec4(bitangent_lut[0], 0.0), vec4(normal_lut[0], 0.0)),
		mat3(vec4(tangent_lut[1], 0.0), vec4(bitangent_lut[1], 0.0), vec4(normal_lut[1], 0.0)),
		mat3(vec4(tangent_lut[2], 0.0), vec4(bitangent_lut[2], 0.0), vec4(normal_lut[2], 0.0)),
		mat3(vec4(tangent_lut[3], 0.0), vec4(bitangent_lut[3], 0.0), vec4(normal_lut[3], 0.0)),
		mat3(vec4(tangent_lut[4], 0.0), vec4(bitangent_lut[4], 0.0), vec4(normal_lut[4], 0.0)),
		mat3(vec4(tangent_lut[5], 0.0), vec4(bitangent_lut[5], 0.0), vec4(normal_lut[5], 0.0))
	);

void main()
{
    vs_out.tex_pos = texture_info;
    vs_out.world_pos = aPos;
    int normal_idx = int(floor(aNormal));
    vs_out.base_normal = normal_lut[normal_idx];
    vs_out.TBN = TBN[normal_idx];
    gl_Position = projection_matrix * view_matrix * vec4(aPos, 1.0);
}
