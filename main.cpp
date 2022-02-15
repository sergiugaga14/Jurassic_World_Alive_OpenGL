#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

float cubeAngle = 0.0f;
float pitch=0.0f, yaw =90.0f;
GLboolean cameraMovement = true;
GLboolean firstMouse = true;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
GLfloat lastX = 400;
GLfloat lastY = 300;


// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::mat4 lightRotation;


//fog
float fogDensity;
GLint fogDensityLoc;

//spotlight
int initSpot=-1;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;


// camera
gps::Camera myCamera(
    glm::vec3(40.0f, 20.0f, 0.0f),
    glm::vec3(-10.0f, 0.0f, 55.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.5f;
GLfloat angle;
GLboolean pressedKeys[1024];

// models
gps::Model3D scene;
gps::Model3D pteranodon;
gps::Model3D asteroid;
gps::Model3D lightCube;


// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;
gps::Shader lightShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;

float lightAngle=0.0f; 


GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
    
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 1.0f;
       
    }

    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle += 1.0f;
      
    }
    if (pressedKeys[GLFW_KEY_V]) {
        
        lightDir += glm::vec3(0.0f, 1.0f, 0.0f);
    }

    if (pressedKeys[GLFW_KEY_B]) {
        
        lightDir += glm::vec3(0.0f, -1.0f, 0.0f);
    }
	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_T)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (glfwGetKey(window, GLFW_KEY_Y)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (glfwGetKey(window, GLFW_KEY_U)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (pressedKeys[GLFW_KEY_F]) {
        fogDensity += 0.001f;
        myBasicShader.useShaderProgram();
        glUniform1f(fogDensityLoc, fogDensity);
    }
    if (pressedKeys[GLFW_KEY_G]) {
        fogDensity -= 0.001f;
        myBasicShader.useShaderProgram();
        glUniform1f(fogDensityLoc, fogDensity);
    }
    if (pressedKeys[GLFW_KEY_P]) {
        initSpot *= -1;
        if (initSpot == 1)
        {
            lightColor = glm::vec3(0.05f, 0.05f, 0.05f);
        }
        else
            lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        myBasicShader.useShaderProgram();
        glm::vec3 spotLightDirection = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 spotLightPosition = glm::vec3(6.0f, 5.0f, 10.0f);

        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDir"), 1, glm::value_ptr(spotLightDirection));
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPos"), 1, glm::value_ptr(spotLightPosition));

        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "initSpot"), initSpot);
    }
}
void updateMouseRotation()
{
    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //TODO
    float sensitivity = 0.05f;

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    yaw+= xpos - lastX;
    pitch+= lastY - ypos;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    lastX = xpos;
    lastY = ypos;

    updateMouseRotation();
    
}

void processMovement() {
   
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}
    
    if (pressedKeys[GLFW_KEY_Z]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (pressedKeys[GLFW_KEY_X]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }


    if (pressedKeys[GLFW_KEY_Q]) {
    
        yaw -= 1;
        myCamera.rotate(pitch, yaw);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {  
        yaw += 1;
        myCamera.rotate(pitch, yaw);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for teapot
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    //fog
   
    //spotLight
    
    if (pressedKeys[GLFW_KEY_M]) {
        cameraMovement = false;   
    }
    if (pressedKeys[GLFW_KEY_N]) {
        cameraMovement = true;
    }



}

void initOpenGLWindow() {
    myWindow.Create(1980, 1024, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    scene.LoadModel("models/scene2.obj");
    pteranodon.LoadModel("models/pterano.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    asteroid.LoadModel("models/astero.obj");
}

void initShaders() {
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	myBasicShader.loadShader("shaders/basic.vert","shaders/basic.frag");
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
  
}
void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
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

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
        0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 175.0f;
    glm::mat4 lightProjection = glm::ortho(-205.0f, 205.0f, -205.0f, 205.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;

}

void initUniforms() {
	myBasicShader.useShaderProgram();
   
    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 5000.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
    lightAngle = 0.0f;
	lightDir = glm::vec3(0.0f, 150.0f, 1.0f);
    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));


	//set light color
	 //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
  
    //fog
    fogDensity = 0.0f;
    fogDensityLoc= glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1f(fogDensityLoc, fogDensity);

   
}

void renderScene(gps::Shader shader,bool shadow) {
    // select active shader program
    shader.useShaderProgram();
    //send teapot model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    //send scene normal matrix data to shader
    if (!shadow)
    {
       // normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
 
    // draw scene
   
   scene.Draw(shader);
   
}
void renderPteranodon(gps::Shader shader,bool shadow) {
    // select active shader program
    shader.useShaderProgram();   
        // model2 = glm::translate(model2, glm::vec3(0.0f, 10.0f, -20.0f));
        glm::mat4 model2 = glm::rotate(glm::mat4(1.0f), glm::radians(cubeAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model2 = glm::translate(model2, glm::vec3(1.0f, 45.0f, 85.0f));
        model2 = glm::rotate(model2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model2));

    //send  normal matrix data to shader

      if (!shadow)
        {
            //normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        }
     
        pteranodon.Draw(shader);

}
void renderAsteroid(gps::Shader shader, bool shadow) {
    // select active shader program
    shader.useShaderProgram();
 
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader

    if (!shadow)
    {
        //normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    asteroid.Draw(shader);

}
void renderLightCube(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send lightCube model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 model = lightRotation;
   
    model = glm::translate(model, 1.0f * (lightDir));
    model = glm::translate(model, glm::vec3(-100,0,0));
    model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    lightCube.Draw(shader);

}


void bind()
{
    myBasicShader.useShaderProgram();

    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

    //bind the shadow map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));

    renderScene(myBasicShader, false);
    renderPteranodon(myBasicShader, false);
   
}
void renderScene() {
	
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderScene(depthMapShader, true);
    renderPteranodon(depthMapShader, true);
   
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



   glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   bind();
   if (initSpot == 1)
       renderAsteroid(myBasicShader, false);
   renderLightCube(lightShader);
    }



void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}
void cameraAnimation()
{
    myCamera.move(gps::MOVE_FORWARD, cameraSpeed/3);
    //update view matrix
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    yaw -= 0.33f;
    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}
int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms();
    initFBO();
    setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        cubeAngle+=0.3f;
        if(cameraMovement)
        cameraAnimation();
         
        processMovement();
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();
    return EXIT_SUCCESS;
}
