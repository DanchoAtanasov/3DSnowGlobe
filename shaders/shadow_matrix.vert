#version 420 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec3 normal;

out vec4 fcolour;		// Output from vertex shader

uniform mat4 model, view, projection;		// Model transformation matrix

void main()
{
	vec4 position_h = vec4(position, 1.0);
	fcolour = vec4(0.1, 0.1, 0.2, 1.0);		
	gl_Position = projection * view * model * position_h;
}