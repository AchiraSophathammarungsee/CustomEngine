# CustomEngine
Custom rendering engine made from c++, OpenGL.

# Build
The application must be run on machine with OpenGL version 4.6 or higher, else it will give out version error
       
The application must be compiled with visual studio with 32-bit debug or release mode (no 64-bit)
       
If compile on laptop, please make sure to select Dedicate Graphics Card (NVidia) for the application (.exe)

# Running
Run Debug/Engine.exe for debug version

Run Release/Engine.exe for release version

Both of these are 32-bit

*There are pre-compiled executables in case the project failed to build.

# Scene navigation
W/A/S/D to move around
Right click + move mouse to look around
Space/Shift to move up / down
*The scene navigation is somewhat similar to Unity

Left window contains global system parameters.
Can switch to [Entity manager] tab at the top to see all game objects.

Bottom window is resource (asset) manager where all loaded resources (mesh, texture, etc.) are stored.

Right window [inspector] shows selected object’s properties.
Resources and Game objects can be selected from both scene window (left) or resource window (bottom)