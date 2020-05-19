#ifndef LOADSHADERSH
#define LOADSHADERSH

#include <Rcpp.h>

//glew Installed make install 
#include <GL/glew.h>
//GLWF3 Installed with cmake, make install 
#include <GLFW/glfw3.h>

GLuint LoadShaders(const Rcpp::CharacterVector vertex_shader, 
                   const Rcpp::CharacterVector fragment_shader, bool verbose);
  
#endif
