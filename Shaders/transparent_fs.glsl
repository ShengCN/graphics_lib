#version 330

in vec3 vs_pos;
in vec3 vs_color;
in vec3 vs_norm;
in vec2 vs_uvs;

uniform float slider;
uniform int cur_selected_id;
uniform sampler2D map_kd;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out float reveal;

void main(){
    vec4 color = vec4(vs_color, 0.05);

    float weight = 
      max(min(1.0, max(max(color.r, color.g), color.b) * color.a), color.a) *
      clamp(0.03 / (1e-5 + pow(gl_FragCoord.z / 200, 4.0)), 1e-2, 3e3);
   
    frag_color = vec4(color.rgb * color.a, color.a) * weight;
    reveal = color.a;
}
