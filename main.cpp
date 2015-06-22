// Base code for program 3 - does not compile as is, needs other files 
//and shaders but has skeleton for much of the data transfer for shading
//and traversal of the mesh for computing normals - you must compute normals

#include <stdio.h>
#include <stdlib.h>
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <stdio.h>
#include "GLSL.h"
#include "tiny_obj_loader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "RenderingHelper.h"

GLFWwindow* window;
using namespace std;

vector<tinyobj::shape_t> shapes;
vector<tinyobj::shape_t> cube;
vector<tinyobj::shape_t> sopwith;
vector<tinyobj::material_t> materials;

int g_SM = 1;
int g_N = 0;
int g_width;
int g_height;
float g_angle = 0;
float g_anglex = 0;
int g_mat_id = 0;
glm::vec3 g_trans(0, 0, 5);
glm::vec3 g_trans2(5, 0, 0);
glm::vec3 g_trans3(-5, 0, 0);
glm::vec3 g_trans4(0, 0, -5);
glm::vec3 g_light(0, 220, 0);

glm::vec3 position = glm::vec3(0, 3, 0);
float theta = 0.0f;
float phi = -0.6f;

float speed = 3.0f;
float mouseSpeed = 0.005f;

GLuint ShadeProg;
GLuint posBufObjB = 0;
GLuint norBufObjB = 0;
GLuint indBufObjB = 0;

GLuint posBufObjC = 0;
GLuint norBufObjC = 0;
GLuint indBufObjC = 0;

GLuint posBufObjS = 0;
GLuint norBufObjS = 0;
GLuint indBufObjS = 0;

GLuint posBufObjG = 0;
GLuint norBufObjG = 0;

//Handles to the shader data
GLint h_aPosition;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uLightPos;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;
GLint h_uShadeM;
GLint h_uN;

RenderingHelper ModelTrans;

/* helper function to make sure your matrix handle is correct */
inline void safe_glUniformMatrix4fv(const GLint handle, const GLfloat data[]) {
	if (handle >= 0)
		glUniformMatrix4fv(handle, 1, GL_FALSE, data);
}

/* helper function to send materials to the shader - you must create your own */
void SetMaterial(int i) {

	glUseProgram(ShadeProg);
	switch (i) {
	case 0: //shiny blue plastic
		glUniform3f(h_uMatAmb, 0.02, 0.02, 0.1);
		glUniform3f(h_uMatDif, 0.0, 0.08, 0.5);
		glUniform3f(h_uMatSpec, 0.14, 0.14, 0.4);
		glUniform1f(h_uMatShine, 120.0);
		break;
	case 1: // flat grey
		glUniform3f(h_uMatAmb, 0.13, 0.13, 0.14);
		glUniform3f(h_uMatDif, 0.3, 0.3, 0.4);
		glUniform3f(h_uMatSpec, 0.3, 0.3, 0.4);
		glUniform1f(h_uMatShine, 4.0);
		break;
	case 2: //gold
		glUniform3f(h_uMatAmb, 0.09, 0.07, 0.08);
		glUniform3f(h_uMatDif, 0.91, 0.782, 0.82);
		glUniform3f(h_uMatSpec, 1.0, 0.913, 0.8);
		glUniform1f(h_uMatShine, 200.0);
		break;
	case 3: //turqoise 
		glUniform3f(h_uMatAmb, 0.09, 0.07, 0.08);
		glUniform3f(h_uMatDif, 0.04, 0.6, 0.8);
		glUniform3f(h_uMatSpec, 1.0, 0.913, 0.8);
		glUniform1f(h_uMatShine, 400.0);
		break;

	case 4: // airport tarmac
		glUniform3f(h_uMatAmb, 0.0, 0.0, 0.0);
		glUniform3f(h_uMatDif, 0.3, 0.3, 0.4);
		glUniform3f(h_uMatSpec, 0.0, 0.0, 0.0);
		glUniform1f(h_uMatShine, 0.0);
		break;

	case 5: //pedestal
		glUniform3f(h_uMatAmb, 0.2, 0.4, 0.0);
		glUniform3f(h_uMatDif, 0.2, 0.4, 0.0);
		glUniform3f(h_uMatSpec, 0.2, 0.4, 0.0);
		glUniform1f(h_uMatShine, 2.0);
		break;

	case 6: //airplane
		glUniform3f(h_uMatAmb, 0.09, 0.07, 0.08);
		glUniform3f(h_uMatDif, 0.91, 0.782, 0.82);
		glUniform3f(h_uMatSpec, 1.0, 0.913, 0.8);
		glUniform1f(h_uMatShine, 200.0);
		break;
	}


}

