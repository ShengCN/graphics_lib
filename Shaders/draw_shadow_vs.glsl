#version 430

layout(location=0) in vec3 pos_attr;

uniform mat4 PVM;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform mat4 light_pv;

out vec4 vs_light_space_pos; // light space position

void main(){
    gl_Position = PVM * vec4(pos_attr,1.0f);

    vs_light_space_pos = light_pv * vec4(pos_attr,1.0f);
}
