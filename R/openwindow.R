#'@title Open Window
#'
#'
#'@param width Window width
#'@param height Window height
#'@export
#'@examples
#'#internal
open_window = function(vertex, fragment, width=640, height=360, type = "glfw") {
  typeval = switch(tonemap, "glfw" = 1,"shadertoy" = 2, 1)
  
  open_window_rcpp(vertex, fragment, width, height, typeval)
  
}

#'@title Open Window Image
#'
#'
#'@param width Window width
#'@param height Window height
#'@export
#'@examples
#'#internal
open_window_image = function(image, width=640, height=360) {
  dims = dim(image)
  r_layer = t(image[,,1] )[dims[2]:1,]
  g_layer = t(image[,,2] )[dims[2]:1,]
  b_layer = t(image[,,3] )[dims[2]:1,]
  vertexshader = "#version 330 core
  layout(location = 0) in vec3 vertexPosition_modelspace;
  layout(location = 1) in vec2 vertexUV;
  
  out vec2 UV;
  uniform mat4 MVP;
  
  void main(){
  	gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
  	UV = vertexUV;
  }"
  
  fragmentshader = "#version 330 core
  in vec2 UV;
  out vec3 color;
  uniform sampler2D myTextureSampler;
  void main(){
  	color = texture( myTextureSampler, UV ).rgb;
  }"
  
  open_window_image_rcpp(vertexshader, fragmentshader, width, height, 
                         r_layer, g_layer, b_layer)
}