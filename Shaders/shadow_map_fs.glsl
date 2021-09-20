#version 430

out vec4 frag_color;

void main(){
    frag_color = vec4(1.0-gl_FragCoord.z*gl_FragCoord.w);
}