/* helper function to set projection matrix - don't touch */
void SetProjectionMatrix() {
	glm::mat4 Projection = glm::perspective(90.0f, (float)g_width / g_height, 0.1f, 100.f);
	safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change beyond the current set up to rotate*/
void SetView() {
	static double lastTime = glfwGetTime();
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	double xpos, ypos;

	glfwGetCursorPos(window, &xpos, &ypos);
	glfwSetCursorPos(window, 512, 384);

	theta += mouseSpeed * float(512 - xpos);
	phi += mouseSpeed * float(384 - ypos);

	phi = glm::max(phi, -1.4f);
	phi = glm::min(phi, 1.4f);

	glm::vec3 direction(
		cos(phi) * sin(theta),
		sin(phi),
		cos(phi) * cos(theta)
		);

	glm::vec3 right = glm::vec3(
		sin(theta - 1.57f),
		0,
		cos(theta - 1.57f)
		);

	glm::vec3 up = glm::cross(right, direction);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		position += direction * deltaTime * speed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		position -= direction * deltaTime * speed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		position += right * deltaTime * speed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		position -= right * deltaTime * speed;

	glm::mat4 Trans = glm::lookAt(
		position,          
		position + direction, 
		up                  
		);

	safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(Trans));

	lastTime = currentTime;
}

void SetModel(glm::vec3 trans, float rot) {
	glm::mat4 Trans = glm::translate(glm::mat4(1.0f), trans);
	glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), rot, glm::vec3(0.0f, 1, 0));
	glm::mat4 com = Trans*RotateY;
	safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(com));
}


//Given a vector of shapes which has already been read from an obj file
// resize all vertices to the range [-1, 1]
void resize_obj(std::vector<tinyobj::shape_t> &shapes){
	float minX, minY, minZ;
	float maxX, maxY, maxZ;
	float scaleX, scaleY, scaleZ;
	float shiftX, shiftY, shiftZ;
	float epsilon = 0.001;

	minX = minY = minZ = 1.1754E+38F;
	maxX = maxY = maxZ = -1.1754E+38F;

	//Go through all vertices to determine min and max of each dimension
	for (size_t i = 0; i < shapes.size(); i++) {
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			if (shapes[i].mesh.positions[3 * v + 0] < minX) minX = shapes[i].mesh.positions[3 * v + 0];
			if (shapes[i].mesh.positions[3 * v + 0] > maxX) maxX = shapes[i].mesh.positions[3 * v + 0];

			if (shapes[i].mesh.positions[3 * v + 1] < minY) minY = shapes[i].mesh.positions[3 * v + 1];
			if (shapes[i].mesh.positions[3 * v + 1] > maxY) maxY = shapes[i].mesh.positions[3 * v + 1];

			if (shapes[i].mesh.positions[3 * v + 2] < minZ) minZ = shapes[i].mesh.positions[3 * v + 2];
			if (shapes[i].mesh.positions[3 * v + 2] > maxZ) maxZ = shapes[i].mesh.positions[3 * v + 2];
		}
	}
	//From min and max compute necessary scale and shift for each dimension
	float maxExtent, xExtent, yExtent, zExtent;
	xExtent = maxX - minX;
	yExtent = maxY - minY;
	zExtent = maxZ - minZ;
	if (xExtent >= yExtent && xExtent >= zExtent) {
		maxExtent = xExtent;
	}
	if (yExtent >= xExtent && yExtent >= zExtent) {
		maxExtent = yExtent;
	}
	if (zExtent >= xExtent && zExtent >= yExtent) {
		maxExtent = zExtent;
	}
	scaleX = 2.0 / maxExtent;
	shiftX = minX + (xExtent / 2.0);
	scaleY = 2.0 / maxExtent;
	shiftY = minY + (yExtent / 2.0);
	scaleZ = 2.0 / maxExtent;
	shiftZ = minZ + (zExtent) / 2.0;

	//Go through all verticies shift and scale them
	for (size_t i = 0; i < shapes.size(); i++) {
		for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
			shapes[i].mesh.positions[3 * v + 0] = (shapes[i].mesh.positions[3 * v + 0] - shiftX) * scaleX;
			assert(shapes[i].mesh.positions[3 * v + 0] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 0] <= 1.0 + epsilon);
			shapes[i].mesh.positions[3 * v + 1] = (shapes[i].mesh.positions[3 * v + 1] - shiftY) * scaleY;
			assert(shapes[i].mesh.positions[3 * v + 1] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 1] <= 1.0 + epsilon);
			shapes[i].mesh.positions[3 * v + 2] = (shapes[i].mesh.positions[3 * v + 2] - shiftZ) * scaleZ;
			assert(shapes[i].mesh.positions[3 * v + 2] >= -1.0 - epsilon);
			assert(shapes[i].mesh.positions[3 * v + 2] <= 1.0 + epsilon);
		}
	}
}



