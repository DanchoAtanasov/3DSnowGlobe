/** Basic Point class to use in an example particle animation 
 Iain Martin
 November 2018
 */

#include "points.h"
#include "glm/gtc/random.hpp"
#include "glm/gtc/matrix_transform.hpp"

/* Constructor, set initial parameters*/
points::points(GLuint number, GLfloat dist, GLfloat sp)
{
	numpoints = number;
	maxdist = dist;
	speed = sp;
	angle_x = angle_y = angle_z = 0.f;
}


points::~points()
{
	delete[] colours;
	delete[] vertices;
	delete[] velocity;
}

void points::updateParams(GLfloat dist, GLfloat sp)
{
	maxdist = dist;
	speed = sp;
}


void  points::create()
{
	vertices = new glm::vec3[numpoints];
	colours = new glm::vec3[numpoints];
	velocity = new glm::vec3[numpoints];
	initial_direction = new glm::vec3[numpoints];

	/* Define random position and velocity */
	for (int i = 0; i < numpoints; i++)
	{
		vertices[i] = glm::ballRand(0.08f); // 0.1f //1.f
		colours[i] = glm::vec3(170.f/255, 213.f/255, 247.f/255);  // Set to snowflake colour
		//velocity[i] = glm::vec3(glm::ballRand(glm::linearRand(0.0, 0.01)));
		//velocity[i] = glm::vec3(glm::linearRand(-0.01, 0.01), -glm::linearRand(0.005, 0.01), 0.f);  // Move in the negative y direction
		velocity[i] = glm::vec3(glm::linearRand(-0.01, 0.01), -glm::linearRand(0.005, 0.01), glm::linearRand(-0.01, 0.01));
		initial_direction[i] = velocity[i];
	}

	/* Create the vertex buffer object */
	/* and the vertex buffer positions */
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), vertices, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &colour_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, colour_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), colours, GL_STATIC_DRAW);
}


void points::draw()
{
	/* Bind  vertices. Note that this is in attribute index 0 */
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Bind cube colours. Note that this is in attribute index 1 */
	glBindBuffer(GL_ARRAY_BUFFER, colour_buffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Draw our points*/
	glDrawArrays(GL_POINTS, 0, numpoints);
}


void points::animate()
{
	for (int i = 0; i < numpoints; i++)
	{
		glm::vec3 new_vertex = vertices[i] + velocity[i] / 50.f * speed;
		
		GLfloat dist = glm::length(new_vertex); // Calculate distance to the origin
		if (dist < maxdist) vertices[i] = new_vertex;
		else if (dist == maxdist) {}
		else vertices[i] = vertices[i] * (maxdist / dist); // Stop snowflakes at maxdistance which should be the radius of the snowglobe

		// If we are near the origin then we introduce a new random direction
		//if (dist < 0.01f) velocity[i] = glm::vec3(glm::ballRand(glm::linearRand(0.0, 0.02)));

		// If we are too far away then change direction back to the origin
		//if (dist > maxdist) velocity[i] = -vertices[i] / 500.f * speed;
		//if (dist > maxdist) velocity[i] = glm::vec3(0.f, 0.f, 0.f); //*= -1;
	}

	// Update the vertex buffer data
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, numpoints * sizeof(glm::vec3), vertices, GL_DYNAMIC_DRAW);
}

void points::updateAngle(GLfloat x, GLfloat y, GLfloat z, glm::mat4 rotation_matrix) {
	// If the snowglobe is rotated more than 10 degrees in any direction recalculate the snowflake direction
	// to always go down by using the inverse matrix of the rotation matrix
	if (abs(x - angle_x) > 10.f || abs(y - angle_y) > 10.f || abs(z - angle_z) > 10.f){
		glm::mat4 fix_matrix = glm::inverse(rotation_matrix);
		for (int i = 0; i < numpoints; i++)
		{
			velocity[i] = glm::vec3(fix_matrix * glm::vec4(initial_direction[i], 0.f));
		}

		angle_x = x;
		angle_y = y;
		angle_z = z;
	}
}

