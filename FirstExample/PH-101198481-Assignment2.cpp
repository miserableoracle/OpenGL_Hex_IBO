//***************************************************************************
// PH-101198481-Assignment2.cpp by Hiren Pandya (C) 2018 All Rights Reserved.
//
// Assignment 2 submission.
//
// Description:
// This program explores the 3D objects and their perspective from the 
// user's viewpoint. The program will display two cubes, rotating, using the
// index draw method.
//***************************************************************************

using namespace std;

#include <iostream>
#include "stdlib.h"
#include "time.h"
#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"

#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(1,1,0)
#define XZ_AXIS glm::vec3(1,0,1)
#define YZ_AXIS glm::vec3(0,1,1)
#define XYZ_AXIS glm::vec3(1,1,1)

glm::mat4 mvp, view, projection;

GLuint colours_vbo, modelID;
GLuint vao_cube, points_vbo_cube, ibo_cube;

GLuint vao_hex, points_vbo_hex, ibo_hex;
// Horizontal and vertical ortho offsets.
float osH = 0.0f, osV = 0.0f, scrollSpd = 0.25f;
// Offsets for camera in_out, left_right, and up_down
float cam_in_out = 0.0f, cam_l_r = 0.0f, cam_up_dn = 0.0f;
// Rotation angle
float angle = 0.0f;
// Number of cubes to be displayed in the output
int noOfCubes = 2;

GLfloat vertices_cube[] = {
	-0.9f, 0.9f, 0.9f,		// Cube V0
	0.9f, 0.9f, 0.9f,		// Cube V1
	0.9f, -0.9f, 0.9f,		// Cube V2
	-0.9f, -0.9f, 0.9f,		// Cube V3
	-0.9f, -0.9f, -0.9f,		// Cube V4
	-0.9f, 0.9f, -0.9f,		// Cube V5
	0.9f, 0.9f, -0.9f,		// Cube V6
	0.9f, -0.9f, -0.9f		// Cube V7
};

GLushort cube_index_array[] = {
	// Front
	0, 1, 2, 3,
	// Left
	3, 4, 5, 0,
	// Right
	1, 6, 7, 2,
	// Top
	0, 5, 6, 1,
	// Buttom
	2, 7, 4, 3,
	// Back
	4, 7, 6, 5
};

GLfloat vertices_hex[] = {
	0.0f, 0.0f, 0.5f,		// Center of hex - 1
	0.866f, 0.5f, 0.5f,		// V1
	0.866f, -0.5f, 0.5f,	// V2
	0.0f, -1.0f, 0.5f,		// V3
	-0.866f, -0.5f, 0.5f,	// V4
	-0.866f, 0.5f, 0.5f,	// V5
	0.0f, 1.0f, 0.5f,		// V6

	0.0f, 0.0f, -0.5f,		// Center of hex - 2
	0.866f, 0.5f, -0.5f,		// V1
	0.866f, -0.5f, -0.5f,	// V2
	0.0f, -1.0f, -0.5f,		// V3
	-0.866f, -0.5f, -0.5f,	// V4
	-0.866f, 0.5f, -0.5f,	// V5
	0.0f, 1.0f, -0.5f		// V6
};

GLushort hex_index_array[] = {
	// Front hex
	// T1
	0, 1, 2,
	// T2
	0, 2, 3,
	// T3
	0, 3, 4,
	// T4
	0, 4, 5,
	// T5
	0, 5, 6,
	// T6
	0, 6, 1,

	// Back hex
	// T7
	7, 9, 8,
	// T8
	7, 10, 9,
	// T9
	7, 11, 10,
	// T10
	7, 12, 11,
	// T11
	7, 13, 12,
	// T12
	7, 8, 13,

	// Connecting cubes divided in triangles
	
	// T13
	6, 13, 8,
	// T14
	8, 1, 6,
	
	// T15
	1, 8, 9,
	// T16
	9, 2, 1,

	// T17
	2, 9, 10,
	// T18
	10, 3, 2,

	// T19
	3, 10, 11,
	// T20
	11, 4, 3,

	// T21
	4, 11, 12,
	// T22
	12, 5, 4,

	// T23
	5, 12, 13,
	// T24
	13, 6, 5
};

GLfloat colours[] = { 
	1.0f, 1.0f, 1.0f,		// Front -- First hex
	0.0f, 1.0f, 0.0f,		
	1.0f, 0.0f, 1.0f,		
	0.0f, 0.0f, 1.0f,		
	1.0f, 0.0f, 0.0f,		
	1.0f, 0.0f, 1.0f,		
	1.0f, 1.0f, 1.0f,		// Front -- Second hex
	0.0f, 1.0f, 0.0f,		
	1.0f, 0.0f, 1.0f,		
	0.0f, 0.0f, 1.0f,		
	1.0f, 0.0f, 0.0f,		
	1.0f, 0.0f, 1.0f		
};

