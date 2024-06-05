#version 430

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 InColor;

uniform mat4 mvp;

out vec4 fColor;

void main()
{
	gl_Position =  mvp * pos; 
	fColor = InColor;
}
