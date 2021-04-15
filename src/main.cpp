#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <random>
#include <time.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Material.h"
#include "Light.h"
#include "Object.h"
#include "Bunny.h"
#include "Teapot.h"
#include "Ground.h"
#include "Sphere.h"
#include "Revolution.h"

using namespace std;

GLFWwindow* window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;
shared_ptr<double> t;

shared_ptr<Camera> camera;
shared_ptr<Program> prog, revProg, pass2Prog, splitProg;
shared_ptr<MatrixStack> MV;
shared_ptr<MatrixStack> P;
shared_ptr<Shape> bunny, teapot, sphere, ground_mesh, rev, screen;
shared_ptr<Object> ground;

vector<shared_ptr<Object>> bunnies; // contains all objects to be drawn
vector<shared_ptr<Object>> revs; // contains surfaces of revolution to be drawn

// lights
vector<shared_ptr<Light>> lights; // contains all lights to be drawn
int nLights = 10;
glm::vec3* lightPos;
glm::vec3* lightColors;

// textures
int activeTex = 0;
int textureWidth = 640 * 3;
int textureHeight = 480 * 3;
GLuint framebufferID;
GLuint posTexture;
GLuint norTexture;
GLuint kaTexture;
GLuint kdTexture;


bool keyToggles[256] = { false }; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char* description) {
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse) {
	camera->mouseMoved((float)xmouse, (float)ymouse);
}

