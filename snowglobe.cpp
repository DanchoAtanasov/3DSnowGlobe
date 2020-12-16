/**
 * Main class for snowglobe project
 * 
 * Dancho Atanasov 2020
 * 
 */


/* Link to static libraries, could define these as linker inputs in the project settings instead
if you prefer */
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glloadD.lib")
#pragma comment(lib, "opengl32.lib")

/* Include the header to the GLFW wrapper class which
   also includes the OpenGL extension initialisation*/
#include "wrapper_glfw.h"
#include <iostream>

/* Include GLM core and matrix extensions*/
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>

// Include our sphere and object loader classes
#include "tiny_loader.h"
#include "sphere_tex.h"

/* Include the image loader */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "points.h"

class Shader {
public:
	GLuint shaderID;
	GLuint modelID;
	GLuint colourmodeID;
	GLuint viewID;
	GLuint projectionID;
	GLuint alphaValueID;
	GLuint point_sizeID;
	GLuint lightposID;
	GLuint normalmatrixID;
	//GLuint tex_matrixID;

	Shader() {
	
	}

	Shader(GLuint shaderID) {
		printf("Creating shader object\n");
		this->shaderID = shaderID;
		this->modelID = glGetUniformLocation(shaderID, "model");
		this->colourmodeID = glGetUniformLocation(shaderID, "colourmode");
		this->viewID = glGetUniformLocation(shaderID, "view");
		this->projectionID = glGetUniformLocation(shaderID, "projection");
		this->alphaValueID = glGetUniformLocation(shaderID, "alphaValue");

		this->point_sizeID = glGetUniformLocation(shaderID, "size");

		this->lightposID = glGetUniformLocation(shaderID, "lightpos");
		this->normalmatrixID = glGetUniformLocation(shaderID, "normalmatrix");

		//this->tex_matrixID = glGetUniformLocation(shaderID, "tex_matrix");

		GLuint loc = glGetUniformLocation(shaderID, "tex1");
		if (loc >= 0) glUniform1i(loc, 0);
	}
};

Shader * program;		/* Identifier for the shader prgoram */
GLuint const NUM_OF_SHADERS = 4;
Shader shaders[NUM_OF_SHADERS];

GLuint vao;			/* Vertex array (Containor) object. This is the index of the VAO that will be the container for
					   our buffer objects */

GLuint colourmode;	/* Index of a uniform to switch the colour mode in the vertex shader
					  I've included this to show you how to pass in an unsigned integer into
					  your vertex shader. */

/* Position and view globals */
GLfloat angle_x, angle_inc_x, x, scaler, z, y;
GLfloat angle_y, angle_inc_y, angle_z, angle_inc_z;
GLfloat light_x, light_y, light_z;
GLfloat vx, vy, vz;
GLuint drawmode;			// Defines drawing mode of sphere as points, lines or filled polygons
GLfloat alphaValue;
GLfloat step_back;

/* Uniforms*/
GLuint modelID, viewID, projectionID;
GLuint colourmodeID;
GLuint alphaValueID;
GLfloat point_sizeID;

GLfloat aspect_ratio;		/* Aspect ratio of the window defined in the reshape callback*/

TinyObjLoader lamppost, table;			// This is an instance of our basic object loaded
Sphere aSphere(false);		// Create our sphere with no texture coordinates because they aren't handled in the shaders for this example

/* Define textureID*/
GLuint const NUM_OF_TEXTURES = 6;
GLuint texID, particle_texID, floor_texID, back_wall_texID, table_texID, window_texID;

/* Point sprite object and adjustable parameters */
points* point_anim;
GLfloat speed;
GLfloat maxdist;
GLfloat point_size;		// Used to adjust point size in the vertex shader

GLuint floor_vbo, back_wall_vbo, side_wall_vbo, window_vbo;

//GLfloat * quad_data;
// Create data for our quad with vertices, normals and texturee coordinates 
GLfloat floor_data[] = {
	// Vertex positions
	2.75f, 0, -2.75f,
	-2.75f, 0, -2.75f,
	-2.75f, 0, 2.75f,
	2.75f, 0, 2.75f,

	// Normals
	0, 1.f, 0,
	0, 1.f, 0,
	0, 1.f, 0,
	0, 1.f, 0,

	// Texture coordinates. Note we only need two per vertex but have a
	// redundant third to fit the texture coords in the same buffer for this simple object
	/*0.0f, 0.0f, 0,
	5.0f, 0.0f, 0,
	5.0f, 5.0f, 0,
	0.0f, 5.0f, 0,*/

	0.0f, 0.0f, 0,
	1.0f, 0.0f, 0,
	1.0f, 1.0f, 0,
	0.0f, 1.0f, 0,
};

