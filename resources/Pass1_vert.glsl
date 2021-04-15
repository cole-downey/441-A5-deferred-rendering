#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 MVit;

attribute vec4 aPos;
attribute vec3 aNor;

void main()
{
	gl_Position = P * MV * aPos;
}
