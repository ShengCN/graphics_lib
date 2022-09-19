#version 410

layout(location = 0) in vec3 pos_attr;
layout(location = 1) in vec2 tex_coord_attrib;
layout(location = 2) in vec3 normal_attrib;

uniform mat4 PVM;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec2 tex_coord;
out vec3 normal;

void main(){
    gl_Position = vec4(pos_attr, 1.0);
    tex_coord = 0.5*pos_attr.xy + vec2(0.5);
}