static void char_callback(GLFWwindow* window, unsigned int key) {
	keyToggles[key] = !keyToggles[key];
	switch (key) {
	case '0':
		activeTex = 0;
		break;
	case '1':
		activeTex = 1;
		break;
	case '2':
		activeTex = 2;
		break;
	case '3':
		activeTex = 3;
		break;
	case '4':
		activeTex = 4;
		break;
	case 'w':
		camera->w();
		break;
	case 'a':
		camera->a();
		break;
	case 's':
		camera->s();
		break;
	case 'd':
		camera->d();
		break;
	case 'z':
		camera->addFovy(-0.1f);
		break;
	case 'Z':
		camera->addFovy(0.1f);
		break;
	case 'p':
		// disable pointer and use raw mouse motion
		if (keyToggles[(unsigned)'p']) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		break;
	}
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	textureWidth = width;
	textureHeight = height;

	glBindTexture(GL_TEXTURE_2D, posTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, norTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, kaTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, kdTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);

	
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char* filepath, GLFWwindow* w) {
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if (rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

// This function is called once to initialize the scene and OpenGL
static void init() {
	// Initialize time.
	glfwSetTime(0.0);
	srand((unsigned)time(NULL));
	t = make_shared<double>();
	*t = 0.0;


	// Set background color.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);
	/////////////////////
	// Set up programs //
	/////////////////////
	// first pass
	prog = make_shared<Program>();
	prog->setShaderNames(RESOURCE_DIR + "bp_vert.glsl", RESOURCE_DIR + "Pass1_frag.glsl");
	prog->setVerbose(true);
	prog->init();
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->addUniform("MV");
	prog->addUniform("IT");
	prog->addUniform("P");
	prog->addUniform("ka");
	prog->addUniform("kd");
	prog->setVerbose(false);
	// surface of revolution program (1st pass)
	revProg = make_shared<Program>();
	revProg->setShaderNames(RESOURCE_DIR + "rev_vert.glsl", RESOURCE_DIR + "Pass1_frag.glsl");
	revProg->setVerbose(true);
	revProg->init();
	revProg->addAttribute("aPos");
	revProg->addUniform("MV");
	revProg->addUniform("IT");
	revProg->addUniform("P");
	revProg->addUniform("ka");
	revProg->addUniform("kd");
	revProg->addUniform("t");
	revProg->setVerbose(false);
	// second pass program
	pass2Prog = make_shared<Program>();
	pass2Prog->setShaderNames(RESOURCE_DIR + "Pass2_vert.glsl", RESOURCE_DIR + "Pass2_frag.glsl");
	pass2Prog->setVerbose(true);
	pass2Prog->init();
	pass2Prog->addAttribute("aPos");
	pass2Prog->addUniform("MV");
	pass2Prog->addUniform("P");
	pass2Prog->addUniform("windowSize");

	pass2Prog->addUniform("posTexture");
	pass2Prog->addUniform("norTexture");
	pass2Prog->addUniform("kaTexture");
	pass2Prog->addUniform("kdTexture");
	pass2Prog->addUniform("lightPos");
	pass2Prog->addUniform("lightColors");
	pass2Prog->bind();
	glUniform1i(pass2Prog->getUniform("posTexture"), 0);
	glUniform1i(pass2Prog->getUniform("norTexture"), 1);
	glUniform1i(pass2Prog->getUniform("kaTexture"), 2);
	glUniform1i(pass2Prog->getUniform("kdTexture"), 3);
	pass2Prog->unbind();
	pass2Prog->setVerbose(false);
	// split rendering program
	splitProg = make_shared<Program>();
	splitProg->setShaderNames(RESOURCE_DIR + "Pass2_vert.glsl", RESOURCE_DIR + "split_frag.glsl");
	splitProg->setVerbose(true);
	splitProg->init();
	splitProg->addAttribute("aPos");
	splitProg->addUniform("MV");
	splitProg->addUniform("P");
	splitProg->addUniform("windowSize");

	splitProg->addUniform("posTexture");
	splitProg->addUniform("norTexture");
	splitProg->addUniform("kaTexture");
	splitProg->addUniform("kdTexture");
	splitProg->addUniform("activeTex");
	splitProg->bind();
	glUniform1i(splitProg->getUniform("posTexture"), 0);
	glUniform1i(splitProg->getUniform("norTexture"), 1);
	glUniform1i(splitProg->getUniform("kaTexture"), 2);
	glUniform1i(splitProg->getUniform("kdTexture"), 3);
	splitProg->unbind();
	splitProg->setVerbose(false);
	///////////////////
	// Create shapes //
	///////////////////

	camera = make_shared<Camera>();
	double cursorx, cursory;
	glfwGetCursorPos(window, &cursorx, &cursory);
	camera->setMousePrev((float)cursorx, (float)cursory);

	bunny = make_shared<Shape>();
	bunny->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunny->init();
	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();
	sphere = make_shared<Shape>();
	sphere->loadSphere();
	sphere->init();
	rev = make_shared<Shape>();
	rev->loadRev();
	rev->init();
	ground_mesh = make_shared<Shape>();
	ground_mesh->loadMesh(RESOURCE_DIR + "ground.obj");
	ground_mesh->init();
	screen = make_shared<Shape>();
	screen->loadPlane();
	screen->init();

	///////////////////////////
	// Create render buffers //
	///////////////////////////
	glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	// position buffer
	glGenTextures(1, &posTexture);
	glBindTexture(GL_TEXTURE_2D, posTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, posTexture, 0);
	// normal buffer
	glGenTextures(1, &norTexture);
	glBindTexture(GL_TEXTURE_2D, norTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, norTexture, 0);
	// ka buffer
	glGenTextures(1, &kaTexture);
	glBindTexture(GL_TEXTURE_2D, kaTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, kaTexture, 0);
	// kd buffer
	glGenTextures(1, &kdTexture);
	glBindTexture(GL_TEXTURE_2D, kdTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, kdTexture, 0);
	// depth test in pass 1
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	// tell opengl that we want 2 textures as output of framebuffer
	GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);
	// make sure done correctly
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		cerr << "Framebuffer is not ok" << endl;
	}
	// set back to on screen buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);

	////////////////////
	// Create objects //
	////////////////////

	// create matrix stacks
	P = make_shared<MatrixStack>();
	MV = make_shared<MatrixStack>();

	// create objects to be drawn
	glm::vec3 groundMax = glm::vec3(5.0f, 0.0f, 5.0f);
	glm::vec3 groundMin = glm::vec3(-5.0f, 0.0f, -5.0f);
	glm::vec3 groundSize = groundMax - groundMin;
	int num_rows = 11;
	int num_cols = 11;
	groundSize.x /= num_rows;
	groundSize.z /= num_cols;
	for (int x = 1; x < num_rows; x++) {
		for (int z = 1; z < num_cols; z++) {
			if ((x + z) % 4 == 0) { // bunny
				glm::vec3 pos = glm::vec3(groundSize.x * x + groundMin.x, 0.0f, groundSize.z * z + groundMin.x);
				bunnies.push_back(make_shared<Bunny>(pos, glm::vec3(0.0f, -0.333099f, 0.0f), bunny, MV));
			} else if ((x + z) % 4 == 1) { // teapot
				glm::vec3 pos = glm::vec3(groundSize.x * x + groundMin.x, 0.0f, groundSize.z * z + groundMin.x);
				bunnies.push_back(make_shared<Teapot>(pos, glm::vec3(0.0f), teapot, MV));
			} else if ((x + z) % 4 == 2) { // sphere
				glm::vec3 pos = glm::vec3(groundSize.x * x + groundMin.x, 0.0f, groundSize.z * z + groundMin.x);
				bunnies.push_back(make_shared<Sphere>(pos, glm::vec3(0.0f, 1.0f, 0.0f), sphere, MV));
			} else { // surface of revolution
				glm::vec3 pos = glm::vec3(groundSize.x * x + groundMin.x, 0.0f, groundSize.z * z + groundMin.x);
				revs.push_back(make_shared<Revolution>(pos, glm::vec3(0.0f), rev, MV));
				revs.back()->addAnimation(t);
			}
			bunnies.back()->addAnimation(t);
		}
	}

	// create lights
	lightPos = new glm::vec3[nLights];
	lightColors = new glm::vec3[nLights];
	for (int i = 0; i < nLights; i++) {
		lights.push_back(make_shared<Light>(glm::vec3(1.0f * (i - nLights / 2), 0.25f, 1.0f * (i - nLights / 2)), &lightPos[i], &lightColors[i], sphere, MV));
		lights.back()->addPolar(0.5f * i, i * (6.28f / nLights));
		lights.back()->addAnimation(t);
	}

	// create ground
	ground = make_shared<Ground>(ground_mesh, MV);

}

