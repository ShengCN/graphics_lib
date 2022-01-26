#version 430

in vec4 vs_light_space_pos;
out vec4 frag_color;

uniform sampler2D shadow_map;

void main(){
    vec3 projected = vs_light_space_pos.xyz/vs_light_space_pos.w;
    projected = projected * 0.5 + 0.5;
    float lit = 1.0;
    if(projected.x >= 0.0 && projected.x <= 1.0 && projected.y >=0.0 && projected.y <= 1.0) {
        float z = texture(shadow_map, projected.xy).r;
        float r = projected.z;

        if(r<=z) {
            lit = 1.0f;
        } else  {
            lit = 0.0f;
        }
    }

    vec3 col = vec3(lit);
    frag_color = vec4(col, 1.0);
}