void loadShapes(const string &objFile, std::vector<tinyobj::shape_t>& shapes)
{
	string err = tinyobj::LoadObj(shapes, materials, objFile.c_str());
	if (!err.empty()) {
		cerr << err << endl;
	}
	resize_obj(shapes);
}


void initBunny(std::vector<tinyobj::shape_t>& shape) {

	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjB);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjB);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjB);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjB);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initCube(std::vector<tinyobj::shape_t>& shape) {

	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjC);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjC);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjC);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initSopwith(std::vector<tinyobj::shape_t>& shape) {

	// Send the position array to the GPU
	const vector<float> &posBuf = shape[0].mesh.positions;
	glGenBuffers(1, &posBufObjS);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);

	// Send the normal array to the GPU
	vector<float> norBuf;
	glm::vec3 v1, v2, v3;
	glm::vec3 edge1, edge2, norm;
	int idx1, idx2, idx3;
	//for every vertex initialize the vertex normal to 0
	for (int j = 0; j < shape[0].mesh.positions.size() / 3; j++) {
		norBuf.push_back(0);
		norBuf.push_back(0);
		norBuf.push_back(0);
	}
	//process the mesh and compute the normals - for every face
	//add its normal to its associated vertex
	for (int i = 0; i < shape[0].mesh.indices.size() / 3; i++) {
		idx1 = shape[0].mesh.indices[3 * i + 0];
		idx2 = shape[0].mesh.indices[3 * i + 1];
		idx3 = shape[0].mesh.indices[3 * i + 2];
		v1 = glm::vec3(shape[0].mesh.positions[3 * idx1 + 0], shape[0].mesh.positions[3 * idx1 + 1], shape[0].mesh.positions[3 * idx1 + 2]);
		v2 = glm::vec3(shape[0].mesh.positions[3 * idx2 + 0], shape[0].mesh.positions[3 * idx2 + 1], shape[0].mesh.positions[3 * idx2 + 2]);
		v3 = glm::vec3(shape[0].mesh.positions[3 * idx3 + 0], shape[0].mesh.positions[3 * idx3 + 1], shape[0].mesh.positions[3 * idx3 + 2]);
		edge1 = v2 - v1;
		edge2 = v3 - v1;
		norm = glm::cross(edge1, edge2);
		norBuf[3 * idx1 + 0] += (norm.x);
		norBuf[3 * idx1 + 1] += (norm.y);
		norBuf[3 * idx1 + 2] += (norm.z);
		norBuf[3 * idx2 + 0] += (norm.x);
		norBuf[3 * idx2 + 1] += (norm.y);
		norBuf[3 * idx2 + 2] += (norm.z);
		norBuf[3 * idx3 + 0] += (norm.x);
		norBuf[3 * idx3 + 1] += (norm.y);
		norBuf[3 * idx3 + 2] += (norm.z);
	}
	glGenBuffers(1, &norBufObjS);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);

	// Send the index array to the GPU
	const vector<unsigned int> &indBuf = shape[0].mesh.indices;
	glGenBuffers(1, &indBufObjS);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);

	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	GLSL::checkVersion();
	assert(glGetError() == GL_NO_ERROR);
}

