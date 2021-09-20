#version 430

in vec4 vs_light_space_pos;

uniform int pass;
uniform sampler2D shadow_map;
uniform vec3 light_pos;
out vec4 frag_color;

float lerp(float t1, float t2, float fract){
    return (1.0 - fract) * t1 + fract * t2;
}
float near = 0.1;
float far = 3000.0f;

float LinearizeDepth(float depth) 
{
    
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main(){
    if(pass==1) {
        frag_color = vec4(1.0-gl_FragCoord.z*gl_FragCoord.w);
        // frag_color = vec4(0.0f);
        // frag_color = vec4(1.0);
        frag_color = vec4(LinearizeDepth(gl_FragCoord.w));
        return;
    }
    
    
    vec3 projected = vs_light_space_pos.xyz/vs_light_space_pos.w;
    projected = projected * 0.5 + 0.5;
    float lit = 1.0;
    if(projected.x >= 0.0 && projected.x <= 1.0 && projected.y >=0.0 && projected.y <= 1.0) {
        float z = texture(shadow_map, projected.xy).r;
        float r = projected.z;

        if(r<=z) {
            lit = 1.0f;
        } else  {
            lit = 0.0f;
        }
    }

    vec3 col = vec3(lit);
    // float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    
    // col = vec3(lit);
    
    // frag_color = vec4(1.0);
    frag_color = vec4(col, 1.0);
}