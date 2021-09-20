#version 430

layout(location=0) in vec3 pos_attr;

out vec3 vs_pos;  // world space position
void main(){
    vs_pos = pos_attr;
}