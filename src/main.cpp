#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>

#include "camera.h"
#include "glm/detail/type_mat.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "model.h"
#include "shader.h"
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.3f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX {SCREEN_WIDTH / 2.f};
float lastY {SCREEN_HEIGHT / 2.f};
bool firstMouse = true;

float deltaTime{};
float lastFrame{};

glm::vec3 lightPosition(1.2f, 1.0f, 2.0f);

namespace fs = std::filesystem;
//projectRoot assumes build was compiled from cmake-build-debug, which is not ideal
//however the fix involves a decent amount of hackish code and im the only one running this
//stuff anyway so im leaving it for now
fs::path projectRoot = fs::current_path().parent_path(); //mehhhh
fs::path srcRoot = projectRoot / "src";

int main(int argc, char *argv[]) {
// ======================GLAD+GLFW INITIALIZATION==============================
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = 
    glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Triangle", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  //glad tings
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  stbi_set_flip_vertically_on_load(true);

  glEnable(GL_DEPTH_TEST);
  //=============================SHADER INITIALIZATION===========================
 
  //TODO: get a better file management system (please)
  fs::path shaderRoot = srcRoot / "shaders";
  Shader backpackShader((shaderRoot / "backpack" / "vertex.glsl").c_str(),
                        (shaderRoot / "backpack" / "fragment.glsl").c_str());
  fs::path modelRoot = projectRoot / "models";
  Model backpack(modelRoot / "backpack" / "backpack.obj");

  //enable polygon mode
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  
// =============================RENDERING LOOP=================================
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    process_input(window);

    // rendering commands
    glClearColor(0.05f, 0.05, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    backpackShader.use();

    glm::mat4 projection = 
      glm::perspective(glm::radians(camera.Zoom),
                       (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT,
                       0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    backpackShader.setMat4("projection", projection);
    backpackShader.setMat4("view", view);

    // render the loaded model
    glm::mat4 model = glm::mat4(1.0f);
    // translate it down so it's at the center of the scene
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));  
    // it's a bit too big for our scene, so scale it down
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	
    backpackShader.setMat4("model", model);

    backpackShader.setVec3("viewerPos", camera.Position);

    backpackShader.setVec3("pointlight.position", lightPosition);
    backpackShader.setFloat("pointlight.constant", 1.0f);
    backpackShader.setFloat("pointlight.linear", 0.09f);
    backpackShader.setFloat("pointlight.quadratic", 0.032f);
    backpackShader.setVec3("pointlight.ambient", glm::vec3(0.05f));
    backpackShader.setVec3("pointlight.diffuse", glm::vec3(0.8f));
    backpackShader.setVec3("pointlight.specular", glm::vec3(1.f));

    backpack.Draw(backpackShader);

    // check + call events & swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
 
  //movement
  const float cameraSpeed{2.5f * deltaTime};
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    camera.ProcessKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    camera.ProcessKeyboard(RIGHT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    camera.ProcessKeyboard(LEFT, deltaTime);
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
  if (firstMouse){
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;
  
  camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
  camera.ProcessMouseScroll(yoffset);
}

