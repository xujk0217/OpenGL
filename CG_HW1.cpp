// OpenGL and FreeGlut headers.
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM.
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// C++ STL headers.
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>

// My headers.
#include "TriangleMesh.h"


// Global variables.
const int screenWidth = 600;
const int screenHeight = 600;
TriangleMesh* mesh = nullptr;
std::vector<std::string> modelNames;  // List of model names
std::string currentModel = "Bunny.obj";  // Default model name

// Function prototypes.
void SetupRenderState();
void SetupScene(const std::string&);
void ReleaseResources();
void RenderSceneCB();
void ReshapeCB(int, int);
void ProcessSpecialKeysCB(int, int, int);
void ProcessKeysCB(unsigned char, int, int);
void RenderText(const std::string&, float, float);
void ShowModelMenu(int);



// Callback function for glutDisplayFunc.
void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render the triangle mesh.
    // Add your code here.
    // ...

    if (mesh) {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->GetVboId());

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), (void*)0);

        // glDrawArrays(GL_TRIANGLES, 0, mesh->GetNumVertices()*3);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIboId());
        glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), GL_UNSIGNED_INT, nullptr);

        glDisableVertexAttribArray(0);
    }
    // Render current model name.
    RenderText("Current Model: " + currentModel, -0.9f, 0.9f);
    RenderText("Right click to change model", -0.9f, 0.85f);
    
    glutSwapBuffers();
}

void RenderText(const std::string& text, float x, float y) {
    glColor3f(1.0f, 1.0f, 1.0f); // Set text color to white
    glRasterPos2f(x, y); // Set the position for the text
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c); // Render each character
    }
}


// Callback function for glutReshapeFunc.
void ReshapeCB(int w, int h)
{
    // Adjust camera and projection here.
    // Implemented in HW2.
}

// Callback function for glutSpecialFunc.
void ProcessSpecialKeysCB(int key, int x, int y)
{
    // Handle special (functional) keyboard inputs such as F1, spacebar, page up, etc. 
    switch (key) {
    case GLUT_KEY_F1:
        // Render with point mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        break;
    case GLUT_KEY_F2:
        // Render with line mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case GLUT_KEY_F3:
        // Render with fill mode.
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    default:
        break;
    }
}

// Callback function for glutKeyboardFunc.
void ProcessKeysCB(unsigned char key, int x, int y)
{
    // Handle other keyboard inputs those are not defined as special keys.
    if (key == 27) {
        // Release memory allocation if needed.
        ReleaseResources();
        exit(0);
    }
}

void ReleaseResources()
{
    // Release memory if needed.
    // Add your code here.
    // ...
    if (mesh) {
        delete mesh;
        mesh = nullptr;
    }
}

void SetupRenderState()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
    glClearColor(
        (GLclampf)(clearColor.r), 
        (GLclampf)(clearColor.g), 
        (GLclampf)(clearColor.b), 
        (GLclampf)(clearColor.a)
    );
}

// Load a model from obj file and apply transformation.
// You can alter the parameters for dynamically loading a model.
void SetupScene(const std::string& modelPath)
{
    ReleaseResources();
    mesh = new TriangleMesh();
    mesh->LoadFromFile(modelPath);
    currentModel = modelPath;
 
    // Please DO NOT TOUCH the following code.
    // ------------------------------------------------------------------------
    // Build transformation matrices.
    // World.
    glm::mat4x4 M(1.0f);
    // Camera.
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 2.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4x4 V = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    // Projection.
    float fov = 40.0f;
    float aspectRatio = (float)screenWidth / (float)screenHeight;
    float zNear = 0.1f;
    float zFar = 100.0f;
    glm::mat4x4 P = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);

    // Apply CPU transformation.
    glm::mat4x4 MVP = P * V * M;
    
    mesh->ApplyTransformCPU(MVP);
    
    // Create and upload vertex/index buffers.
    mesh->CreateBuffers();
}

// Function to create model menu and handle selection.
void ShowModelMenu(int value) {
    if (value < 0 || value >= modelNames.size()) return;  // Invalid selection
    SetupScene("TestModels_HW1/"+modelNames[value]);  // Load the selected model
}

// Mouse callback function for right click.
void MouseButtonCB(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        glutCreateMenu(ShowModelMenu);
        for (int i = 0; i < modelNames.size(); i++) {
            glutAddMenuEntry(modelNames[i].c_str(), i);
        }
        glutAttachMenu(GLUT_RIGHT_BUTTON);
    }
}

// Function to load model filenames from a directory.
void LoadModelNames(const std::string& directory) {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.path().extension() == ".obj") {
            modelNames.push_back(entry.path().filename().string());
        }
    }
}

int main(int argc, char** argv)
{
    LoadModelNames("TestModels_HW1");

    // Setting window properties.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("HW1: OBJ Loader");

    // Initialize GLEW.
    // Must be done after glut is initialized!
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        std::cerr << "GLEW initialization error: " 
                  << glewGetErrorString(res) << std::endl;
        return 1;
    }

    // Initialization.
    SetupRenderState();
    currentModel = modelNames[0];
    SetupScene("TestModels_HW1/" + currentModel);

    // Register callback functions.
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
    glutReshapeFunc(ReshapeCB);
    glutSpecialFunc(ProcessSpecialKeysCB);
    glutKeyboardFunc(ProcessKeysCB);
    glutMouseFunc(MouseButtonCB);

    // Start rendering loop.
    glutMainLoop();

    return 0;
}

