#version 430

in vec3 vs_pos;
in vec3 vs_color;
in vec3 vs_norm;
in vec2 vs_uvs;

out vec4 frag_color;

void main(){
    // float z1 = -100.0, z2 = 100.0;
    // float z = lerp(z1, z2, slider);

    // vec3 light = vec3(0.0, 100.0, 80.0);
    // float ka = 0.3;
    // float kd = clamp(dot(normalize(light - vs_pos),vs_norm), 0.0f, 1.0f);
    // // vec3 col_kd = texture(map_kd, vs_uvs).rgb;
    // // if(length(col_kd) < 0.1f)
    // //     col_kd = vs_color;
    // vec3 col = (ka + (1.0 - ka) * kd) * vs_color;
    float frequency = 50.0;
    float x = sin(vs_pos.x * frequency), y = sin(vs_pos.z * frequency);
    float width = 0.9;
    vec3 col = vec3(1.0-clamp(step(width, x) + step(width, y), 0.0, 1.0));
    frag_color = vec4(col, 0.3);
}