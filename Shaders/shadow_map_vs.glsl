#version 330

layout(location=0) in vec3 pos_attr;

uniform mat4 PVM;

void main(){
    gl_Position = PVM * vec4(pos_attr,1.0f);
}
