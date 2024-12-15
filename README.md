# How To Run

If you want to go through the codebase with neovim do `cmake -B build -G Ninja` to generate the compilation database it needs for code-completion. 

Depending on the IDE you want use, do `cmake -B build -G` followed by the generator, refer to https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#manual:cmake-generators(7) for a list of generators. I haven't tested all configurations and this is a pretty rough build system right now so be nice. 

After that do `cmake --build ./build` to build the project. Currently only Debug config is working. 

To run the exe, go into Binaries/App/Debug/App.exe

![Bridge Engine Demo](https://github.com/user-attachments/assets/a9237fb4-d383-40fe-be0d-1b4a799365bb)
