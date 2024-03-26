#version 430

out vec4 color;
in float intensity;

void main()
{
	color =  mix(vec4(0.0f, 0.2f, 1.0f, 1.0f), vec4(0.6f, 0.05f, 0.3f, 1.0f), intensity);
}