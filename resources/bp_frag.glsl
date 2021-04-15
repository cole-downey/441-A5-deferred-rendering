#version 120

uniform vec3 lightPos[10];
uniform vec3 lightColors[10];
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 vertPos; 
varying vec3 vertNor;

void main()
{
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

	gl_FragColor = vec4(fragColor, 1.0);
}
