// Minimal fragment shader
// Iain Martin 2018

#version 400

in vec4 fcolour;
in vec2 ftexcoord;
out vec4 outputColor;

uniform float alphaValue;

uniform sampler2D tex1;

void main()
{
	vec4 texcolour = texture(tex1, ftexcoord);
	//outputColor = fcolour * texcolour;
	outputColor = fcolour;
	outputColor.a = alphaValue;
	//outputColor = vec4(1,1,1,1);
	//outputColor = texcolour;
}