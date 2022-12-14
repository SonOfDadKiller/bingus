#version 420 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec2 in_uv;

layout (std140, binding = 0) uniform Camera
{
    mat4 projection;
    mat4 view;
};

out vec2 uv;

void main()
{
    gl_Position = projection * view * vec4(in_pos, 1);
    uv = in_uv;
}
