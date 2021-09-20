#version 430

in vec3 vs_pos;
in vec3 vs_color;
in vec3 vs_norm;
in vec2 vs_uvs;

uniform float slider;
uniform int cur_selected_id;
uniform sampler2D map_kd;
out vec4 frag_color;

float lerp(float t1, float t2, float fract){
    return (1.0 - fract) * t1 + fract * t2;
}

void main(){
    vec3 col = vec3(1.0f);
    frag_color = vec4(col, 1.0);
}