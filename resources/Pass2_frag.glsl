#version 120
    
uniform sampler2D posTexture;
uniform sampler2D norTexture;
uniform sampler2D kaTexture;
uniform sampler2D kdTexture;
uniform vec2 windowSize;
    
void main()
{
    vec2 tex;
    tex.x = gl_FragCoord.x/windowSize.x;
    tex.y = gl_FragCoord.y/windowSize.y;
    
    vec3 pos = texture2D(posTexture, tex).rgb;
    vec3 nor = texture2D(norTexture, tex).rgb;
    vec3 ka = texture2D(kaTexture, tex).rgb;
    vec3 kd = texture2D(kdTexture, tex).rgb;
    
    gl_FragColor.rgb = ka;
}