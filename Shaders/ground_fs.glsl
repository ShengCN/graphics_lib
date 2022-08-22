#version 330

in vec3 vs_pos;
in vec3 vs_color;
in vec3 vs_norm;
in vec2 vs_uvs;

out vec4 frag_color;

void main(){
    float frequency = 50.0;
    float x = sin(vs_pos.x * frequency), y = sin(vs_pos.z * frequency);
    float width = 0.98;
    float ori_z = gl_FragCoord.z / gl_FragCoord.w;
    float fadding = 1.0/exp(ori_z * 0.5); 
    vec3 col = vec3(1.0-clamp(step(width, x) + step(width, y), 0.0, 1.0) * fadding);
    frag_color = vec4(col, 1.0);
}
