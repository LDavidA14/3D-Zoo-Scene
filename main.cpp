#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "SkyBox.hpp"

int glWindowWidth = 1920;
int glWindowHeight = 1001;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(0.0f, 2.0f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.05f;

bool pressedKeys[1024];



gps::Model3D Static_Zoo;
gps::Model3D LeftGate;
gps::Model3D RightGate;
gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

int shiftOn = 0;
bool ViewScene = false;

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

float lastX = 320;
float lastY = 240;
float yaw = -90;
float pitch = 0;
bool firstMouse = true;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.01f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;
	
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	myCamera.rotate(pitch, yaw);
}

GLfloat LeftGateAngle = 90;
GLfloat RightGateAngle = 92.5;
bool OpenGates = false;
bool CloseGates = true;

float x= 0;
float y= 0;
float z= 0; /// Pentru view Scene automat

void processMovement()
{	
	if (pressedKeys[GLFW_KEY_Z]) {
		shiftOn = 1;
	}
	else {
		shiftOn = 0;
	}
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed, shiftOn);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed, shiftOn);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed, shiftOn);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed,shiftOn);
	}
	//SOLID
	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	//WIREFRAME
	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	//POINT
	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	// Gates
	if (pressedKeys[GLFW_KEY_G]) {
		OpenGates = true;
	}
	if (pressedKeys[GLFW_KEY_H]) {
		CloseGates = true;
	}
	// Moving Camera
	if (pressedKeys[GLFW_KEY_4]) {
		 ViewScene = true;
		  x = -10.746698;
		  y = -0.658498;
		  z = 6.488187;
	}
	if (pressedKeys[GLFW_KEY_5]) {
		myCamera.printCameraPosition();
	}

}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
    Static_Zoo.LoadModel("objects/Static_Zoo.obj");
	LeftGate.LoadModel("objects/Gate/LeftGate.obj");
	RightGate.LoadModel("objects/Gate/RightGate.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");  //shadere pentru obiecte din blender
	myCustomShader.useShaderProgram();  
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");   //shadere pentru cub lumina 
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");  //shadere tasta M
    screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag"); //shadere pentru umbra
	depthMapShader.useShaderProgram();
}

