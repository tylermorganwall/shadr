
<!-- README.md is generated from README.Rmd. Please edit that file -->

# shadr

<!-- badges: start -->

[![Lifecycle:
experimental](https://img.shields.io/badge/lifecycle-experimental-orange.svg)](https://www.tidyverse.org/lifecycle/#experimental)
<!-- badges: end -->

shadr provides an interface to write GLSL shaders in R\!

## Installation

Installation is a bit tricky, as it requires GLFW and GLEW. In all
cases, you’ll need
[RTools](https://cran.r-project.org/bin/windows/Rtools/) and
[CMake](https://cmake.org) installed. On macOS and Ubuntu the process is
fairly straightforward. For Windows 10 instructions it’s a bit more
complex–skip down to the Windows 10 section.

For macOS and Ubuntu, first download GLFW from either the [GLFW
website](https://www.glfw.org) via [this
link](https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.zip).

Now, unzip the folder and open it up in the terminal. Run the following:

### macOS:

``` sh
cd glfw-3.3.2
cmake CMakeLists.txt
sudo make install
```

Now, GLFW should be installed on macOS.

Installing GLEW is straightforward. [Download the source
code](https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.zip/download),
unzip, and run `make`:

``` sh
cd glew-2.1.0
sudo make install
```

Hopefully, you should be good to go.

### Ubuntu

On Ubuntu 20.04, I also had to install a series of libraries before
installing GLFW:

``` sh
sudo apt-get install -y libxrandr-dev
sudo apt-get install -y libxinerama-dev
sudo apt-get install -y libxcursor-dev
sudo apt-get install -y libxi-dev
cd glfw-3.3.2
cmake CMakeLists.txt
sudo make install
```

For GLEW, the process was slightly more involved: First, [download the
source
code](https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0.zip/download),
unzip, and run `make`:

``` sh
cd glew-2.1.0
sudo make install
```

I then had to manually link the shared object in `/usr/lib` (your system
configuration may differ):

``` sh
sudo ln -s /usr/lib64/libGLEW.so.2.1 /usr/lib/libGLEW.so.2.1
```

### Windows 10

For Windows 10, we are going to download the pre-compiled binaries for
GLFW and GLEW. *Note: I was only able to get this to work for the 64-bit
version of R, in all cases. I’ll update these instructions once I figure
out what’s wrong with the 32-bit installation.*

GFLW: [Link to page](https://www.glfw.org/download.html) with
pre-compiled binaries. [Direct link for 64-bit
binaries](https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN64.zip)
[Direct link for 32-bit
binaries](https://github.com/glfw/glfw/releases/download/3.3.2/glfw-3.3.2.bin.WIN32.zip)
GLEW: [Link to page](http://glew.sourceforge.net) with pre-compiled
binaries. [Direct link to
binaries](https://sourceforge.net/projects/glew/files/glew/2.1.0/glew-2.1.0-win32.zip/download)

Unzip the files into the directory of your choice.

First, clone this repo:

``` sh
git clone https://github.com/tylermorganwall/shadr
```

You will now need to change the `src/Makevars.win` file to point to the
pre-compiled binaries. There are three variables at the top of the file.
Change them to point to the directories you just downloaded on your
filesystem (I have left my directories in the file so you can see an
example):

``` sh
BASE_DIR_GLEW = C:/Users/tyler/Documents/glew-2.1.0-win32/glew-2.1.0
BASE_DIR_GLFW64 = C:/Users/tyler/Documents/glfw-3.3.2.bin.WIN64/glfw-3.3.2.bin.WIN64
BASE_DIR_GLFW32 = C:/Users/tyler/Documents/glfw-3.3.2.bin.WIN32/glfw-3.3.2.bin.WIN32
```

Now open the `shadr.proj` file in the `shady` directory to open RStudio,
select “Clean and Rebuild” in the Build menu, and the package will be
installed.

### Compiling on Windows 10

If the above process doesn’t work for you, you will need to compile GLEW
and GLFW yourself. First, download the source files. You will also need
to add `C:/Rtools/mingw_64/bin/` (or wherever Rtools is) to your PATH
environment variable. Navigate to the GLFW source folder in a terminal
and run the following:

``` sh
cd glfw-3.3.2
mkdir build
cd build
cmake -G"MinGW Makefiles" ..
mingw32-make.exe
```

Do the same for GLEW, but you have to go to the `glew-2.1.0/build/cmake`
directory before running the following:

``` sh
mkdir build
cd build
cmake -G"MinGW Makefiles" ..
mingw32-make.exe
```

Now, go in the `Makevars.win` and replace everything there with the
following:

``` make
CXX_STD = CXX11
PKG_LIBS = -lglfw3 -lglew32 -lgdi32 -lopengl32
```

Now open the `shadr.proj` file in the `shady` directory to open RStudio,
select “Clean and Rebuild” in the Build menu, and the package will be
installed. Hopefully.

## Examples

If you haven’t played with shaders before, check out [The Book of
Shaders](https://www.thebookofshaders.com) (no affiliation) for a nice
intro tutorial. Then check out lots of simple to advanced examples on
[Shadertoy](https://www.shadertoy.com).

I haven’t yet included the ability to pull screenshots, so here are some
manually captured gifs.

Let’s start with a bouncing ball:

``` r
library(shadr)

fragmentshader = "#version 330 core
uniform vec2 u_resolution;
uniform float u_time;
out vec3 color;

vec3 colorA = vec3(0.149,0.141,0.912);
vec3 colorB = vec3(1.000,0.833,0.224);

void main(){
    vec2 st = gl_FragCoord.xy/u_resolution.xy;
    st.x += cos(u_time*10)/10;
    st.y += sin(u_time*10)/10;
  float pct = 1.0;
  vec2 toCenter = vec2(0.5)-st;
  pct = smoothstep(length(toCenter)-0.2,0.1 * abs(sin(u_time*4.0)),0.8);
  color = mix(colorA,colorB,pct);
}"

run_shader(fragmentshader, width=800,height=800) 
```

Rendering and animating a signed distance field:

``` r
fragmentshader = "#version 330 core
uniform vec2 u_resolution;
uniform float u_time;
out vec3 color;

void main(){
  vec2 st = gl_FragCoord.xy/u_resolution.xy;
  st.x *= u_resolution.x/u_resolution.y;
  float d = 0.0;

  // Remap the space to -1. to 1.
  st = st *2.0-1.0;

  // Make the distance field
  d = length( abs(st)-.3 );

  // Visualize the distance field
  color = vec3(smoothstep(fract(d*abs(sin(u_time))*10.0),0.2,0.8));
}"

run_shader(fragmentshader, width=800,height=800) 
```

Here’s a complex one: “Seascape” by Alexander Alekseev aka TDM - 2014
(License Creative Commons Attribution-NonCommercial-ShareAlike 3.0
Unported License), translated for `shadr`.

``` r

fragmentshader = "#version 330 core
#ifdef GL_ES
precision mediump float;
#endif
uniform vec2 u_resolution;
uniform float u_time;
out vec3 color;

const int NUM_STEPS = 8;
const float PI      = 3.141592;
const float EPSILON = 1e-3;
#define EPSILON_NRM (0.1 / u_resolution.x)
//#define AA

// sea
const int ITER_GEOMETRY = 3;
const int ITER_FRAGMENT = 5;
const float SEA_HEIGHT = 0.6;
const float SEA_CHOPPY = 4.0;
const float SEA_SPEED = 0.8;
const float SEA_FREQ = 0.16;
const vec3 SEA_BASE = vec3(0.0,0.09,0.18);
const vec3 SEA_WATER_COLOR = vec3(0.8,0.9,0.6)*0.6;
#define SEA_TIME (1.0 + u_time * SEA_SPEED)
const mat2 octave_m = mat2(1.6,1.2,-1.2,1.6);

// math
mat3 fromEuler(vec3 ang) {
    vec2 a1 = vec2(sin(ang.x),cos(ang.x));
    vec2 a2 = vec2(sin(ang.y),cos(ang.y));
    vec2 a3 = vec2(sin(ang.z),cos(ang.z));
    mat3 m;
    m[0] = vec3(a1.y*a3.y+a1.x*a2.x*a3.x,a1.y*a2.x*a3.x+a3.y*a1.x,-a2.y*a3.x);
    m[1] = vec3(-a2.y*a1.x,a1.y*a2.y,a2.x);
    m[2] = vec3(a3.y*a1.x*a2.x+a1.y*a3.x,a1.x*a3.x-a1.y*a3.y*a2.x,a2.y*a3.y);
    return m;
}
float hash( vec2 p ) {
    float h = dot(p,vec2(127.1,311.7)); 
    return fract(sin(h)*43758.5453123);
}
float noise( in vec2 p ) {
    vec2 i = floor( p );
    vec2 f = fract( p );    
    vec2 u = f*f*(3.0-2.0*f);
    return -1.0+2.0*mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

// lighting
float diffuse(vec3 n,vec3 l,float p) {
    return pow(dot(n,l) * 0.4 + 0.6,p);
}
float specular(vec3 n,vec3 l,vec3 e,float s) {    
    float nrm = (s + 8.0) / (PI * 8.0);
    return pow(max(dot(reflect(e,n),l),0.0),s) * nrm;
}

// sky
vec3 getSkyColor(vec3 e) {
    e.y = (max(e.y,0.0)*0.8+0.2)*0.8;
    return vec3(pow(1.0-e.y,2.0), 1.0-e.y, 0.6+(1.0-e.y)*0.4) * 1.1;
}

// sea
float sea_octave(vec2 uv, float choppy) {
    uv += noise(uv);        
    vec2 wv = 1.0-abs(sin(uv));
    vec2 swv = abs(cos(uv));    
    wv = mix(wv,swv,wv);
    return pow(1.0-pow(wv.x * wv.y,0.65),choppy);
}

float map(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_GEOMETRY; i++) {        
        d = sea_octave((uv+SEA_TIME)*freq,choppy);
        d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
        uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

float map_detailed(vec3 p) {
    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;
    vec2 uv = p.xz; uv.x *= 0.75;
    
    float d, h = 0.0;    
    for(int i = 0; i < ITER_FRAGMENT; i++) {        
        d = sea_octave((uv+SEA_TIME)*freq,choppy);
        d += sea_octave((uv-SEA_TIME)*freq,choppy);
        h += d * amp;        
        uv *= octave_m; freq *= 1.9; amp *= 0.22;
        choppy = mix(choppy,1.0,0.2);
    }
    return p.y - h;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist) {  
    float fresnel = clamp(1.0 - dot(n,-eye), 0.0, 1.0);
    fresnel = pow(fresnel,3.0) * 0.5;
        
    vec3 reflected = getSkyColor(reflect(eye,n));    
    vec3 refracted = SEA_BASE + diffuse(n,l,80.0) * SEA_WATER_COLOR * 0.12; 
    
    vec3 color = mix(refracted,reflected,fresnel);
    
    float atten = max(1.0 - dot(dist,dist) * 0.001, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
    
    color += vec3(specular(n,l,eye,60.0));
    
    return color;
}

// tracing
vec3 getNormal(vec3 p, float eps) {
    vec3 n;
    n.y = map_detailed(p);    
    n.x = map_detailed(vec3(p.x+eps,p.y,p.z)) - n.y;
    n.z = map_detailed(vec3(p.x,p.y,p.z+eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

float heightMapTracing(vec3 ori, vec3 dir, out vec3 p) {  
    float tm = 0.0;
    float tx = 1000.0;    
    float hx = map(ori + dir * tx);
    if(hx > 0.0) return tx;   
    float hm = map(ori + dir * tm);    
    float tmid = 0.0;
    for(int i = 0; i < NUM_STEPS; i++) {
        tmid = mix(tm,tx, hm/(hm-hx));                   
        p = ori + dir * tmid;                   
        float hmid = map(p);
        if(hmid < 0.0) {
            tx = tmid;
            hx = hmid;
        } else {
            tm = tmid;
            hm = hmid;
        }
    }
    return tmid;
}

vec3 getPixel(in vec2 coord, float time) {    
    vec2 uv = coord / u_resolution.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;    
        
    // ray
    vec3 ang = vec3(sin(time*3.0)*0.1,sin(time)*0.2+0.3,time);    
    vec3 ori = vec3(0.0,3.5,time*5.0);
    vec3 dir = normalize(vec3(uv.xy,-2.0)); dir.z += length(uv) * 0.14;
    dir = normalize(dir) * fromEuler(ang);
    
    // tracing
    vec3 p;
    heightMapTracing(ori,dir,p);
    vec3 dist = p - ori;
    vec3 n = getNormal(p, dot(dist,dist) * EPSILON_NRM);
    vec3 light = normalize(vec3(0.0,1.0,0.8)); 
             
    // color
    return mix(
        getSkyColor(dir),
        getSeaColor(p,n,light,dir,dist),
        pow(smoothstep(0.0,-0.02,dir.y),0.2));
}

// main
void main( ) {
  vec3 color2 = getPixel(gl_FragCoord.xy, u_time);
    color = vec3(pow(color2,vec3(0.65)));
}"

run_shader(fragmentshader, width=800,height=800) 
```
