//glew Installed make install 
#include <GL/glew.h>
//GLWF3 Installed with cmake, make install 
#include <GLFW/glfw3.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void saveImage(const char* file, GLFWwindow* window) {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  GLsizei n_channels = 3;
  GLsizei stride = n_channels * width;
  stride += (stride % 4) ? (4 - stride % 4) : 0;
  GLsizei buffer_size = stride * height;
  std::vector<char> buffer(buffer_size);
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  glReadBuffer(GL_FRONT);
  glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
  stbi_flip_vertically_on_write(true);
  stbi_write_png(file, width, height, n_channels, buffer.data(), stride);
}