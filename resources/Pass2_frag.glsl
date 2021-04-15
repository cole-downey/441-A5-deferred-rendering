#version 120
    
uniform sampler2D posTexture;
uniform sampler2D norTexture;
uniform sampler2D kaTexture;
uniform sampler2D kdTexture;
uniform vec2 windowSize;

uniform vec3 lightPos[10];
uniform vec3 lightColors[10];
    
void main()
{
    vec2 tex;
    tex.x = gl_FragCoord.x/windowSize.x;
    tex.y = gl_FragCoord.y/windowSize.y;
    
    vec3 vertPos = texture2D(posTexture, tex).rgb;
    vec3 vertNor = texture2D(norTexture, tex).rgb;
    vec3 ka = texture2D(kaTexture, tex).rgb;
    vec3 kd = texture2D(kdTexture, tex).rgb;
    vec3 ks = vec3(1.0f);
    float s = 10.0f;
    if (kd == vec3(0.0)) {
        s = 1.0f;
    } else if (kd == vec3(1.0)) {
        s = 200.0f;
    }

    vec3 n = normalize(vertNor);
    vec3 eye = normalize(vec3(0) - vertPos);

    vec3 fragColor = ka;

    for (int i = 0; i < 10; i++) {
        vec3 l = normalize(lightPos[i] - vertPos);
        vec3 h = normalize(l + eye);

        vec3 cd = kd * max(0, dot(l, n));
        vec3 cs = ks * pow(max(0, dot(h, n)), s);

        float r = distance(vertPos, lightPos[i]);
        float a = 1.0f / (1.0f + 0.0429f * r + 0.9857f * pow(r, 2));

        vec3 color = vec3(0.0f);
        color.r = lightColors[i].r * (cd.r + cs.r);
        color.g = lightColors[i].g * (cd.g + cs.g);
        color.b = lightColors[i].b * (cd.b + cs.b);
        fragColor += color * a;
    }
    gl_FragColor.rgb = fragColor;
}