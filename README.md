# How To Run

If you want to go through the codebase with neovim do `cmake -B build -G Ninja` to generate the compilation database it needs for code-completion. 

Depending on the IDE you want use, do `cmake -B build -G` followed by the generator, refer to https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#manual:cmake-generators(7) for a list of generators. I haven't tested all configurations and this is a pretty rough build system right now so be nice. 

After that do `cmake --build ./build` to build the project. Currently only Debug config is working. 

To run the exe, go into Binaries/App/Debug/App.exe
# 04-12-2025
After a year of typescript and C# it feels good to get some real work done in C++ and Vulkan. Bridge Engine now supports loading and texturing multiple models, and up to 16 lights in the scene. The codebase is very scrappy but that's a symptom of my Vulkan learning journey. The architecture will improve as time passes and my understanding of how the different parts of Vulkan work together to produce the effects of more complex modern computer graphics applications. 

<img width="1061" height="914" alt="image" src="https://github.com/user-attachments/assets/3a7e175b-240a-45e5-9a95-c85f8425b7dc" />

<img width="930" height="551" alt="image" src="https://github.com/user-attachments/assets/526eda91-a794-4b02-86f7-1a2aac7d513d" />

## Goals for 2026
- Architecture Refactor: The engine started as a vulkan learning testbed but is growing past that now as my understanding matures. To accommodate more complex features the architecture will need to change.
- PBR System
- Physics Simulation
- Scene Hierarchy
- Runtime Shader Recompilation
- Event System to further separate concerns
- Multithreading

# 15-12-2024
![Bridge Engine Demo](https://github.com/user-attachments/assets/a9237fb4-d383-40fe-be0d-1b4a799365bb)



