# Instructions

Run `install.bat` to download the dependencies and initialize CMake. This will create a folder named *build-win64* and in there you will find the Visual Studio solution.

If you add new files or change something in the *CMakeLists.txt* files run `reload.bat` to reload CMake.

Requirements:
* Git
* CMake 3.9.0 or higher
* Visual Studio 2017
* Latest windows sdk.

# Documentation for engine programmers

Initializing a application using my renderer works as followed:

1. Create a `rlr::Window`
2. Create a `rlr::RenderSystem`
3. Register a staging and imgui render function.
4. Initialize the render system using `rlr::RenderSystem::Init`
5. Create a loop and call `rlr::Window::PollEvents` and `rlr::RenderSystem::Render`
6. The render function takes a *scene graph*. This is a hierarchical tree of nodes that the renderer parses. After constructing a scene graph call init on it to initialize the nodes. (This will be automatic in the future)

For a example of loading files from disk, creating nodes and registering pipelines see the *RenderTest* example.

**IMPORTANT:** The scene graph currently also expects a viewport. This is going to be deprecated in the future.

**IMPORTANT:** A lot of functions currently take references instead of pointers. This will be changed back to pointers in the future. (This is a change I made for a personal project and haven't reverted yet)

**IMPORTANT:**  After you load textures and models they need to be uploaded to the GPU. This is called *staging*.

# Documentation for graphics programmers

The render part of the engine exists of 2 parts: 
1. A small C like abstraction layer of D3D12 
2. A "high" level render system which uses the abstraction layer.

## Abstraction layer

The abstraction layer still needs some clean up. All the functions take references instead of pointers. This was due to a personal project I was doing with it. Its however not beneficial to the actual renderer at all and will make us do lots of dereferencing. So all those function definitions will change in the future. Other than that the abstraction layer will slowly grow when we will use more and more features of D3D12.

## The High level part

The high level renderer consists of a few things such as resource loaders and skeletal meshes. But it also has a render system class. This class is a complete mess and is going to undergo lots of cleanup. The scene graph is not yet fully implemented but will come soon. The skeletal mesh needs also a lot of improvements regarding the animation loading and switching between animations.

Everything in the render system should be quite easy to understand. At some points I may still use my deprecated math library but that will also go into the bin. Especially the header of the render system is a big mess and contains lots of stuff that will need to be separated into different files. Also when looking at the header you might notice I create everything on the stack. This was for a old project and will be reverted to pointers soon enough.

Shaders are located in `resources/engine` mostly. They are written rather quickly and can use a lot of optimization and improvements.
