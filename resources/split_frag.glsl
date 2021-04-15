#version 120
    
uniform sampler2D posTexture;
uniform sampler2D norTexture;
uniform sampler2D kaTexture;
uniform sampler2D kdTexture;
uniform vec2 windowSize;
uniform int activeTex;

void main()
{
    vec2 tex;
    tex.x = gl_FragCoord.x/windowSize.x;
    tex.y = gl_FragCoord.y/windowSize.y;
    
    vec3 vertPos = texture2D(posTexture, tex).rgb;
    vec3 vertNor = texture2D(norTexture, tex).rgb;
    vec3 ka = texture2D(kaTexture, tex).rgb;
    vec3 kd = texture2D(kdTexture, tex).rgb;

    if (activeTex == 1) {
        gl_FragColor.rgb = vertPos;
    } else if (activeTex == 2) {
        gl_FragColor.rgb = vertNor;
    } else if (activeTex == 3) {
        gl_FragColor.rgb = ka;
    } else if (activeTex == 4) {
        gl_FragColor.rgb = kd;
    }
}