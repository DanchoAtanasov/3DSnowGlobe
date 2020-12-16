// Basic  fragment shader to add a 2D texture

#version 420

//in vec4 fcolour;
//in vec2 ftexcoord;

// Ins
in vec3 fposition, fnormal, flightdir;
in vec4 fdiffusecolour, fambientcolour;
in vec2 ftexcoord;

// Outs
out vec4 outputColor;

// Uniforms
uniform mat4 model, view, projection;
uniform mat3 normalmatrix;
uniform uint colourmode, emitmode;
uniform vec4 lightpos;

uniform sampler2D tex1;

// Global constants
vec4 specular_albedo = vec4(1.0,1.0,1.0,1.0);
vec4 global_ambient = vec4(0.05, 0.05, 0.05, 1.0);
float shininess = 8.0;

void main()
{
	vec3 N = fnormal;
	vec3 L = flightdir;
	vec3 P = fposition;

	vec4 position_h = vec4(fposition, 1.0);

	// Calculate the diffuse component
	vec4 diffuse = max(dot(N, L), 0.0) * fdiffusecolour;

	// Calculate the specular component using Phong specular reflection
	vec3 V = normalize(-P);	
	vec3 R = reflect(-L, N);
	vec4 specular = pow(max(dot(R, V), 0.0), shininess) * specular_albedo;

	vec3 light_pos3 = lightpos.xyz;
	float distanceToLight = length(light_pos3 - P);	// For attenuation

	float attenuation;
	float attenuation_k1 = 0.4;
	float attenuation_k2 = 0.4;
	float attenuation_k3 = 0.4;
	attenuation = 1.0 / (attenuation_k1 + attenuation_k2*distanceToLight + 
								attenuation_k3 * pow(distanceToLight, 2));

	vec4 emissive = vec4(0); // Create a vec3(0, 0, 0) for our emmissive light
	// If emitmode is 1 then we enable emmissive lighting
	if (emitmode == 1) emissive = vec4(1.0, 1.0, 0.8, 1.0);

	vec4 fcolour = attenuation * (fambientcolour + diffuse + specular) + emissive + global_ambient;

	vec4 texcolour = texture(tex1, ftexcoord);
	outputColor = fcolour * texcolour;
}