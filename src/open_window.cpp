#include <Rcpp.h>
using namespace Rcpp;

//glew Installed make install 
#include <GL/glew.h>
//GLWF3 Installed with cmake, make install 
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp" 
#include "controls.h"
#include "loadshaders.h"

// [[Rcpp::export]]
int open_window_rcpp(const CharacterVector vertex_shader, const CharacterVector fragment_shader,
                    int width, int height, int type) {
  if(!glfwInit()){
    return(-1);
  }
  int nx = width;
  int ny = height;
  glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
  GLFWwindow* window; // (In the accompanying source code, this variable is global for simplicity)
  window = glfwCreateWindow( nx, ny, "shadr", NULL, NULL);
  if( window == NULL ){
    Rcpp::Rcout << "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" ;
    glfwTerminate();
    return(-1);
  }
  glfwMakeContextCurrent(window); // Initialize GLEW
  glewExperimental=true; // Needed in core profile
  if (glewInit() != GLEW_OK) {
    Rcpp::Rcout << "Failed to initialize GLEW\n";
    return(-1);
  }
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  // Hide the mouse and enable unlimited movement
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  
  glfwPollEvents();
  // glfwSetCursorPos(window, nx/2, ny/2);
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);
  
  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders( vertex_shader, fragment_shader );
  
  GLuint MatrixID = glGetUniformLocation(programID, "MVP");
  glm::mat4 Projection = glm::ortho(-1.0f, 1.0f,-1.0f,1.0f, -0.5f, 1000.0f);
  
  // Camera matrix
  glm::mat4 View       = glm::lookAt(
    glm::vec3(0,0,-1), // Camera Location
    glm::vec3(0,0,0), // Looks at the origin
    glm::vec3(0,1,0)  // Camera up is +Y
  );
  glm::mat4 Model      = glm::mat4(1.0f);
  glm::mat4 MVP        = Projection * View * Model; 
  
  static const GLfloat g_vertex_buffer_data[] = {
    -1.0f,-1.0f, 0.0f,
    1.0f,1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f,
    -1.0f,-1.0f, 0.0f,
    1.0f,-1.0f, 0.0f,
    1.0f,1.0f, 0.0f
  };
  
  static const GLfloat g_uv_buffer_data[] = {
    0.0f, 0.0f,
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
  };
  
  GLuint uTime;
  if(type == 1) {
    uTime = glGetUniformLocation(programID, "u_time");
  } else {
    uTime = glGetUniformLocation(programID, "iTime");
  }
  float t = 0;
  
  GLuint screenResolution;
  if(type == 1) {
    screenResolution = glGetUniformLocation(programID, "u_resolution");
  } else {
    screenResolution = glGetUniformLocation(programID, "iResolution");
  }
  
  GLuint mousePos;
  mousePos = glGetUniformLocation(programID, "u_mouse");
  
  GLuint vertexbuffer;
  glGenBuffers(1, &vertexbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
  
  GLuint uvbuffer;
  glGenBuffers(1, &uvbuffer);
  glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
  
  bool pause = false;
  double xpos, ypos;
  double debounce_time = 0.0;

  do{
    // Clear the screen
    if(glfwGetTime() - debounce_time > 0.1) {
      if(!pause) {
        t += 0.01;
      } 
    }
    glfwGetCursorPos(window, &xpos, &ypos);
    glfwPollEvents();
    
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
      pause = !pause;
      debounce_time = glfwGetTime();
    }
    glfwPollEvents();
    
      
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use our shader
    glUseProgram(programID);
    glUniform1f(uTime, t);
    
    int width2, height2;
    glfwGetFramebufferSize(window, &width2, &height2);
    glViewport(0, 0, width2, height2);    
    
    glUniform2f(screenResolution, width2, height2);
    glUniform2f(mousePos, xpos, ypos);
    
    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
      0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
      3,                  // size
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
    );
    
    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
      1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
      2,                                // size : U+V => 2
      GL_FLOAT,                         // type
      GL_FALSE,                         // normalized?
      0,                                // stride
      (void*)0                          // array buffer offset
    );
    
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 2*3); // 12*3 indices starting at 0 -> 12 triangles
    
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    
    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
    
  } while(!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);
  
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
  glDeleteProgram(programID);
  glDeleteVertexArrays(1, &VertexArrayID);
  
  glfwWaitEvents();
  glfwDestroyWindow(window);
  glfwWaitEvents();
  glfwTerminate();
  return(1);
}