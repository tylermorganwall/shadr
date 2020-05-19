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
#'@param verbose Default `interactive()`. If `TRUE`, will output status messages.
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
#'\donttest{
#'run_shader(fragmentshader, width=800, height=800)
#'}
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
#'\donttest{
#'run_shader(fragmentshader, width=800, height=800)
#'}
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
#'\donttest{
#'run_shader(fragmentshader, width=800, height=800)
#'}
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
#'\donttest{
#'run_shader(fragmentshader, width=800, height=800)
#'}
run_shader = function(fragment, vertex=NULL, width=640, height=360, 
                      type = "glfw", replace = TRUE, verbose = interactive()) {
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
  if(verbose) {
    message("Hit [space] to pause and [esc] to close.")
  }
  open_window_rcpp(vertex, fragment, width, height, verbose, typeval)
}

#'@title Generate Shader Snapshot
#'
#'Generates a snapshot of the shader at the specified time.
#'
#'@param fragment Fragment shader.
#'@param time Default `0`. Time to take the snapshot.
#'@param filename Default `NULL`. if `NULL`, writes to current device. Otherwise, filename of the image.
#'@param vertex Default `NULL`. THe vertex shader.
#'@param width Default `640`. Width of the window.
#'@param height Default `320`. Width of the window.
#'@param type Default `glfw`. Can also be `shadertoy`. 
#'@param replace Default `TRUE`. If `type = "shadertoy"`, this will parse and replace `mainImage(...)`
#'with `main()`, `fragCoord` with `gl_FragCoord`, and `fragColor` with `color`. Note that `color` here
#'is a `vec3`, while it's a `vec4` on `shadertoy` (you will have to account for this yourself).
#'@param verbose Default `interactive()`. If `TRUE`, will output status messages.
#'@export
#'@examples
#'#We'll create a shader and take a few snapshots:
#'fragmentshader = "#version 330 core
#'uniform vec2 u_resolution;
#'uniform float u_time;
#'out vec3 color;
#'
#'#define PI 3.14159265359
#'
#'mat2 rotate2d(float _angle){
#'  return mat2(cos(_angle),-sin(_angle),
#'              sin(_angle),cos(_angle));
#'}
#'
#'void main(){
#'  vec2 st = gl_FragCoord.xy/u_resolution.xy;
#'  st.x *= u_resolution.x/u_resolution.y;
#'  float d = 0.0;
#'  
#'  // Remap the space to -1. to 1.
#'  st = st *2.0-1.0;
#'  st = rotate2d( sin(u_time)*PI ) * st;
#'  
#'  // Make the distance field
#'  d = length( abs(st)-.5 );
#'  
#'  // Visualize the distance field
#'  color = vec3(smoothstep(fract(d*abs(sin(u_time)*10)),0.3,0.8));
#'}"
#'
#'generate_shader_snapshot(fragmentshader, time=pi/2,width=500,height=500)
#'generate_shader_snapshot(fragmentshader, time=pi/4,width=500,height=500)
#'generate_shader_snapshot(fragmentshader, time=-pi/8,width=500,height=500)
#'generate_shader_snapshot(fragmentshader, time=4,width=500,height=500)
generate_shader_snapshot = function(fragment, time = 0, filename=NULL, vertex=NULL, 
                                    width=640, height=360, 
                                    type = "glfw", replace = TRUE, verbose = interactive()) {
  if(is.null(vertex)) {
    vertex = "#version 330 core
    layout(location = 0) in vec3 vertexPosition_modelspace;
    
    void main(){
    	gl_Position =  vec4(vertexPosition_modelspace,1);
    }"
  }
  if(!is.null(filename)) {
    if(tools::file_ext(filename) != "png") {
      filename = paste0(filename,".png")
    }
  }
  nofilename = FALSE
  if(is.null(filename)) {
    nofilename = TRUE
    filename = tempfile()
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
  if(verbose) {
    message("Hit [space] to pause and [esc] to close.")
  }
  generate_video_rcpp(vertex, fragment, width, height, typeval, verbose,
                      step=time, frames = 1L,
                      filename = filename)
  if(nofilename) {
    rayimage::plot_image(sprintf("%s%d.png", filename, 1))
  } 
}


#'@title Generate Shader Video
#'
#'
#'@param fragment Fragment shader.
#'@param filename Default `output`. Filename. If no file extension, `.mp4` will be added.
#'If file extension is `.gif``, a gif will be produced instead.
#'@param vertex Default `NULL`. THe vertex shader.
#'@param width Default `640`. Width of the window.
#'@param height Default `320`. Width of the window.
#'@param type Default `glfw`. Can also be `shadertoy`. 
#'@param replace Default `TRUE`. If `type = "shadertoy"`, this will parse and replace `mainImage(...)`
#'with `main()`, `fragCoord` with `gl_FragCoord`, and `fragColor` with `color`. Note that `color` here
#'is a `vec3`, while it's a `vec4` on `shadertoy` (you will have to account for this yourself).
#'@param verbose Default `interactive()`. If `TRUE`, will output status messages.
#'@param timestep Default `pi/180`. The timestep in the video.
#'@param frames Default `360`. Number of frames to generate in the video.
#'@param framerate Default `30`. Frames per second.
#'@export
#'@examples
#'#We'll create a shader and generate a video:
#'fragmentshader = "#version 330 core
#'uniform vec2 u_resolution;
#'uniform float u_time;
#'out vec3 color;
#'
#'#define PI 3.14159265359
#'
#'mat2 rotate2d(float _angle){
#'  return mat2(cos(_angle),-sin(_angle),
#'              sin(_angle),cos(_angle));
#'}
#'
#'void main(){
#'  vec2 st = gl_FragCoord.xy/u_resolution.xy;
#'  st.x *= u_resolution.x/u_resolution.y;
#'  float d = 0.0;
#'  
#'  // Remap the space to -1. to 1.
#'  st = st *2.0-1.0;
#'  st = rotate2d( sin(u_time)*PI ) * st;
#'  
#'  // Make the distance field
#'  d = length( abs(st)-.5 );
#'  
#'  // Visualize the distance field
#'  color = vec3(smoothstep(fract(d*abs(sin(u_time)*10)),0.2,0.8));
#'}"
#'\donttest{
#'generate_shader_video(fragmentshader, filename="sdf.mp4", width=500, height=500)
#'generate_shader_video(fragmentshader, filename="sdf.gif", timestep = pi/180*6,
#'                      width=500, height=500, frames=60, framerate = 15)
#'}
generate_shader_video = function(fragment, filename="output.mp4", vertex=NULL, 
                                 width=640, height=360,
                                 type = "glfw", replace = TRUE, verbose = interactive(),
                                 timestep = pi/180, frames = 360, framerate=30) {
  if(tools::file_ext(filename) != "mp4") {
    if(tools::file_ext(filename) != "gif") {
      filename = paste0(filename,"mp4")
    } 
  }
  if(is.null(vertex)) {
    vertex = "#version 330 core
    layout(location = 0) in vec3 vertexPosition_modelspace;
    
    void main(){
    	gl_Position =  vec4(vertexPosition_modelspace,1);
    }"
  }
  tempfilename = tempfile()
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
  frames = as.integer(frames)
  if(verbose) {
    message("Hit [space] to pause and [esc] to close.")
  }
  generate_video_rcpp(vertex, fragment, width, height, verbose, typeval, 
                      step=timestep, frames=frames,
                      filename = tempfilename)
  if(tools::file_ext(filename) == "mp4") {
    if("av" %in% rownames(utils::installed.packages())) {
      av::av_encode_video(input = sprintf("%s%d.png", tempfilename, seq_len(frames)), 
                         framerate= framerate, output = filename)
    } else {
      stop("{av} package required for generating mp4 files.")
    }
  } else {
    if("gifski" %in% rownames(utils::installed.packages())) {
      gifski::gifski(sprintf("%s%d.png", tempfilename, seq_len(frames)),
                     width = width, height = height,
                     gif_file = filename, delay = 1/framerate, progress = interactive())
    } else {
      stop("{gifski} package required for generating gifs")
    }
  }
}

#'@title Open Window Image
#'
#'
#'@param width Window width
#'@param height Window height
#'@param verbose Default `interactive()`. If `TRUE`, will output status messages.
#'@keywords internal
#'@examples
#'#internal
open_window_image = function(image, width=640, height=360, verbose = interactive()) {
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
  if(verbose) {
    message("Hit [space] to pause and [esc] to close.")
  }
  open_window_image_rcpp(vertexshader, fragmentshader, width, height, 
                         verbose=verbose,
                         r_layer, g_layer, b_layer)
}