void initUniforms() {

	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));


	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light 
	lightDir = glm::vec3(8.502719,3.461945,-7.512266);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view )) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	float a = 24.0f;
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = -a, far_plane = a;
	glm::mat4 lightProjection = glm::ortho(-a, a, -a, a, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	//-----------------------------------------------------   STATIC ZOO

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	
	Static_Zoo.Draw(shader);


	//----------------------------------------------------- LEFT GATE
	
	if (OpenGates == true && LeftGateAngle <173) {
		LeftGateAngle++;
		RightGateAngle++;
		CloseGates == false;
	}
	else if (LeftGateAngle == 173) {
		OpenGates = false;
	}
    if (CloseGates ==true && LeftGateAngle >90 && OpenGates==false) {
		RightGateAngle--;
		LeftGateAngle--;
	}
	else if (LeftGateAngle == 90) {
		CloseGates = false;
	}

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-7.595f, -1.0f, 6.058f));
	model = glm::scale(model, glm::vec3(0.17f, 0.17f, 0.17f));
	model = glm::rotate(model, glm::radians(LeftGateAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth mapS
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	LeftGate.Draw(shader);

	//----------------------------------------------------- RIGHT GATE
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-7.598f, -0.9999f, 6.835f));
	model = glm::scale(model, glm::vec3((0.172f, 0.172f, 0.172f)));
	model = glm::rotate(model, glm::radians(-RightGateAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth mapS
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	RightGate.Draw(shader);
 
	
}

int i = 0;
int aux = 0;
float highSpeed = 0.04f;//0.025f;
float slowMotionSpeed=0.01f;
float speedForSceneView = highSpeed; 

void renderScene() {
	if (ViewScene == true) {
		myCamera.setCameraPosition(glm::vec3(x, y, z));
		if (x < -5.199625 && aux==0 ) {  //Entering ZOO
			x += speedForSceneView;
			myCamera.move(gps::MOVE_FORWARD, speedForSceneView, 0);
			if ((x > -9.2) && (i == 0)) {
				OpenGates = true;
				i = 1;
			}
			if (x > -5.8 && i == 1) {
				CloseGates = true;
				i = 2;
			}
		}
		else if(aux==0) {
			aux = 1;
		}
		if (z > -5.8 && aux ==1) {    // MILKA 
			if(z > 3.856777 && z < 6.041224 ) speedForSceneView = slowMotionSpeed;
			else {
				speedForSceneView = highSpeed;
			}
			z -= speedForSceneView;
			myCamera.move(gps::MOVE_LEFT, speedForSceneView, 0);
			if (x < -4.8) {
				x += speedForSceneView;
				myCamera.move(gps::MOVE_FORWARD, speedForSceneView, 0);
			}
		}
		else if(aux == 1) {
			aux = 2;
		}
	    if (x < -2.788504 && aux==2) {   // Monkey - Rabbit
			x += speedForSceneView;
				myCamera.move(gps::MOVE_FORWARD, speedForSceneView, 0);
		}
		else if (aux == 2) {
			
			aux = 3;
		}
		if (z < 5.213937 && aux==3) {   //Bears - Rhino
			z += speedForSceneView;
			myCamera.move(gps::MOVE_RIGHT, speedForSceneView, 0);
		}
		else if (aux == 3) {
			aux = 4;
		}
		if (x < 0.566905 && aux == 4) { //Rhino - Pingu
			x += speedForSceneView;
			myCamera.move(gps::MOVE_FORWARD, speedForSceneView, 0);
		}
		else if (aux == 4) {
			aux = 5;
		}
		if (z > 0.095745 && aux == 5) {  // Pingu -Elephant
			if (z > 4.0f) speedForSceneView = slowMotionSpeed;
			else {
				speedForSceneView = highSpeed;
			}
			z -= speedForSceneView;
			myCamera.move(gps::MOVE_LEFT, speedForSceneView, 0);
		}
		else if (aux == 5) {
			aux = 6;
		}
		if (z > -3.810913 && aux == 6) { // Elephant - Bisons
			if (z < 1.300628 && z > -0.88) speedForSceneView = slowMotionSpeed;
			else {
				speedForSceneView = highSpeed;
			}
			z -= speedForSceneView;
			myCamera.move(gps::MOVE_LEFT, speedForSceneView, 0);
			if (x < 3.333930) {
				x += speedForSceneView;
				myCamera.move(gps::MOVE_FORWARD, speedForSceneView, 0);
			}
		}
		else if (aux == 6) {

			aux = 7;
		}
		if (z < -1.898528 && aux == 7) { // Bisons- Deer
			z += speedForSceneView;
			myCamera.move(gps::MOVE_RIGHT, speedForSceneView, 0);
			if (x < 3.910546) {
				x += speedForSceneView;
				myCamera.move(gps::MOVE_FORWARD, speedForSceneView, 0);
			}
		}
		else if (aux == 7) {
			aux = 8;
		}
		if (z < 1.798850 && aux == 8) { // Deer
			z += speedForSceneView;
			myCamera.move(gps::MOVE_RIGHT, speedForSceneView, 0);
			if (x < 4.012312) {
				x += speedForSceneView;
				myCamera.move(gps::MOVE_FORWARD, speedForSceneView, 0);
			}
		}
		else if (aux == 8) {

			aux = 9;
		}
		if (z < 4.129407 && aux == 9) { // Deer - Raccoons
			z += speedForSceneView;
			myCamera.move(gps::MOVE_RIGHT, speedForSceneView, 0);
			if (x < 5.524299) {
				x += speedForSceneView;
				myCamera.move(gps::MOVE_FORWARD, speedForSceneView, 0);
			}
		}
		else if (aux == 9) {

			aux = 10;
		}
		if (z < 6.683823 && aux == 10) { // Racoons
			z += speedForSceneView;
			myCamera.move(gps::MOVE_RIGHT, speedForSceneView, 0);
			if (x > 5.096613) {
				x -= speedForSceneView;
				myCamera.move(gps::MOVE_BACKWARD, speedForSceneView, 0);
			}
		}
		else if (aux == 10) {

			aux = 11;
		}
		if(aux==11) {
			aux = 0;
			i = 0;
			ViewScene = false;
			
		}
	}
	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, retina_width, retina_height);

	glClear(GL_COLOR_BUFFER_BIT);


	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		
		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();
	
		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));


		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view )) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));
		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		drawObjects(myCustomShader, false);
		
	}
	
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {
	
	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	glCheckError();
	
	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
