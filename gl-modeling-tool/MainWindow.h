//
//  MainWindow.h
//  gl-modeling-tool
//
//  Created by Melih Mete on 17.02.2019.
//  Copyright © 2019 Melih Mete. All rights reserved.
//

#ifndef MainWindow_h
#define MainWindow_h
#define GLFW_STATIC

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Cube.h"
#include "Triangle.h"
#include "Shaders.h"
#include "Polygon.h"
#include "Camera.h"

GLfloat lastX;
GLfloat lastY;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
bool keys[1024];
bool firstMouse = false;
Camera *camera = new Camera(glm::vec3(0.0f, 0.0f, -10.0f));
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
float unit = 3.0f;
float movementSpeed = 0.5f;
float velocity;

void DoMovement( )
{
    // Camera controls
    
    if( keys[GLFW_KEY_W] ) {
        camera->ProcessKeyboard( FORWARD, deltaTime );
    }
    
    if( keys[GLFW_KEY_S]  ) {
        camera->ProcessKeyboard( BACKWARD, deltaTime );
    }
    
    if( keys[GLFW_KEY_A] ) {
        camera->ProcessKeyboard( LEFT, deltaTime );
    }
    
    if( keys[GLFW_KEY_D] ) {
        camera->ProcessKeyboard( RIGHT, deltaTime );
    }
}

// Is called whenever a key is pressed/released via GLFW
static void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode )
{
    if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    if ( key >= 0 && key < 1024 )
    {
        if( action == GLFW_PRESS )
        {
            keys[key] = true;
        }
        else if( action == GLFW_RELEASE )
        {
            keys[key] = false;
        }
    }
}

class MainWindow {

public:
    
    MainWindow() {
        windowsHeigth = 1600;
        windowsWidth  = 2560;
        programID = 0;
        
    }
    
    ~MainWindow() { }
    
    static void ScrollCallback( GLFWwindow *window, double xOffset, double yOffset )
    {
        camera->ProcessMouseScroll( yOffset );
    }
    
    static void MouseCallback( GLFWwindow *window, double xPos, double yPos )
    {
        if( firstMouse )
        {
            lastX = xPos;
            lastY = yPos;
            firstMouse = false;
        }
        
        GLfloat xOffset = xPos - lastX;
        GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left
        
        lastX = xPos;
        lastY = yPos;
        
        camera->ProcessMouseMovement( xOffset, yOffset );
    }
    
    void Init() {
        camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
        lastX = windowsWidth / 2;
        lastY = windowsHeigth / 2;
        firstMouse = false;
        deltaTime = 0.0f;
        lastFrame = 0.0f;
        
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        
        if(glfwInit()) {
            std::cout << "Initiated GLFW" << std::endl;
        }
        
        window = glfwCreateWindow(windowsWidth, windowsHeigth, "Modeling Tool", NULL, NULL);
        if(window == NULL) {
            std::cout << "Error creating on windows." << std::endl;
        }
        
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, MouseCallback);
        glfwSetScrollCallback(window, ScrollCallback);
        
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetFramebufferSize(window, &windowsWidth, &windowsHeigth);
        glfwMakeContextCurrent( window );
        
        glewExperimental = GL_TRUE;
        if (glewInit () != GLEW_NO_ERROR) {
            std::cout << "Failed to initialize GLEW... " << std::endl;
            return;
        }
        
        glViewport(0.0f, 0.0f, windowsWidth, windowsHeigth);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, windowsWidth, 0, windowsHeigth, 0, 1000);
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity( );
        
        model = glm::mat4(1.0f);
        view = glm::mat4(1.0f);
        projection = glm::mat4(1.0f);
        mvp = glm::mat4(1.0f);
    }

    void Run() {
        
        polygon = new Polygon();
        polygon->Init();
        
        shader = new Shaders();
        shader->Init();
        programID = shader->GetShaderID();
        glUseProgram(programID);
        
        while (!glfwWindowShouldClose(window)) {
            
            // Calculating deltaTime to disable performance differences
            GLfloat currentFrame = glfwGetTime( );
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            
            glfwPollEvents();
            DoMovement();
            
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glClearColor(0.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f, 1.0f);
            
            // Calculating model, view & projection matrices
            DoModelTranslation(model);
            view  = camera->GetViewMatrix();
            projection = glm::perspective(camera->GetZoom(), (float) windowsWidth / (float) windowsHeigth, 0.1f, 1000.0f);
            mvp = projection * view * model;
            
            //Setting the last matrix on shader object.
            shader->EditMatrix4("mvp", mvp);
            
            // Drawing polygon
            polygon->Draw();
            
            // Swaping buffers to render objects
            glfwSwapBuffers(window);
            
        }
        
        delete polygon;
        delete shader;
        delete camera;
        
        glfwTerminate();
    }
    
    void DoModelTranslation(glm::mat4& modelMatrix) {
        if( keys[GLFW_KEY_UP] ) {
            polygon->CalculateTranslation( 1, modelMatrix, deltaTime );
        }
        
        if( keys[GLFW_KEY_DOWN]  ) {
            polygon->CalculateTranslation( 2, modelMatrix, deltaTime );
        }
        
        if( keys[GLFW_KEY_LEFT] ) {
            polygon->CalculateTranslation( 3, modelMatrix, deltaTime );
        }
        
        if( keys[GLFW_KEY_RIGHT] ) {
            polygon->CalculateTranslation( 4, modelMatrix, deltaTime );
        }
        
        if (false) {
            polygon->CalculateTranslation(5, modelMatrix, deltaTime);
        }
    }
    
private:
    int windowsHeigth = 1080;
    int windowsWidth = 1920;
    GLFWwindow *window;
    Shaders *shader;
    GLuint programID;
    Polygon* polygon;
    glm::mat4 mvp;
    
};

#endif /* MainWindow_h */
