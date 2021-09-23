#version 430

in vec4 vs_light_space_pos;
out vec4 frag_color;

uniform sampler2D shadow_map;

void main(){
    // vec3 col = vec3(lit);
    vec3 col = vec3(0.5f);
    frag_color = vec4(col, 1.0);
}