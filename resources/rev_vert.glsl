#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 IT;
uniform float t;

attribute vec4 aPos; // In object space

varying vec3 vertPos; // In camera space
varying vec3 vertNor; // In camera space

void main()
{	
	float x = aPos.x;
	float theta = aPos.y;
	float fx = cos(x + t) + 2;
	vec4 pos = vec4(x / 10, fx * cos(theta) / 10, fx * sin(theta) / 10, 1.0);
	float dfx = -sin(x + t);
	vec3 norm = cross(vec3(1.0f, dfx * cos(theta), dfx * sin(theta)), 
		vec3(0.0f, -fx * sin(theta), fx * cos(theta)));
	norm = normalize(norm);

	gl_Position = P * (MV * pos);
	vertPos = vec3(MV * pos);
	vertNor = vec3(IT * vec4(norm, 1.0));
}