/* I don't like using namespaces in header files but have less issues with them in
   seperate cpp files */
using namespace std;
using namespace glm;

/**
*Uses stb_image.h to loadand imageand create a texture from it
*/
bool load_texture(const char* filename, GLuint& texID, bool bGenMipmaps, bool flip = true)
{
	glGenTextures(1, &texID);
	// local image parameters
	int width, height, nrChannels;

	stbi_set_flip_vertically_on_load(flip);

	/* load an image file using stb_image */
	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

	// check for an error during the load process
	if (data)
	{
		// Note: this is not a full check of all pixel format types, just the most common two!
		int pixel_format = 0;
		if (nrChannels == 3)
			pixel_format = GL_RGB;
		else
			pixel_format = GL_RGBA;

		// Bind the texture ID before the call to create the texture.
			// texID[i] will now be the identifier for this specific texture
		glBindTexture(GL_TEXTURE_2D, texID);

		// Create the texture, passing in the pointer to the loaded image pixel data
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);

		// Generate Mip Maps
		if (bGenMipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		printf("stb_image  loading error: filename=%s", filename);
		return false;
	}
	stbi_image_free(data);
	return true;
}


/*
This function is called before entering the main rendering loop.
Use it for all your initialisation stuff
*/
void init(GLWrapper *glw)
{
	/* Set the object transformation controls to their initial values */
	x = 0;//0.05f;
	y = 0;
	z = 0;
	angle_x = angle_y = angle_z = 0;
	angle_inc_x = angle_inc_y = angle_inc_z = 0;
	light_x = 0; light_y = 0.5f; light_z = 0;
	vx = vy = vz = 0;
	scaler = 1.f;
	aspect_ratio = 1.3333f;
	colourmode = 0;
	alphaValue = 0.4;
	step_back = 0.f;

	// Set point parameters
	speed = 0.5f;//0.1f;
	maxdist = 0.162f; ;//1.f;
	point_anim = new points(1000, maxdist, speed);
	point_anim->create();
	point_size = 15;
	
	// Generate index (name) for one vertex array object
	glGenVertexArrays(1, &vao);

	// Create the vertex array object and make it current
	glBindVertexArray(vao);

	/* Load and create our lamppost object*/
	lamppost.load_obj("lamp_post_4.obj");
	// Set all the colour vertices to blue
	// Note that this is a quick hack, it would be better to load and process the mtl file
	lamppost.overrideColour(vec4(0.8f, 0.8f, 0.8f, 1.f));

	table.load_obj("table_with_tex.obj");
	table.overrideColour(vec4(0.8f, 0.8f, 0.8f, 1.f));

	// Creater the sphere (params are num_lats and num_longs)
	aSphere.makeSphere(60, 60);

	GLuint* textures[] = { &texID, &particle_texID, &floor_texID, &back_wall_texID, &table_texID, &window_texID};
	const GLchar* texture_filenames[] = { "..\\..\\images\\glass1.jpg", "..\\..\\images\\snowflake2.png", "..\\..\\images\\wooden_plank_2.jpg", "..\\..\\images\\wooden_plank_3.jpg", "..\\..\\images\\wood_table_1.jpg", "..\\..\\images\\old_house_window.jpg" };
	for (int i = 0; i < NUM_OF_TEXTURES; i++) {
		if (!load_texture(texture_filenames[i], *textures[i], true, false))
		{
			cout << "Fatal error loading texture: " << endl;
			exit(0);
		}
	}

	/*if (!load_texture("..\\..\\images\\glass1.jpg", texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	if (!load_texture("..\\..\\images\\snowflake2.png", particle_texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	if (!load_texture("..\\..\\images\\wooden_plank_2.jpg", floor_texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	if (!load_texture("..\\..\\images\\wooden_plank_3.jpg", back_wall_texID, true, false))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	if (!load_texture("..\\..\\images\\wood_table_1.jpg", table_texID, true, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}

	if (!load_texture("..\\..\\images\\old_house_window.jpg", window_texID, true, true))
	{
		cout << "Fatal error loading texture: " << endl;
		exit(0);
	}*/

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	// Enable gl_PointSize
	glEnable(GL_PROGRAM_POINT_SIZE);

	/* Load and build the vertex and fragment shaders */
	try
	{
		shaders[0] = Shader(glw->LoadShader("lamppost.vert", "lamppost.frag"));
		shaders[1] = Shader(glw->LoadShader("glass.vert", "glass.frag"));
		shaders[2] = Shader(glw->LoadShader("point_sprites.vert", "point_sprites.frag"));
		shaders[3] = Shader(glw->LoadShader("floor.vert", "floor.frag"));
	}
	catch (exception& e)
	{
		cout << "Caught exception: " << e.what() << endl;
		cin.ignore();
		exit(0);
	}

	// Set first shader as default
	program = &shaders[0];

	// Create our quad and texture
	glGenBuffers(1, &floor_vbo);
	glGenBuffers(1, &back_wall_vbo);
	glGenBuffers(1, &side_wall_vbo);
	glGenBuffers(1, &window_vbo);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/* Called to update the display. Note that this function is called in the event loop in the wrapper
   class because we registered display as a callback function */
void display()
{
	/* Define the background colour */
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	/* Clear the colour and frame buffers */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Enable depth test  */
	glEnable(GL_DEPTH_TEST);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	mat4 projection = perspective(radians(30.0f), aspect_ratio, 0.1f, 100.0f);

	// Camera matrix
	mat4 view = lookAt(
		vec3(0, 0, 4), // Camera is at (0,0,4), in World Space
		vec3(0, 0, 0), // and looks at the origin
		vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Apply rotations to the view position. This wil get appleid to the whole scene
	view = translate(view, vec3(0.f, 0.f, -step_back));
	view = rotate(view, -radians(vx), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	view = rotate(view, -radians(vy), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	view = rotate(view, -radians(vz), vec3(0, 0, 1));

	// Define the light position and transform by the view matrix
	vec4 lightpos = view * vec4(light_x, light_y, light_z, 1.0);

	program = &shaders[1]; // Change to 0
	glUseProgram(program->shaderID);

	mat4 model = mat4(1.0f);
	model = translate(model, vec3(light_x, light_y, light_z));
	model = scale(model, vec3(0.05f, 0.05f, 0.05f)); // make a small sphere
	// Recalculate the normal matrix and send the model and normal matrices to the vertex shader																							// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																								// Recalculate the normal matrix and send to the vertex shader																						// Recalculate the normal matrix and send to the vertex shader
	/*glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);
	normalmatrix = transpose(inverse(mat3(view * model.top())));
	glUniformMatrix3fv(normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);*/

	glUniform1ui(program->colourmodeID, colourmode);
	glUniformMatrix4fv(program->viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(program->projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform1f(program->alphaValueID, alphaValue);
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);

	/* Draw our lightposition sphere  with emit mode on*/
	/*emitmode = 1;
	glUniform1ui(emitmodeID, emitmode);*/
	aSphere.drawSphere(drawmode);
	/*emitmode = 0;
	glUniform1ui(emitmodeID, emitmode);*/
	

	/* Make the compiled shader program current */
	// Draw lamppost
	program = &shaders[0]; // Change to 0
	glUseProgram(program->shaderID);

	// Define the model transformations for the object
	model = mat4(1.0f);
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); 
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); 
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1));
	model = translate(model, vec3(x, y - 0.1f, z)); // - 0.8f
	model = scale(model, vec3(scaler / 5.f, scaler / 5.f, scaler / 5.f));

	// Send our uniforms variables to the currently bound shader,
	glUniform1ui(program->colourmodeID, colourmode);
	glUniformMatrix4fv(program->viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(program->projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform1f(program->alphaValueID, alphaValue);
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);

	// Use the lamppost texture
	glBindTexture(GL_TEXTURE_2D, texID);

	/* Draw our Blender lamppost object */
	lamppost.drawObject(drawmode);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Draw table

	// Define the model transformations for the object
	model = mat4(1.0f);
	model = translate(model, vec3(x, y - 0.1f, z)); // - 0.8f
	model = translate(model, vec3(0.f, -2.f, 0.f)); // - 0.8f
	model = scale(model, vec3(scaler / 2.f, scaler / 2.f, scaler / 2.f));
	model = rotate(model, -radians(90.f), vec3(0, 1, 0));

	// Send our uniforms variables to the currently bound shader,
	/*glUniform1ui(program->colourmodeID, colourmode);
	glUniformMatrix4fv(program->viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(program->projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform1f(program->alphaValueID, alphaValue);*/
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);

	// Use the lamppost texture
	glBindTexture(GL_TEXTURE_2D, table_texID);

	/* Draw our Blender table object */
	table.drawObject(drawmode);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Particle animation

	glEnable(GL_BLEND); // Maybe we need this here because of overlapping stars?

	// Switch to particle shader
	program = &shaders[2];
	glUseProgram(program->shaderID);

	glBindTexture(GL_TEXTURE_2D, particle_texID);

	mat4 rotation_matrix = mat4(1.f);

	// Define the model transformations 
	model = mat4(1.0f);
	model = translate(model, vec3(x, y, z));
	model = scale(model, vec3(scaler * 5, scaler * 5, scaler * 5));//scale equally in all axis
	// These probably should be removed so the snow doesnt rotate
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	rotation_matrix = rotate(rotation_matrix, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	rotation_matrix = rotate(rotation_matrix, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	rotation_matrix = rotate(rotation_matrix, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);
	glUniform1ui(program->colourmodeID, colourmode);
	glUniform1f(program->point_sizeID, point_size);
	glUniformMatrix4fv(program->viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(program->projectionID, 1, GL_FALSE, &projection[0][0]);

	point_anim->updateAngle(angle_x, angle_y, angle_z, rotation_matrix);
	point_anim->draw();
	point_anim->animate();

	glBindTexture(GL_TEXTURE_2D, 0);

	// Draw floor
	program = &shaders[3];
	glUseProgram(program->shaderID);
	
	glBindTexture(GL_TEXTURE_2D, floor_texID);

	// Define the model transformations 
	model = mat4(1.0f);
	//model = translate(model, vec3(x, y, z));
	model = translate(model, vec3(0.f, -3.f, 0.7f));
	model = scale(model, vec3(2.f, 1.0f, 1.f));//scale equally in all axis

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);
	glUniform1ui(program->colourmodeID, colourmode);
	glUniformMatrix4fv(program->viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(program->projectionID, 1, GL_FALSE, &projection[0][0]);
	//glUniform4fv(program->lightposID, 1, GL_FALSE, value_ptr(lightpos));
	glUniform4fv(program->lightposID, 1, value_ptr(lightpos));

	mat3 normalmatrix = transpose(inverse(mat3(view * model)));
	glUniformMatrix3fv(program->normalmatrixID, 1, GL_FALSE, &normalmatrix[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, floor_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_data), floor_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Draw back wall
	glBindTexture(GL_TEXTURE_2D, back_wall_texID);

	// Define the model transformations 
	model = mat4(1.0f);
	model = translate(model, vec3(0.f, -0.3f, -2.f));	
	model = rotate(model, -radians(-90.f), vec3(1, 0, 0)); 
	model = scale(model, vec3(2.f, 1.5f, 1.f));

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, back_wall_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_data), floor_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Draw side wall
	glBindTexture(GL_TEXTURE_2D, back_wall_texID);

	// Define the model transformations 
	model = mat4(1.0f);
	model = translate(model, vec3(2.5f, -0.3f, 0.7f));
	model = rotate(model, -radians(-90.f), vec3(0, 1, 0));
	model = rotate(model, -radians(-90.f), vec3(1, 0, 0));
	model = scale(model, vec3(1.f, 1.f, 1.f));

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, side_wall_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_data), floor_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Draw window
	glBindTexture(GL_TEXTURE_2D, window_texID);

	// Define the model transformations 
	model = mat4(1.0f);
	model = translate(model, vec3(2.499f, 0.5f, 0.7f));
	model = rotate(model, -radians(-90.f), vec3(0, 1, 0));
	model = rotate(model, -radians(-90.f), vec3(1, 0, 0));
	model = scale(model, vec3(0.6f, 0.6f, 0.6f));

	// Send our uniforms variables to the currently bound shader,
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);

	glBindBuffer(GL_ARRAY_BUFFER, window_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_data), floor_data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(12 * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(24 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// Snowglobe
	// Note: Needs to be last because of Blending
	program = &shaders[1];
	glUseProgram(program->shaderID);

	/* Define the model transformations for our sphere */
	model = mat4(1.0f);
	model = translate(model, vec3(x, y, z));
	model = scale(model, vec3(scaler / 1.2f, scaler / 1.2f, scaler / 1.2f));//scale equally in all axis
	model = rotate(model, -radians(angle_x), vec3(1, 0, 0)); //rotating in clockwise direction around x-axis
	model = rotate(model, -radians(angle_y), vec3(0, 1, 0)); //rotating in clockwise direction around y-axis
	model = rotate(model, -radians(angle_z), vec3(0, 0, 1)); //rotating in clockwise direction around z-axis

	glUniform1ui(program->colourmodeID, colourmode);
	glUniformMatrix4fv(program->viewID, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(program->projectionID, 1, GL_FALSE, &projection[0][0]);
	glUniform1f(program->alphaValueID, alphaValue);
	glUniformMatrix4fv(program->modelID, 1, GL_FALSE, &model[0][0]);

	/* Draw our sphere */
	/* Note that you probably want a different texture for this Sphere! */
	//glBindTexture(GL_TEXTURE_2D, texID);
	
	glEnable(GL_BLEND);

	aSphere.drawSphere(drawmode);

	//glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);

	/* Modify our animation variables */
	angle_x += angle_inc_x;
	angle_y += angle_inc_y;
	angle_z += angle_inc_z;	
}

/* Called whenever the window is resized. The new window size is given, in pixels. */
static void reshape(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	aspect_ratio = ((float)w / 640.f*4.f) / ((float)h / 480.f*3.f);
}

/* change view angle, exit upon ESC */
static void keyCallback(GLFWwindow* window, int key, int s, int action, int mods)
{
	/* Enable this call if you want to disable key responses to a held down key*/
	//if (action != GLFW_PRESS) return;

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == 'Q') angle_inc_x -= 0.05f;
	if (key == 'W') angle_inc_x += 0.05f;
	if (key == 'E') angle_inc_y -= 0.05f;
	if (key == 'R') angle_inc_y += 0.05f;
	if (key == 'T') angle_inc_z -= 0.05f;
	if (key == 'Y') angle_inc_z += 0.05f;
	if (key == 'A') scaler -= 0.02f;
	if (key == 'S') scaler += 0.02f;
	if (key == 'Z') x -= 0.05f;
	if (key == 'X') x += 0.05f;
	if (key == 'C') y -= 0.05f;
	if (key == 'V') y += 0.05f;
	if (key == 'B') z -= 0.05f;
	if (key == 'N') z += 0.05f;

	// This is ball speed pls change
	if (key == '1') light_x -= speed;
	if (key == '2') light_x += speed;
	if (key == '3') light_y -= speed;
	if (key == '4') light_y += speed;
	if (key == '5') light_z -= speed;
	if (key == '6') light_z += speed;

	if (key == '7') vx -= 1.f;
	if (key == '8') vx += 1.f;
	if (key == '9') vy -= 1.f;
	if (key == '0') vy += 1.f;
	if (key == 'O') vz -= 1.f;
	if (key == 'P') vz += 1.f;

	if (key == 'K') step_back += 1.f;
	if (key == 'L') step_back -= 1.f;


	if (key == 'M' && action != GLFW_PRESS)
	{
		colourmode = !colourmode;
		cout << "colourmode=" << colourmode << endl;
	}

	/* Cycle between drawing vertices, mesh and filled polygons */
	if (key == 'N' && action != GLFW_PRESS)
	{
		drawmode ++;
		if (drawmode > 2) drawmode = 0;
	}

}



/* Entry point of program */
int main(int argc, char* argv[])
{
	GLWrapper *glw = new GLWrapper(1024, 768, "Snowglobe");;

	if (!ogl_LoadFunctions())
	{
		fprintf(stderr, "ogl_LoadFunctions() failed. Exiting\n");
		return 0;
	}

	glw->setRenderer(display);
	glw->setKeyCallback(keyCallback);
	glw->setReshapeCallback(reshape);

	init(glw);

	glw->eventLoop();

	delete(glw);
	return 0;
}