// This function is called every frame to draw the scene.
static void render() {
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}


	// activate and bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, posTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, norTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, kaTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, kdTexture);

	//screen->draw(pass2Prog);
	glActiveTexture(GL_TEXTURE0);


	*t = glfwGetTime();
	if (keyToggles[(unsigned)' ']) {
		// Spacebar turns animation on/off
		*t = 0.0f;
	}

	// Apply camera transforms
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = (float)width / (float)height;
	camera->setAspect(aspect);
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
	camera->applyViewMatrix(MV);

	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	glViewport(0, 0, textureWidth, textureHeight);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	prog->bind();
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	// refresh posCam to keep in camera space
	for (int i = 0; i < nLights; i++) {
		lights.at(i)->refreshPosCam();
	}
	// Draw every object
	for (int i = 0; i < bunnies.size(); i++) {
		bunnies.at(i)->draw(prog);
	}
	for (int i = 0; i < lights.size(); i++) {
		lights.at(i)->draw(prog);
	}
	ground->draw(prog);
	prog->unbind();

	// draw surfaces of revolution
	revProg->bind();
	glUniformMatrix4fv(revProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	// Draw every rev
	for (int i = 0; i < revs.size(); i++) {
		revs.at(i)->draw(revProg);
	}
	revProg->unbind();

	MV->popMatrix();
	P->popMatrix();

	////////////////////////
	// Second pass render //
	////////////////////////
	auto activeProg = pass2Prog;
	if (activeTex != 0)
		activeProg = splitProg;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Get current frame buffer size.
	glViewport(0, 0, width, height);
	activeProg->bind();
	// Apply camera transforms
	P->pushMatrix();
	P->multMatrix(glm::ortho(-0.5, 0.5, -0.5, 0.5, 1.0, 10.0));
	MV->pushMatrix();
	MV->translate(0.0f, 0.0f, -5.0f);
	glUniformMatrix4fv(activeProg->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniformMatrix4fv(activeProg->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
	glUniform2f(activeProg->getUniform("windowSize"), (float)width, (float)height);
	if (activeTex == 0) {
		glUniform3fv(activeProg->getUniform("lightPos"), nLights, glm::value_ptr(*lightPos));
		glUniform3fv(activeProg->getUniform("lightColors"), nLights, glm::value_ptr(*lightColors));
	} else {
		glUniform1i(activeProg->getUniform("activeTex"), activeTex);
	}
	screen->draw(activeProg);
	glActiveTexture(GL_TEXTURE0);

	activeProg->unbind();
	P->popMatrix();
	MV->popMatrix();



	GLSL::checkError(GET_FILE_LINE);

	if (OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		cout << "Usage: A3 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");

	// Optional argument: offline rendering
	if (argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// optional argument: active texture
	if (argc >= 4) {
		activeTex = atoi(argv[3]);
		if (activeTex < 0) activeTex = 0;
		else if (activeTex > 4) activeTex = 4;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if (!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	// default is 640 x 480
	window = glfwCreateWindow(640 * 3, 480 * 3, "A5 - Cole Downey", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while (!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
