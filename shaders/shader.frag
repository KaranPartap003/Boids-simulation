#version 430

out vec4 color;
in vec4 fColor;

void main()
{
	vec3 col = max(fColor.rgb, vec3(0.1));
	color =  vec4(col, 1.0f);
}
