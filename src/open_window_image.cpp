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
int open_window_image_rcpp(const CharacterVector vertex_shader, const CharacterVector fragment_shader,
                      int width, int height, 
                      NumericMatrix& r_layer,  NumericMatrix& g_layer,  NumericMatrix& b_layer) {
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
  
  //SETUP DONE
  
  //Create red texture
  int texnx = r_layer.ncol();
  int texny = r_layer.nrow();
  
  float *tex_array = new float[3*texnx*texny];
  for(int i = 0; i < texny; i++) {
    for(int j = 0; j < texnx; j++) {
      tex_array[3*j + (3*texnx)*i] = b_layer(i,j);
      tex_array[3*j+1 + (3*texnx)*i] = g_layer(i,j);
      tex_array[3*j+2 + (3*texnx)*i] = r_layer(i,j);
    }
  }
  //TEXTURE ARRAY DONE
  
  glfwPollEvents();
  glfwSetCursorPos(window, nx/2, ny/2);
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);
  // glEnable(GL_CULL_FACE);
  
  GLuint VertexArrayID;
  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);
  
  // Create and compile our GLSL program from the shaders
  GLuint programID = LoadShaders( vertex_shader, fragment_shader );
  
  GLuint MatrixID = glGetUniformLocation(programID, "MVP");
  
  // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
  // glm::mat4 Projection = glm::perspective(glm::radians(90.0f), 3.0f / 3.0f, 0.1f, 100.0f);
  glm::mat4 Projection = glm::ortho(-1.0f, 1.0f,-1.0f,1.0f, -0.5f, 1000.0f);
  
  // Camera matrix
  glm::mat4 View       = glm::lookAt(
    glm::vec3(0,0,-1), // Camera is at (4,3,3), in World Space
    glm::vec3(0,0,0), // and looks at the origin
    glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
  );
  // Model matrix : an identity matrix (model will be at the origin)
  glm::mat4 Model      = glm::mat4(1.0f);
  // Our ModelViewProjection : multiplication of our 3 matrices
  glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around
  //Everything above here is fine--now custom texture stuff
  
  GLuint textureID;
  glGenTextures(1, &textureID);
  
  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, texnx, texny, 0, GL_BGR, GL_FLOAT, tex_array);
  // 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  // Get a handle for our "myTextureSampler" uniform
  GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
  
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
  uTime = glGetUniformLocation(programID, "u_time");
  float t = 0;
  
  GLuint screenResolution;
  screenResolution = glGetUniformLocation(programID, "u_resolution");
  
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
    glUniform2f(screenResolution, nx*2, ny*2);
    glUniform2f(mousePos, xpos, ypos);
    
    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    
    // Bind our texture in Texture Unit 0
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, TextureID);
    // Set our "myTextureSampler" sampler to use Texture Unit 0
    // glUniform1i(TextureID, 0);
    
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
    
  } // Check if the ESC key was pressed or the window was closed
  while(!glfwWindowShouldClose(window) && glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS);
  
  glDeleteBuffers(1, &vertexbuffer);
  glDeleteBuffers(1, &uvbuffer);
  glDeleteProgram(programID);
  // glDeleteTextures(1, &TextureID);
  glDeleteVertexArrays(1, &VertexArrayID);
  // delete[] tex_array;
  
  glfwWaitEvents();
  glfwDestroyWindow(window);
  glfwWaitEvents();
  glfwTerminate();
  return(1);
}