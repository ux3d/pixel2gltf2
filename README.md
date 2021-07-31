[![](glTF.png)](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)

# Use pixelated image and generate voxels in glTF 2.0

pixel2gltf2 is a command line tool for generating voxel cubes in glTF 2.0.  
The tool is stepping through a pixelated image and is generating a cube as a voxel.  
A background pixel is not generated as a voxel.  

Usage: `Usage: pixel2gltf2 image [-p 25 -r 245 -g 245 -b 245]`  

`image` Filename of the pixelated image.  
`-p 25` Dimension of the pixel. This is the step size iterating the pixels of an image.  
`-r 245` Background red value.  
`-g 245` Background green value.  
`-b 245` Background blue value.  


## Software Requirements

* C/C++ 17 compiler e.g. gcc or Visual C++
* [Eclipse IDE for C/C++ Developers](https://www.eclipse.org/downloads/packages/release/2021-03/r/eclipse-ide-cc-developers) or  
* [CMake](https://cmake.org/)  


## Import the generated glTF

Import the generated glTF in e.g. [Gestaltor](https://gestaltor.io/) to inspect the generated scene.  
