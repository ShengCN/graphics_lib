#version 430

in vec3 vs_pos;
in vec3 vs_color;
in vec3 vs_norm;
in vec2 vs_uvs;
in vec4 vs_light_space_pos;

uniform float slider;
uniform int cur_selected_id;
uniform int is_draw_normal;
uniform int draw_shadow;
uniform sampler2D shadow_map;

out vec4 frag_color;

float lerp(float t1, float t2, float fract){
    return (1.0 - fract) * t1 + fract * t2;
}

float shadow(vec4 light_space_pos) {
    vec3 projected = light_space_pos.xyz/light_space_pos.w;
    
    projected = projected * 0.5 + 0.5;
    float lit = 1.0;
    if(projected.x >= 0.0 && 
        projected.x <= 1.0 && 
        projected.y >=0.0  && 
        projected.y <= 1.0) {
        float closest = texture(shadow_map, projected.xy).r;
        float cur_depth = projected.z;

        float bias = 0.000001;
        //lit = cur_depth - closest > bias  ? 0.0 : 1.0;
        lit = cur_depth > closest  ? 0.0 : 1.0;
    }
    return lit;
}

void main(){
    float z1 = -100.0, z2 = 100.0;
    float z = lerp(z1, z2, slider);

    vec3 light = vec3(0.0, 100.0, 80.0);
    float ka = 0.3;
    float kd = clamp(dot(normalize(light - vs_pos),vs_norm), 0.0f, 1.0f);

    float shadow_eff = shadow(vs_light_space_pos);
    if (draw_shadow == 0) {
        shadow_eff = 1.0;
    }

    vec3 col = (ka + (1.0 - ka) * kd) * vs_color * shadow_eff;
    if(is_draw_normal > 0){
        col = vs_norm;
    }

    frag_color = vec4(col, 1.0);
}
