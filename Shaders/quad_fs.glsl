#version 430

in vec2 tex_coord;
uniform sampler2D img_tex;
out vec4 frag_color;

void main(){
    // vec2 uv = (vec2(vs_pos) + 1.0 ) * 0.5; 
    
    frag_color =texture(img_tex, tex_coord);
    // frag_color = vec4(1.0,0.0,0.0,1.0);
}