void initGround() {

	float G_edge = 20;
	GLfloat g_backgnd_data[] = {
		-G_edge, -1.0f, -G_edge,
		-G_edge, -1.0f, G_edge,
		G_edge, -1.0f, -G_edge,
		-G_edge, -1.0f, G_edge,
		G_edge, -1.0f, -G_edge,
		G_edge, -1.0f, G_edge,
	};


	GLfloat nor_Buf_G[] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
	};

	glGenBuffers(1, &posBufObjG);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_backgnd_data), g_backgnd_data, GL_STATIC_DRAW);

	glGenBuffers(1, &norBufObjG);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
	glBufferData(GL_ARRAY_BUFFER, sizeof(nor_Buf_G), nor_Buf_G, GL_STATIC_DRAW);

}

void initGL()
{
	// Set the background color
	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);
	// Enable Z-buffer test
	glEnable(GL_DEPTH_TEST);
	glPointSize(18);

	initBunny(shapes);
	initCube(cube);
	initSopwith(sopwith);
	initGround();

	//initialize the modeltrans matrix stack
	ModelTrans.useModelViewMatrix();
	ModelTrans.loadIdentity();
	theta = 0;
}

bool installShaders(const string &vShaderName, const string &fShaderName)
{
	GLint rc;

	// Create shader handles
	GLuint VS = glCreateShader(GL_VERTEX_SHADER);
	GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shader sources
	const char *vshader = GLSL::textFileRead(vShaderName.c_str());
	const char *fshader = GLSL::textFileRead(fShaderName.c_str());
	glShaderSource(VS, 1, &vshader, NULL);
	glShaderSource(FS, 1, &fshader, NULL);

	// Compile vertex shader
	glCompileShader(VS);
	GLSL::printError();
	glGetShaderiv(VS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(VS);
	if (!rc) {
		printf("Error compiling vertex shader %s\n", vShaderName.c_str());
		return false;
	}

	// Compile fragment shader
	glCompileShader(FS);
	GLSL::printError();
	glGetShaderiv(FS, GL_COMPILE_STATUS, &rc);
	GLSL::printShaderInfoLog(FS);
	if (!rc) {
		printf("Error compiling fragment shader %s\n", fShaderName.c_str());
		return false;
	}

	// Create the program and link
	ShadeProg = glCreateProgram();
	glAttachShader(ShadeProg, VS);
	glAttachShader(ShadeProg, FS);
	glLinkProgram(ShadeProg);

	GLSL::printError();
	glGetProgramiv(ShadeProg, GL_LINK_STATUS, &rc);
	GLSL::printProgramInfoLog(ShadeProg);
	if (!rc) {
		printf("Error linking shaders %s and %s\n", vShaderName.c_str(), fShaderName.c_str());
		return false;
	}

	/* get handles to attribute data */
	h_aPosition = GLSL::getAttribLocation(ShadeProg, "aPosition");
	h_aNormal = GLSL::getAttribLocation(ShadeProg, "aNormal");
	h_uProjMatrix = GLSL::getUniformLocation(ShadeProg, "uProjMatrix");
	h_uViewMatrix = GLSL::getUniformLocation(ShadeProg, "uViewMatrix");
	h_uModelMatrix = GLSL::getUniformLocation(ShadeProg, "uModelMatrix");
	h_uLightPos = GLSL::getUniformLocation(ShadeProg, "uLightPos");
	h_uMatAmb = GLSL::getUniformLocation(ShadeProg, "UaColor");
	h_uMatDif = GLSL::getUniformLocation(ShadeProg, "UdColor");
	h_uMatSpec = GLSL::getUniformLocation(ShadeProg, "UsColor");
	h_uMatShine = GLSL::getUniformLocation(ShadeProg, "Ushine");
	h_uShadeM = GLSL::getUniformLocation(ShadeProg, "uShadeModel");
	h_uN = GLSL::getUniformLocation(ShadeProg, "uN");

	assert(glGetError() == GL_NO_ERROR);
	return true;
}


void drawGL()
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use our GLSL program
	glUseProgram(ShadeProg);

	SetProjectionMatrix();
	SetView();

	//draw thing
	SetMaterial(6);
	SetModel(glm::vec3(0, 0, 3), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjB);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjB);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	int nIndices = (int)shapes[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjB);

	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(7, 0, 7), -125);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(0, 0, 8), -180);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(-7, 0, 7), -225);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(0, 0, -8), 0);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(8, 0, 0), -90);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(7, 0, -7), -45);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(-7, 0, -7), 45);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(-8, 0, 0), 90);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(3, 0, 3), 0);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	SetModel(glm::vec3(-3, 0, 3), 0);
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	GLSL::disableVertexAttribArray(h_aPosition);
	GLSL::disableVertexAttribArray(h_aNormal);

	// Disable and unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	//draw hier1
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(0, 0, -11));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.rotate(0, glm::vec3(0, 0, 1));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier2
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(0, 0, 11));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(180, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier3
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(11, 0, 0));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(-90, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier4
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(-11, 0, 0));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(90, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier5
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(13, 0, 13));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(225, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier6
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(-13, 0, 13));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(135, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier7
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(13, 0, -13));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(-45, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier8
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(-13, 0, -13));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(45, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier9
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(3, 0, 14));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(180, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw hier10
	SetMaterial(5);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjC);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjC);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)cube[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjC);

	//create the model transforms 
	ModelTrans.loadIdentity();
	ModelTrans.scale(1, 2, 1);
	//ModelTrans.rotate(90, glm::vec3(0, 0, 1));
	ModelTrans.translate(glm::vec3(-3, 0, 13));
	//theta += 0.1;
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	//draw sop
	SetMaterial(3);
	SetModel(glm::vec3(0, 0, 1), 0);
	glUniform3f(h_uLightPos, g_light.x, g_light.y, g_light.z);
	glUniform1i(h_uShadeM, g_SM);
	glUniform1i(h_uN, g_N);

	// Enable and bind position array for drawing
	GLSL::enableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjS);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable and bind normal array for drawing
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjS);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Bind index array for drawing
	nIndices = (int)sopwith[0].mesh.indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufObjS);

	//Draw upper arm
	ModelTrans.pushMatrix();
	//ModelTrans.scale(2, 0.25, 1);
	ModelTrans.rotate(180, glm::vec3(0, 1, 0));
	ModelTrans.translate(glm::vec3(0, 1.2, 0));
	//ModelTrans.translate(glm::vec3(0, 0, -4));
	ModelTrans.pushMatrix();
	glUniformMatrix4fv(h_uModelMatrix, 1, GL_FALSE, glm::value_ptr(ModelTrans.modelViewMatrix));
	glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_INT, 0);

	GLSL::disableVertexAttribArray(h_aPosition);
	GLSL::disableVertexAttribArray(h_aNormal);

	// Disable and unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	SetMaterial(4);
	SetModel(glm::vec3(0), 0);
	//draw the ground
	glEnableVertexAttribArray(h_aPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posBufObjG);
	glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GLSL::enableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, norBufObjG);
	glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	GLSL::disableVertexAttribArray(h_aPosition);
	GLSL::disableVertexAttribArray(h_aNormal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);
	assert(glGetError() == GL_NO_ERROR);

}