void init(void)
{
	//Specifying the name of vertex and fragment shaders.
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	//Loading and compiling shaders
	GLuint program = LoadShaders(shaders);
	glUseProgram(program);

	modelID = glGetUniformLocation(program, "mvp");

	// Projection - Setting camera into perspective mode
	projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
	// View (Camera) Setup
	// Setting up camera on Y-Z plane and center should be (0, 0, 0)
	view = glm::lookAt(
		glm::vec3(0, 1.0f, 6.0f),
		glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0)
	);

	vao_cube = 0;
	glGenVertexArrays(1, &vao_cube);
	glBindVertexArray(vao_cube);

	glGenBuffers(1, &ibo_cube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_index_array), cube_index_array, GL_STATIC_DRAW);

	glGenBuffers(1, &points_vbo_cube);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo_cube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_cube), vertices_cube, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &colours_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	//glBindBuffer(GL_ARRAY_BUFFER, 0); // Can optionally unbind the buffer to avoid modification.
		
	glBindVertexArray(0); // Can optionally unbind the vertex array to avoid modification.

	vao_hex = 0;
	glGenVertexArrays(1, &vao_hex);
	glBindVertexArray(vao_hex);

	glGenBuffers(1, &ibo_hex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_hex);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(hex_index_array), hex_index_array, GL_STATIC_DRAW);

	glGenBuffers(1, &points_vbo_hex);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo_hex);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_hex), vertices_hex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &colours_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colours), colours, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
		
	// Enable depth test.
	glEnable(GL_DEPTH_TEST);
	//glFlush();
}

//---------------------------------------------------------------------
//
// transformModel
//

void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, glm::vec3(scale));
	mvp = projection * view * Model;
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &mvp[0][0]);
}

//---------------------------------------------------------------------
//
// display
//

void display(void)
{
	// Setting up camera on Y-Z plane and center should be (0, 0, 0)
	view = glm::lookAt(
		glm::vec3(0.1f + cam_l_r, 1.0f + cam_up_dn, 6.0f + cam_in_out),
		glm::vec3(0, 0, 0),
		glm::vec3(0.0f, 1.0f, 0)
	);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.01f, 0.2f, 0.4f, 0.0f);
	glBindVertexArray(vao_cube);

	// Initial position of the cube - First position (1.35 - 0.9 = 0.45f)
	float initPosition = 1.35f;
	// Cube rotation direction
	int rotationDir = 1;

	// For loop to draw elements on the screen based on number of cubes suggested by user
	for (int i = 0; i < noOfCubes; i++)
	{
		//transformObject(glm::vec3(0.45f), Y_AXIS * glm::vec3(rotationDir), angle += 2.01f, glm::vec3(0.0f, initPosition -= 0.9f, 0.0f));
		//Ordering the GPU to start the pipeline
		//glDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, 0);
		//rotationDir *= -1;
	}

	// Draw the platform after all the cubes have been drawn
	//transformObject(glm::vec3(2.0f, 0.08f, 2.0f), -Y_AXIS, 0, glm::vec3(0.0f, initPosition -= 0.7f, 0.0f));
	//Ordering the GPU to start the pipeline
	//glDrawElements(GL_QUADS, 24, GL_UNSIGNED_SHORT, 0);
	
	glBindVertexArray(0); // Can optionally unbind the vertex array to avoid modification.

	glBindVertexArray(vao_hex);
	transformObject(glm::vec3(0.45f), Y_AXIS, angle += 1.01f, glm::vec3(0.0f, initPosition -= 1.9f, 0.0f));
	//Ordering the GPU to start the pipeline
	glDrawElements(GL_TRIANGLES, 72, GL_UNSIGNED_SHORT, 0);

	//glFlush(); // Instead of double buffering.
	glutSwapBuffers(); // Instead of double buffering.
}

// Keyboard input
void keyDown(unsigned char key, int x, int y)
{
	// Orthographic.
	switch (key)
	{
	// W and S for camera in and out
	case 'w':
		cam_in_out -= scrollSpd;
		break;
	case 's':
		cam_in_out += scrollSpd;
		break;
	// A and D for camera left and right
	case 'a':
		cam_l_r += scrollSpd;
		break;
	case 'd':
		cam_l_r -= scrollSpd;
		break;
	// R and F for camera up and down
	case 'r':
		cam_up_dn += scrollSpd;
		break;
	case 'f':
		cam_up_dn -= scrollSpd;
		break;
	}
}

void idle()
{
	//glutPostRedisplay();
}

void timer(int id)
{
	glutPostRedisplay();
	glutTimerFunc(33, timer, 0);
}

//---------------------------------------------------------------------
//
// main
//

int main(int argc, char** argv)
{
	// Ask user to enter the number of cubes to be displayed on the screen
	cout << "Enter the number of cube to display (Default: 2): ";
	cin >> noOfCubes;
	// Input validation
	while (!cin) {
		cin.clear();
		cin.ignore(10000, '\n');
		cout << "Invalid Input. Please try again: ";
		cin >> noOfCubes;
	}

	// Default number of cubes is 2, it should be reset to 2 incase of lower input
	if (noOfCubes < 2)
		noOfCubes = 2;
	glutInit(&argc, argv);
	//glutInitDisplayMode(GLUT_RGBA);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// Window size
	glutInitWindowSize(1024, 1024);
	// Window title
	glutCreateWindow("Pandya, Hiren, 101198481");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.
	init();

	glutDisplayFunc(display);
	//glutIdleFunc(idle);
	glutTimerFunc(33, timer, 0);
	glutKeyboardFunc(keyDown);
	glutMainLoop();
}
