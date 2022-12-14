#version 420 core

in vec2 uv;

uniform vec4 color;
uniform sampler2D main_tex;

out vec4 frag_color;

void main()
{
    vec4 tex_sample = texture(main_tex, uv);
    frag_color = tex_sample * color;// + vec4(0.5, 0, 0, 0.5);
}
