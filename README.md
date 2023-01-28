## 3D Maze with Modern OpenGL

### What does it do?
3D maze game using modern OpenGL. The game has an interactive scenario, with navigation in first person, following the format of a standard game. The maze can have more than one floor and has portals (elevators) for transition between floors. The objective of the game is to collect all items and return to the starting position.

### The Game
#### The Navigation Map (Left) and the Game (Right)
<p align="center" width="100%">
    <img width="90%" src="https://raw.githubusercontent.com/alexandreclem/Maze/master/images/maze.png">    
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
    <img width="90%" src="https://raw.githubusercontent.com/alexandreclem/Maze/master/images/maze_ex.png">    
</p>

##### Input Data (3 Floors 8x8) - Each square: One Floor
<p align="center" width="100%">
    <img width="30%" src="https://raw.githubusercontent.com/alexandreclem/Maze/master/images/input_data.png">    
</p>

#### How to Run?

**Pre-Requesites**
- GCC Compiler
- Libraries: OpenGL | GLFW | GLAD | GLM | STB

**Libraries Installation**
- OpenGL
    
