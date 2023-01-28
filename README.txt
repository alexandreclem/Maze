- Creating GLAD library -
gcc -c glad.c
ar rcs libglad.a glad.o

- Creating STB library -
g++ -c stb.cpp
ar rcs libstb.a stb.o

- Compiling maze.cpp -> maze.exe -
g++ maze.cpp -IGLFW -IGLAD -ISTB -LGLFW -LGLAD -LSTB .\GLAD\libglad.a .\GLFW\libglfw3.a .\STB\libstb.a -lopengl32 -lglu32 -lgdi32 -o maze

g++ maze.cpp -Idependencies/GLFW -Idependencies/GLAD -Idependencies/STB -Ldependencies/GLFW -Ldependencies/GLAD -Ldependencies\STB .\dependencies\GLAD\libglad.a .\dependencies\GLFW\libglfw3.a .\dependencies\STB\libstb.a -lopengl32 -lglu32 -lgdi32 -o maze
