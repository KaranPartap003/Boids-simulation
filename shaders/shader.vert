#version 430

layout (location = 0) in vec4 pos;
out float intensity;

uniform mat4 mvp;

void main()
{
	gl_Position =  mvp * pos; 
	intensity = pos.w;
}
