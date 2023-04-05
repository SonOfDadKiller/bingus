#version 420 core

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec4 in_color;

layout (std140, binding = 0) uniform Camera
{
    mat4 projection;
    mat4 view;
};

out vec4 color;

void main()
{
    gl_Position = projection * view * vec4(in_pos, 1);
    color = in_color;
}
