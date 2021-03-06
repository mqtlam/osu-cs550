// Simple 2D OpenGL Program

//Includes vec, mat, and other include files as well as macro defs
#define GL3_PROTOTYPES

// COMMENT OR UNCOMMENT LINE BELOW DEPENDING ON PLATFORM
//#define USE_WINDOWS

// Include the vector and matrix utilities from the textbook, as well as some
// macro definitions.
#ifdef USE_WINDOWS
#include "../include/Angel.h"
#include "../headers/SceneLoader.h"
#else
#include "Angel.h"
#include "SceneLoader.h"
#endif
#include <stdio.h>
#include <string.h>
#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#endif
using namespace graphics;

typedef Angel::vec4 color4;
typedef Angel::vec4 point4;

int NumVertices = 0; //(6 faces)(2 triangles/face)(3 vertices/triangle)

GLuint model_view;  // model-view matrix uniform shader variable location
GLuint projection; // projection matrix uniform shader variable location

point4* points;
vec4* normals;
enum TransformType {
	perspective, orthonormal
};

TransformType tranformType = perspective;
string sceneFile = "scene.scn";
float fx, fy, fz, ax, ay, az, ux, uy, uz, fovy, aspect, zNear, zFar;
float left_, right_, bottom_, top_;

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = { point4(-0.5, -0.5, 0.5, 1.0), point4(-0.5, 0.5, 0.5,
		1.0), point4(0.5, 0.5, 0.5, 1.0), point4(0.5, -0.5, 0.5, 1.0), point4(
		-0.5, -0.5, -0.5, 1.0), point4(-0.5, 0.5, -0.5, 1.0), point4(0.5, 0.5,
		-0.5, 1.0), point4(0.5, -0.5, -0.5, 1.0) };

//----------------------------------------------------------------------------

int Index = 0;
void myFunc(Object object, Face face) {
	for (int i = 0; i < 3; i++) {
		Vertex vertex = object.vertices[face.vertexId[i]];
		Vertex vertexNormal = object.verticeNormals[face.vertexNormalId[i]];
		points[Index] = point4(vertex.x, vertex.y, vertex.z, 1);
		normals[Index] = vec4(vertexNormal.x, vertexNormal.y, vertexNormal.z,
				0);
		Index++;
	}
}
// quad generates two triangles for each face and assigns colors
//    to the vertices.  Notice we keep the relative ordering when constructing the tris
//void quad(int a, int b, int c, int d) {
//
//	vec4 u = vertices[b] - vertices[a];
//	vec4 v = vertices[c] - vertices[b];
//
//	vec4 normal = normalize(cross(u, v));
//	normal[3] = 0.0;
//
//	normals[Index] = normal;
//	points[Index] = vertices[a];
//	Index++;
//	normals[Index] = normal;
//	points[Index] = vertices[b];
//	Index++;
//	normals[Index] = normal;
//	points[Index] = vertices[c];
//	Index++;
//	normals[Index] = normal;
//	points[Index] = vertices[a];
//	Index++;
//	normals[Index] = normal;
//	points[Index] = vertices[c];
//	Index++;
//	normals[Index] = normal;
//	points[Index] = vertices[d];
//	Index++;
//}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
//void colorcube() {
//	quad(4, 5, 6, 7);
//	quad(5, 4, 0, 1);
//	quad(1, 0, 3, 2);
//	quad(2, 3, 7, 6);
//	quad(3, 0, 4, 7);
//	quad(6, 5, 1, 2);
//}

//----------------------------------------------------------------------------

