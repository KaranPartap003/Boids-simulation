#version 430

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 InColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;

out vec4 fColor;

void main()
{
	gl_Position =  perspective * view * model * pos; 
	fColor = InColor;
}
