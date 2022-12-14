#version 420 core

in vec2 uv;

uniform sampler2D main_tex;

out vec4 frag_color;

void main()
{
    float tex_sample = texture(main_tex, uv).r;
    frag_color = vec4(tex_sample);// + vec4(0, 0, 0, 0.5);
}
