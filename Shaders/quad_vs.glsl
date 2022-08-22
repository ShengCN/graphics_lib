#version 330

layout(location=0) in vec3 pos_attr;

out vec2 tex_coord; 

void main(){
    gl_Position = vec4(pos_attr, 1.0);
    tex_coord = 0.5*pos_attr.xy + vec2(0.5);
}
