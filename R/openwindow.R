#'@title Run Shader
#'
#'Opens a window and runs a GLSL shader. This version includes two uniforms: 
#'`u_resolution` (shadertoy: `iResolution`) and `u_time` (shadertoy: `iTime`). You can get the 
#'screenspace coordinate with `gl_FragCoord`. 
#'
#'Check out shadertoy.com for advanced examples, and thebookofshaders.com for a nice intro tutorial.
#'(no affiliation with either).
#'
#'@param fragment Fragment shader.
#'@param vertex Default `NULL`. THe vertex shader.
#'@param width Default `640`. Width of the window.
#'@param height Default `320`. Width of the window.
#'@param type Default `glfw`. Can also be `shadertoy`. 
#'@param replace Default `TRUE`. If `type = "shadertoy"`, this will parse and replace `mainImage(...)`
#'with `main()`, `fragCoord` with `gl_FragCoord`, and `fragColor` with `color`. Note that `color` here
#'is a `vec3`, while it's a `vec4` on `shadertoy` (you will have to account for this yourself).
#'@export
#'@examples
#'#The default vertex shader is:
#'
#'vertexshader = "#version 330 core
#'layout(location = 0) in vec3 vertexPosition_modelspace;
#'void main(){
#'  gl_Position =  vec4(vertexPosition_modelspace,1);
#'}"
#'
#'#Here we simply display a color palette across the screen:
#'fragmentshader = "#version 330 core
#'uniform vec2 u_resolution;  // Canvas size (width,height)
#'uniform float u_time;       // Time in seconds since load
#'
#'out vec3 color;
#'
#'void main() {
#'  vec2 st = gl_FragCoord.xy/(u_resolution);
#'  color = vec3(st.x,st.y,0.0);
#'}"
#'run_shader(fragmentshader, width=800, height=800)
#'
#'#Animating a signed distance field with time
#'fragmentshader = "#version 330 core
#'uniform vec2 u_resolution;
#'uniform float u_time;
#'out vec3 color;
#'
#'void main(){
#'  vec2 st = gl_FragCoord.xy/u_resolution.xy;
#'  st.x *= u_resolution.x/u_resolution.y;
#'  float d = 0.0;
#'  
#'  // Remap the space to -1. to 1.
#'  st = st *2.0-1.0;
#'  
#'  // Make the distance field
#'  d = length( abs(st)-.3 );
#'  
#'  // Visualize the distance field
#'  color = vec3(smoothstep(fract(d*abs(sin(u_time))*10.0),0.2,0.8));
#'}"
#'run_shader(fragmentshader, width=800, height=800)
#'
#'#Animating between colors:
#'fragmentshader = "#version 330 core
#'uniform vec2 u_resolution;
#'uniform float u_time;
#'out vec3 color;
#'
#'vec3 colorA = vec3(0.0,0.0,1.0);
#'vec3 colorB = vec3(1.0,0.0,0.0);
#'
#'void main() {
#'  vec3 color2 = vec3(0.0);
#'  float pct = abs(sin(u_time*4));
#'  color = mix(colorA, colorB, pct);
#'}"
#'run_shader(fragmentshader, width=800, height=800)
#'
#'#A moving, bouncing ball
#'fragmentshader = "#version 330 core
#'uniform vec2 u_resolution;
#'uniform float u_time;
#'out vec3 color;
#'
#'vec3 colorA = vec3(0.1,0.1,0.9);
#'vec3 colorB = vec3(1.0,0.8,0.2);
#'
#'void main(){
#'  vec2 st = gl_FragCoord.xy/u_resolution.xy;
#'  st.x += cos(u_time*10)/10;
#'  st.y += sin(u_time*10)/10;
#'  float pct = 1.0;
#'  vec2 toCenter = vec2(0.5)-st;
#'  pct = smoothstep(length(toCenter)-0.2,0.1 * abs(sin(u_time*4.0)),0.8);
#'  color = mix(colorA,colorB,pct);
#'}"
#'run_shader(fragmentshader, width=800, height=800)
run_shader = function(fragment, vertex=NULL, width=640, height=360, type = "glfw", replace = TRUE) {
  if(is.null(vertex)) {
    vertex = "#version 330 core
    layout(location = 0) in vec3 vertexPosition_modelspace;
    
    void main(){
    	gl_Position =  vec4(vertexPosition_modelspace,1);
    }"
  }
  typeval = switch(type, "glfw" = 1,"shadertoy" = 2, 1)
  if(typeval == 2) {
    #Replace
    fragment = gsub(pattern="(void )(mainImage\\(.+\\))(.+\\{)", 
                    replacement="\\1main()\\3", x=fragment, perl=TRUE)
    fragment = gsub(pattern="fragCoord",  fixed=TRUE,
                    replacement="gl_FragCoord", x=fragment)
    fragment = gsub(pattern="fragColor", fixed=TRUE,
                    replacement="color", x=fragment)
  }
  message("Hit [space] to pause and [esc] to close.")
  open_window_rcpp(vertex, fragment, width, height, typeval)
}

#'@title Open Window Image
#'
#'
#'@param width Window width
#'@param height Window height
#'@keywords internal
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