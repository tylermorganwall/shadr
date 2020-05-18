//glew Installed make install 
#include <GL/glew.h>
//GLWF3 Installed with cmake, make install 
#include <GLFW/glfw3.h>

// Include GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "controls.h"


glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
  return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
  return ProjectionMatrix;
}


// Initial position : on +Z
glm::vec3 position = { 0, 0, 5 }; 
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 10.0f; // 3 units / second
float mouseSpeed = 0.005f;


namespace glm {
void computeMatricesFromInputs(GLFWwindow* window){
  
  // glfwGetTime is called only once, the first time this function is called
  static double lastTime = glfwGetTime();
  
  // Compute time difference between current and last frame
  double currentTime = glfwGetTime();
  float deltaTime = float(currentTime - lastTime);
  
  // Get mouse position
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  
  // Reset mouse position for next frame
  glfwSetCursorPos(window, 1024/2, 768/2);
  
  // Compute new orientation
  horizontalAngle += mouseSpeed * float(1024/2 - xpos );
  verticalAngle   += mouseSpeed * float( 768/2 - ypos );
  
  // Direction : Spherical coordinates to Cartesian coordinates conversion
  glm::vec3 direction(
      cos(verticalAngle) * sin(horizontalAngle), 
      sin(verticalAngle),
      cos(verticalAngle) * cos(horizontalAngle)
  );
  glm::vec3 direction2(
      0, 
      1,
      0
  );
  
  // Right vector
  glm::vec3 right = glm::vec3(
    sin(horizontalAngle - 3.14f/2.0f), 
    0,
    cos(horizontalAngle - 3.14f/2.0f)
  );
  
  // Up vector
  glm::vec3 up = glm::cross( right, direction );
  
  // Move forward
  if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
    position += direction * deltaTime * speed;
  }
  // Move backward
  if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
    position -= direction * deltaTime * speed;
  }
  // Move Upward
  if (glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS){
    position += direction2 * deltaTime * speed;
  }
  // Move Downward
  if (glfwGetKey( window, GLFW_KEY_Z ) == GLFW_PRESS){
    position -= direction2 * deltaTime * speed;
  }
  // Strafe right
  if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
    position += right * deltaTime * speed;
  }
  // Strafe left
  if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
    position -= right * deltaTime * speed;
  }

  
  float FoV = initialFoV;
  ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
  // Camera matrix
  ViewMatrix       = glm::lookAt(
    position,           // Camera is here
    position+direction, // and looks here 
    up                  // Head is up
  );
  lastTime = currentTime;
}

}
