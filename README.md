## 3D Maze with Modern OpenGL

### What does it do?
3D maze game using modern OpenGL. The game has an interactive scenario, with navigation in first person, following the format of a standard game. The maze can have more than one floor and has portals (elevators) for transition between floors. The objective of the game is to collect all items and return to the starting position.

### The Game
#### The Navigation Map (Left) and the Game (Right)
<p align="center" width="100%">
    <img width="100%" src="https://raw.githubusercontent.com/alexandreclem/Maze/master/images/maze.png">    
</p>


### Description
#### Navigation Map
- Each cell can be classified like this:
    - x: Current player position
    - 0: Corridor
    - 1: Wall
    - 2: Elevator to go up
    - 3 Elevator to go down
    - 4: Collectable

#### Example of a Three Floor Maze
##### Architecture
<p align="center" width="100%">
    <img width="100%" src="https://raw.githubusercontent.com/alexandreclem/Maze/master/images/maze_ex.png">    
</p>

##### Input Data (3 Floors 8x8) - Each square: One Floor
<p align="center" width="100%">
    <img width="25%" src="https://raw.githubusercontent.com/alexandreclem/Maze/master/images/input_data.png">    
</p>

#### How to Run?

**Pre-Requesites**
- GCC Compiler
- Libraries: OpenGL | GLFW | GLAD | GLM | STB

**Libraries Installation**
- OpenGL
    - Windows
        - Generally can be found at **system/win32/lopengl32**
    - Linux
        - Already installed

- GLFW
    - Windows         
        - Install the 64 or 32bits binaries from: **https://www.glfw.org/download.html**
        - Unzip and after that:
            - Get the **glfw3.h** file from **include/GLFW/glfw3** directory
            - Get the **libglfw3.a** file from **lib-mingw-w64** directory
        - Paste the **glfw3.h** and **libglfw3.a** in the project **src/dependencies/GLFW** directory
    - Linux
        - Run the commands:
            ```bash
            $ sudo apt-get install libglfw3
            $ sudo apt-get install libglfw3-dev
            ```
        > **NOTE**
        >                    
        > You need to modify the header #include "dependencies/GLFW/glfw3.h" to #include \<GLFW/glfw3.h> in the **src/maze.cpp** file.
- GLAD    
    - Find out your OpenGL version:
        - Windows
            - Use **https://opengl-extensions-viewer.en.softonic.com/**
        - Linux
            - Run:
                ```bash
                $ sudo apt-get install mesa-utils
                $ glxinfo | grep "OpenGL version"
                ```
    - Install GLAD here **https://glad.dav1d.de/**
        - Settings:
            - Language: C/C++
            - Specification: OpenGL
            - API gl: your_opengl_version
            - Profile: Core
    - Unzip and after that:
        - Get the **glad.h** and **khrplatform.h** files from the **include** directory
        - Get the **glad.c** file from the **src** directory
        - Paste the **glad.h** and **khrplatform.h** and **glad.c** in the project **src/dependencies/GLAD** directory            
        > **NOTE**
        >            
        > You need to modify the **glad.c** header from #include \<glad/glad.h> to #include "glad.h".
    - Build the Library
        - Within the **src/dependencies/GLAD**, run:
            ```bash
            $ gcc -c glad.c
            $ ar rcs libglad.a glad.o
            ```

- GLM
    - The library is already available at the **src/dependencies/GLM** directory
    - However, if you want, can be found at **https://glm.g-truc.net/0.9.8/index.html** in the downloads section

- STB
    - Download the stb.h from here **https://github.com/nothings/stb/blob/master/stb_image.h**
    - After that, create a stb.cpp file with this code:
        ```C++
        #define STB_IMAGE_IMPLEMENTATION
        #include "stb_image.h"        
        ```
    - Paste the **stb.h** and **stb.cpp** files in the **src/dependencies/STB** directory
    - Build the Library
        - Within the **src/dependencies/STB** directory, run:
            ```bash
            $ g++ -c stb.cpp
            $ ar rcs libstb.a stb.o
            ```
    
**Execution**

- Within the **src** directory, run:
   - Windows
        - Compile
            ```bash
            $ g++ maze.cpp -Idependencies/GLFW -Idependencies/GLAD -Idependencies/STB -Ldependencies/GLFW -Ldependencies/GLAD -Ldependencies\STB .\dependencies\GLAD\libglad.a .\dependencies\GLFW\libglfw3.a .\dependencies\STB\libstb.a -lopengl32 -lglu32 -lgdi32 -o maze            
            ```
        - Run
            ```bash
            $ maze.exe
            ```

    - Linux
        ```bash
           
        ```

    

    
    