void window_size_callback(GLFWwindow* window, int w, int h){
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	g_width = w;
	g_height = h;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/*if (key == GLFW_KEY_A && action == GLFW_PRESS)
		g_angle += 10;
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
		g_angle -= 10;
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
		g_anglex += 10;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		g_anglex -= 10; */
	if (key == GLFW_KEY_X && action == GLFW_PRESS)
		g_mat_id = (g_mat_id + 1) % 4;
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		g_SM = !g_SM;
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		g_light.x -= 0.25;
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		g_light.x += 0.25;
	if (key == GLFW_KEY_N && action == GLFW_PRESS)
		g_N = !g_N;
}

int main(int argc, char **argv)
{

	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	// Open a window and create its OpenGL context
	g_width = 1024;
	g_height = 768;
	window = glfwCreateWindow(g_width, g_height, "P3 - shading", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	// Initialize glad
	if (!gladLoadGL()) {
		fprintf(stderr, "Unable to initialize glad");
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}
	
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	loadShapes("eurofighter.obj", shapes);
	loadShapes("sopwith.obj", sopwith);
	loadShapes("cube.obj", cube);
	initGL();
	installShaders("vert.glsl", "frag.glsl");

	glClearColor(0.6f, 0.6f, 0.8f, 1.0f);


	do{
		drawGL();
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	glfwWindowShouldClose(window) == 0);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}