// OpenGL initialization
void init() {

	SceneLoader sceneLoader;
	sceneLoader.loadSceneFile(sceneFile);
	sceneLoader.loadAndAddObjects();
	for (unsigned int i = 0; i < sceneLoader.objects.size(); i++) {
		NumVertices += sceneLoader.objects[i].faces.size();
	}
	NumVertices = NumVertices * 3;
	cout << NumVertices << endl;
	points = new point4[NumVertices];
	normals = new vec4[NumVertices];
	for (unsigned int i = 0; i < sceneLoader.objects.size(); i++) {
		for (unsigned int j = 0; j < sceneLoader.objects[i].faces.size(); j++) {
			myFunc(sceneLoader.objects[i], sceneLoader.objects[i].faces[j]);
		}
	}
//    colorcube();

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer( GL_ARRAY_BUFFER, buffer);
	int pointsSize = NumVertices * sizeof(point4);
	int normalzSize = NumVertices * sizeof(vec4);
	glBufferData( GL_ARRAY_BUFFER, pointsSize + normalzSize,
	NULL, GL_STATIC_DRAW);
	glBufferSubData( GL_ARRAY_BUFFER, 0, pointsSize, points);
	glBufferSubData( GL_ARRAY_BUFFER, pointsSize, normalzSize, normals);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("shaders/vshader.glsl", "shaders/fshader.glsl");
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(pointsSize));

	// Initialize shader lighting parameters
	// RAM: No need to change these...we'll learn about the details when we
	// cover Illumination and Shading
	point4 light_position(1.5, 0.5, 2.0, 1.0);
	color4 light_ambient(0.2, 0.2, 0.2, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);

	color4 material_ambient(1.0, 0.0, 1.0, 1.0);
	color4 material_diffuse(1.0, 0.8, 0.0, 1.0);
	color4 material_specular(1.0, 0.8, 0.0, 1.0);
	float material_shininess = 100.0;

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv( glGetUniformLocation(program, "AmbientProduct"), 1,
			ambient_product);
	glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"), 1,
			diffuse_product);
	glUniform4fv( glGetUniformLocation(program, "SpecularProduct"), 1,
			specular_product);

	glUniform4fv( glGetUniformLocation(program, "LightPosition"), 1,
			light_position);

	glUniform1f( glGetUniformLocation(program, "Shininess"),
			material_shininess);

	model_view = glGetUniformLocation(program, "ModelView");
	projection = glGetUniformLocation(program, "Projection");
	mat4 p;
	if (tranformType == perspective) {
		p = Perspective(fovy, aspect, zNear, zFar);
	} else {
		p = Ortho(left_, right_, bottom_, top_, zNear, zFar);
	}
	point4 eye(fx, fy, fz, 1.0);
	point4 at(ax, ay, az, 1.0);
	vec4 up(ux, uy, uz, 0.0);

	mat4 mv = LookAt(eye, at, up);
	//vec4 v = vec4(0.0, 0.0, 1.0, 1.0);

	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p);

	glEnable( GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);
}

//----------------------------------------------------------------------------

void display(void) {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays( GL_TRIANGLES, 0, NumVertices);
	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 033:  // Escape key
	case 'q':
	case 'Q':
		exit( EXIT_SUCCESS);
		break;
	}
}

//----------------------------------------------------------------------------

/*
 *  simple.c
 *  This program draws a red rectangle on a white background.
 *
 * Still missing the machinery to move to 3D
 */

/* glut.h includes gl.h and glu.h*/
int main(int argc, char** argv) {
	sceneFile = string(argv[1]);
	if (strcmp(argv[2], "p") == 0) {
		tranformType = perspective;
		fx = atof(argv[3]);
		fy = atof(argv[4]);
		fz = atof(argv[5]);
		ax = atof(argv[6]);
		ay = atof(argv[7]);
		az = atof(argv[8]);
		ux = atof(argv[9]);
		uy = atof(argv[10]);
		uz = atof(argv[11]);
		fovy = atof(argv[12]);
		aspect = atof(argv[13]);
		zNear = atof(argv[14]);
		zFar = atof(argv[15]);
	} else {
		tranformType = orthonormal;
		fx = atof(argv[3]);
		fy = atof(argv[4]);
		fz = atof(argv[5]);
		ax = atof(argv[6]);
		ay = atof(argv[7]);
		az = atof(argv[8]);
		ux = atof(argv[9]);
		uy = atof(argv[10]);
		uz = atof(argv[11]);
		left_ = atof(argv[12]);
		right_ = atof(argv[13]);
		bottom_ = atof(argv[14]);
		top_ = atof(argv[15]);
		zNear = atof(argv[16]);
		zFar = atof(argv[17]);
		//left_ = atof(argv[3]);
		//right_ = atof(argv[4]);
		//bottom_ = atof(argv[5]);
		//top_ = atof(argv[6]);
		//zNear = atof(argv[7]);
		//zFar = atof(argv[8]);
	}
	glutInit(&argc, argv);
#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitContextVersion(3, 2);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
#endif
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(500, 300);
	glutCreateWindow("Simple Open GL Program");
	printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

#ifndef __APPLE__
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	init();

	//NOTE:  callbacks must go after window is created!!!
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glutMainLoop();

	return (0);
}
