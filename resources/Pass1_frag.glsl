#version 120
    
varying vec3 vertPos; // in camera space
varying vec3 vertNor; // in camera space
    
uniform vec3 ka;
uniform vec3 kd;
    
void main()
{
    gl_FragData[0].xyz = vertPos;
    gl_FragData[1].xyz = vertNor;
    gl_FragData[2].xyz = ka;
    gl_FragData[3].xyz = kd;
}