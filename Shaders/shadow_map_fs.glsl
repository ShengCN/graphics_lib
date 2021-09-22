#version 430

out vec4 frag_color;

void main(){
    // frag_color = vec4(1.0-gl_FragCoord.z*gl_FragCoord.w);
    frag_color = vec4(vec3(gl_FragCoord.w), 1.0);
    // frag_color = vec4(0.5f);
}