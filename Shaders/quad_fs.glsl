#version 410

in vec2 tex_coord;
in vec3 normal;

uniform float slider;
uniform int cur_selected_id;
uniform sampler2D accum_tex;
uniform sampler2D weight_tex;

layout(location = 0) out vec4 frag_color;

void main(){
    vec4 accum = texture(accum_tex, tex_coord);
    float reveal = texture(weight_tex, tex_coord).r;

    frag_color = vec4(accum.rgb/max(accum.a, 1e-5), reveal);
    // frag_color = vec4(vec3(reveal), 0.5);
}
