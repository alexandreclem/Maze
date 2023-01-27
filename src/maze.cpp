#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <random>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "GLM/glm.hpp"
#include "GLM/gtc/matrix_transform.hpp"
#include "GLM/gtc/type_ptr.hpp"

#include "shaders.h"
#include "read_obj.h"
#include "stb_image.h"


// Settings //
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

// Camera //
glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f,  0.0f);

// "Vision" using the mouse
float last_x = SCR_WIDTH / 2.0f;
float last_y = SCR_HEIGHT / 2.0f;
float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch =  0.0f;
bool first_mouse = true;

// Zooming
float fov   =  45.0f;

// Timing between frames //
float delta_time = 0.0f;	// Time between current frame and last frame
float last_frame = 0.0f;    // Time of last frame

// Maze elements attributes //
int number_of_layers;
int rows_per_layer;

typedef struct maze_element {
    int type;
    glm::vec3 position;
    int walls[4];   
}maze_element;
std::vector<maze_element> layers[100];
maze_element layer_matrix[100][100];

int current_layer;
std::pair<int, int> current_element_position;
std::pair<int, int> initial_element_position;
maze_element current_element;

int collectables; // Number of items that have already been collected
int total_collectables;

// Textures //
unsigned int texture_1, texture_2, texture_3, texture_4, texture_5;

// Maze Elements CPU & GPU Data //
unsigned int room_VBO, room_VAO;
unsigned int sphere_VBO, sphere_VAO;
unsigned int elevator_VBO, elevator_VAO;
int sphere_vertices_length = 0;
int elevator_vertices_length = 0;

// General Functions //
void gpu_data_room(float vertices[], int size);
void gpu_data_sphere(float vertices[], int size);
void gpu_data_elevator(float vertices[], int size);
void draw_maze_2d();
void draw_room(maze_element element, Shader shader);
void draw_sphere(maze_element element, Shader shader);
void draw_elevator(maze_element element, Shader shader);
void draw_maze(Shader shader);
void process_input(GLFWwindow *window);
void update_view_proj(Shader shader);
void load_textures();
void load_maze();
void bind_textures(unsigned int t_1, unsigned int t_2);
void player_status();

// Callback functions //
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


int main() { 
    // GLFW initialization //
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 3D Maze
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Maze", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); 
 
    // GLAD initialization //
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Callbacks //
    // Window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);       

    // Mouse        
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);   
    glfwSetCursorPosCallback(window, mouse_callback);  
    glfwSetScrollCallback(window, scroll_callback);    

    // Shaders initialization //
    Shader shader("vertex_shader.txt", "fragment_shader.txt"); // you can name your shader files however you like
    shader.use();

    // Initial OpenGL state //
    // Z-Buffer
    glEnable(GL_DEPTH_TEST);  
    
    // View port
    glViewport(0.0f, 0.0f, SCR_WIDTH, SCR_HEIGHT);

    // Initial projection Matrix      
    glm::mat4 projection_matrix = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float) SCR_HEIGHT, 0.01f, 100000.f);        
    shader.setMat4("projection", projection_matrix);

    // Textures //    
    load_textures();

    // Tell opengl for each sampler to which texture unit it belongs to (only has to be done once)   
    shader.setInt("TextureSampler2D_1", 0);
    shader.setInt("TextureSampler2D_2", 1);

    // Loading the Maze //        
    load_maze();  
    draw_maze_2d();      
   
    // Vertex Data (CPU) // 
    // 3D Room
    float room_vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };  
    
    // Sphere
    BlenderObject sphere;	
 	sphere.read_file("sphere.obj");    
	std::vector<float> sphere_data = sphere.vertices_data(sphere.get_vertices(), sphere.get_textures(), sphere.get_normals(), sphere.get_faces());   

	float *sphere_vertices = &sphere_data[0];
    sphere_vertices_length = sphere_data.size();	

    // Elevator
    BlenderObject elevator;	
 	elevator.read_file("elevator.obj");    
	std::vector<float> elevator_data = elevator.vertices_data(elevator.get_vertices(), elevator.get_textures(), elevator.get_normals(), elevator.get_faces());   
	
    float *elevator_vertices = &elevator_data[0];
	elevator_vertices_length = elevator_data.size();

    // Creating the VBOs and VAOs (GPU) //    
    gpu_data_room(room_vertices, sizeof(room_vertices));   
    gpu_data_sphere(sphere_vertices, sphere_vertices_length * sizeof(float));
    gpu_data_elevator(elevator_vertices, elevator_vertices_length * sizeof(float));        

    // Render Loop //
    while (!glfwWindowShouldClose(window)) {       
         // Checking if the player has collected all items and won the game
        player_status();        

        // Time per frame
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;       

        // Enabling shaders
        shader.use();       

        // input        
        process_input(window);

        // Updating the View (Camera) & Projection Matrices
        update_view_proj(shader);            
       
        // Cleaning the screen      
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);         
        
        // Rendering Objects (Model Matrix)      
        draw_maze(shader);                                                

        // GLFW: Swap buffers
        glfwSwapBuffers(window);       

        // Poll I/O events
        glfwPollEvents();
    }

    // De-allocate resources //    
    glDeleteVertexArrays(1, &room_VAO);
    glDeleteBuffers(1, &room_VBO);
    glDeleteVertexArrays(1, &sphere_VAO);
    glDeleteBuffers(1, &sphere_VBO);    
    glDeleteVertexArrays(1, &elevator_VAO);
    glDeleteBuffers(1, &elevator_VBO);

    // GLFW: terminate, clearing all previously allocated GLFW resources //
    glfwTerminate();   
    return 0;
}


// Transforming the room vertex data into room gpu data
void gpu_data_room(float vertices[], int size) { 
    // VBO and VAO initialization 
    glGenVertexArrays(1, &room_VAO);
    glGenBuffers(1, &room_VBO);

    // Bind the Vertex Array Object first,   
    glBindVertexArray(room_VAO);

    // Then bind and set vertex buffer(s), and
    glBindBuffer(GL_ARRAY_BUFFER, room_VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Then configure vertex attributes(s).
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind VAO (optional)
    glBindVertexArray(0);
}


// Transforming the sphere vertex data into sphere gpu data
void gpu_data_sphere(float vertices[], int size) {    
    // VBO and VAO initialization 
    glGenVertexArrays(1, &sphere_VAO);
    glGenBuffers(1, &sphere_VBO);

    // Bind the Vertex Array Object first,   
    glBindVertexArray(sphere_VAO);

    // Then bind and set vertex buffer(s), and
    glBindBuffer(GL_ARRAY_BUFFER, sphere_VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Then configure vertex attributes(s).
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO (optional)
    glBindVertexArray(0);
}


// Transforming the elevator vertex data into elevator gpu data
void gpu_data_elevator(float vertices[], int size) {
// VBO and VAO initialization 
    glGenVertexArrays(1, &elevator_VAO);
    glGenBuffers(1, &elevator_VBO);

    // Bind the Vertex Array Object first,   
    glBindVertexArray(elevator_VAO);

    // Then bind and set vertex buffer(s), and
    glBindBuffer(GL_ARRAY_BUFFER, elevator_VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    // Then configure vertex attributes(s).
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO (optional)
    glBindVertexArray(0);
}


// Drawing the rooms
void draw_room(maze_element element, Shader shader) {
    // Binding the textures that we want in the render
    bind_textures(texture_1, texture_2); 
    // Bind the room VAO 
    glBindVertexArray(room_VAO);

    if (element.type == 1) {
        // Model Matrix -> Scale - Translate - Rotate
        glm::mat4 model_matrix = glm::mat4(1.0f);        
        model_matrix = glm::scale(model_matrix, glm::vec3(100.0f,30.0f, 100.0f));                 
        model_matrix = glm::translate(model_matrix, element.position);                              
        shader.setMat4("model", model_matrix);
        // Draw
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    else if (element.type == 0 || element.type == -1) {
        // Model Matrix -> Scale - Translate - Rotate
        glm::mat4 model_matrix = glm::mat4(1.0f);        
        model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 30.0f, 100.0f));                 
        model_matrix = glm::translate(model_matrix, element.position);                              
        shader.setMat4("model", model_matrix);
        // Draw
        glDrawArrays(GL_TRIANGLES, 24, 36);
    }
    else if (element.type == 2) {
        // Model Matrix -> Scale - Translate - Rotate
        glm::mat4 model_matrix = glm::mat4(1.0f);        
        model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 30.0f, 100.0f));                 
        model_matrix = glm::translate(model_matrix, element.position);                              
        shader.setMat4("model", model_matrix);
        // Draw
        glDrawArrays(GL_TRIANGLES, 24, 36);

        // Draw the elevator to go up
        draw_elevator(element, shader);
    }
    else if (element.type == 3) {
        // Model Matrix -> Scale - Translate - Rotate
        glm::mat4 model_matrix = glm::mat4(1.0f);        
        model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 30.0f, 100.0f));                 
        model_matrix = glm::translate(model_matrix, element.position);                              
        shader.setMat4("model", model_matrix);
        // Draw
        glDrawArrays(GL_TRIANGLES, 24, 36);
        
        // Draw the elevator to go down        
        draw_elevator(element, shader);
    }
    else {
        // Model Matrix -> Scale - Translate - Rotate
        glm::mat4 model_matrix = glm::mat4(1.0f);        
        model_matrix = glm::scale(model_matrix, glm::vec3(100.0f, 30.0f, 100.0f));                 
        model_matrix = glm::translate(model_matrix, element.position);                              
        shader.setMat4("model", model_matrix);
        // Draw
        glDrawArrays(GL_TRIANGLES, 24, 36);        

        // Drawing the collectable
        draw_sphere(element, shader);
    }
}


// Drawing the spheres
void draw_sphere(maze_element element, Shader shader) {
    // Binding the textures that we want in the render
    bind_textures(texture_3, 0);    
    // Bind the sphere VAO 
    glBindVertexArray(sphere_VAO);

    // Model Matrix -> Scale - Translate - Rotate
    glm::mat4 model_matrix = glm::mat4(1.0f);             
    model_matrix = glm::scale(model_matrix, glm::vec3(5.0f, 5.0f, 5.0f));        
    model_matrix = glm::translate(model_matrix, 20.f * element.position);                                          
    model_matrix =  glm::rotate(model_matrix, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.25, 0.25f, 0.25f));         
    shader.setMat4("model", model_matrix);
    // Draw
    glDrawArrays(GL_TRIANGLES, 0, sphere_vertices_length / 5);    
}


// Drawing the elevators
void draw_elevator(maze_element element, Shader shader) {
    // Binding the textures that we want in the render
    if (element.type == 2)
        bind_textures(texture_4, 0); // UP
    else 
        bind_textures(texture_5, 0);  // Down
    
    // Bind the sphere VAO 
    glBindVertexArray(elevator_VAO);

    // Model Matrix -> Scale - Translate - Rotate
    glm::mat4 model_matrix = glm::mat4(1.0f);             
    model_matrix = glm::scale(model_matrix, glm::vec3(5.0f, 5.0f, 5.0f));    
    model_matrix = glm::translate(model_matrix, 20.f * element.position); 
    model_matrix =  glm::rotate(model_matrix, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0, 1.0f, 0.0f));                                                 
    shader.setMat4("model", model_matrix);
    // Draw
    glDrawArrays(GL_TRIANGLES, 0, elevator_vertices_length / 5);    
}


// 2D maze in order to aid navigation
void draw_maze_2d() {      
    std::cout << std::endl;  
    std::cout << "Layer " << current_layer << std::endl;    
    for (int i = 0; i < rows_per_layer; ++i) {
        for (int j = 0; j < rows_per_layer; ++j) {
            if (current_element_position.first == j && current_element_position.second == i)
                std::cout << "x ";
            else if (layers[current_layer][i * rows_per_layer + j].type == -1) 
                std::cout << "-1 ";
            else if (layers[current_layer][i * rows_per_layer + j].type == 0)
                std::cout << "0 ";
            else if (layers[current_layer][i * rows_per_layer + j].type == 1) 
                std::cout << "1 ";
            else if (layers[current_layer][i * rows_per_layer + j].type == 2) 
                std::cout << "2 ";
            else if (layers[current_layer][i * rows_per_layer + j].type == 3) 
                std::cout << "3 ";
            else 
                std::cout << "4 ";
        }
        std::cout << std::endl;
    }
    std::cout << "Items Collected: " << collectables << "/" << total_collectables << std::endl;
}


// Drawing the maze
void draw_maze(Shader shader) {
    // Reading the current layer in a matrix format
    int index = 0;    
    for (int i = 0; i < rows_per_layer; ++i) {
        for (int j = 0; j < rows_per_layer; ++j) {
            layer_matrix[i][j] = layers[current_layer][index];
            ++index;
        }
    }

    // Current layer being drawn element by element
    for (int i = 0; i < rows_per_layer; ++i) {
        for (int j = 0; j < rows_per_layer; ++j) {             
            maze_element element = layer_matrix[i][j];
            draw_room(element, shader);            
        }            
    }
}


// Loading the maze from input file
void load_maze() {    
    // Computing: Total rows, Total columns, Number of layers and Rows per layer
    int rows = 0, columns = 0;
    std::string line, item;    
    std::ifstream file( "input.txt" );

    while(getline(file, line)) {
        rows++;
        if ( rows == 1 ) {                              // First row only: determine the number of columns
      
            std::stringstream string_stream(line);      // Set up up a stream from this line
            while (string_stream >> item) 
                columns++;                              // Each item delineated by spaces adds one to cols               
        }        
    }
    number_of_layers = rows / columns;
    rows_per_layer = columns;    
    // std::cout << "\n- File had " << rows << " rows and " << columns << " columns" << " and "<< number_of_layers << " layers -" << std::endl;

    // Reseting the file to the beginning
    file.clear();
    file.seekg(0);     
    
    // Loading the layers and maps    
    std::vector<maze_element> layer_map;
    maze_element map_element;
    int matrix_map[100][100];
    int i_index, j_index;
    int type;
    char init;
    total_collectables = 0;
    // std::cout << "\n- Layers -" << std::endl;
    for (int layer = 0; layer < number_of_layers; ++layer) {
        // std::cout << "\nLayer " << layer << " maze:" << std::endl;
        for (int row = 0; row < rows_per_layer; ++row) {
            getline(file, line);
            std::stringstream string_stream(line);  
            
            // Reading the columns of a row
            int column = 0;                       
            while (string_stream >> item) {                
                if (std::stringstream(item) >> type) {
                    map_element.position = glm::vec3((float)(0 + column), 0.0f, (float)(0 + row));
                    map_element.type = type;
                    layer_map.push_back(map_element);
                    matrix_map[row][column] = type;   
                    if (type == 4)
                        total_collectables += 1;                 
                    // std::cout << type << " ";
                }
                    
                else if (std::stringstream(item) >> init) {
                    map_element.position = glm::vec3((float)(0 + column), 0.0f, (float)(0 + row));
                    map_element.type = -1;
                    layer_map.push_back(map_element);
                    matrix_map[row][column] = -1;                    
                    // std::cout << init << " ";
                }                     
                ++column;          
            }            
            // std::cout << "\n";
        }        
        
        // Defining the walls
        int index = 0;
        for (int i = 0; i < rows_per_layer; ++i) {
            for (int j = 0; j < columns; ++j) {
                if (matrix_map[i][j] == -1 || matrix_map[i][j] == 0 || matrix_map[i][j] == 2 || matrix_map[i][j] == 3 || matrix_map[i][j] == 4) {
                    if (j - 1 < 0) 
                        layer_map[index].walls[0] = 1;
                    else if (matrix_map[i][j - 1] == 1) 
                        layer_map[index].walls[0] = 1;
                    else
                        layer_map[index].walls[0] = 0;

                    if (j + 1 >= columns)
                        layer_map[index].walls[1] = 1;
                    else if (matrix_map[i][j + 1] == 1) 
                        layer_map[index].walls[1] = 1;
                    else
                        layer_map[index].walls[1] = 0;

                    if (i - 1 < 0) 
                        layer_map[index].walls[2] = 1;
                    if (matrix_map[i - 1][j] == 1) 
                        layer_map[index].walls[2] = 1;
                    else
                        layer_map[index].walls[2] = 0;  

                    if (i + 1 >= rows_per_layer)
                        layer_map[index].walls[3] = 1;                    
                    else if (matrix_map[i + 1][j] == 1) 
                        layer_map[index].walls[3] = 1;
                    else
                        layer_map[index].walls[3] = 0;                
                } 

                else {
                    layer_map[index].walls[0] = 1;
                    layer_map[index].walls[1] = 1;
                    layer_map[index].walls[2] = 1;
                    layer_map[index].walls[3] = 1;
                }
                ++index;
            }
        }

        // Cleaning the map
        layers[layer] = layer_map;
        layer_map.clear();   

        // std::cout << std::endl;
    }    

    // Setting up the initial position in the maze
    current_layer = 0;  
    int index = 0;
    for (int i = 0; i < rows_per_layer; ++i) {
        for (int j = 0; j < rows_per_layer; ++j) {
            layer_matrix[i][j] = layers[current_layer][index];  // Reading the first layer in a matrix format
            if (layers[current_layer][index].type == -1) {
                current_element = layers[current_layer][index];
                current_element_position.first = j;
                current_element_position.second = i;
                initial_element_position.first = j;
                initial_element_position.second = i;
                camera_pos = 100.f * current_element.position;         
            }   
            ++index;          
        }
    }       
    collectables = 0; // Number of items that have already been collected   
   
    // Printing the first layer with all attributes
    std::vector<maze_element> layer_0 = layers[2];
    // std::cout << "- First Layer -" << std::endl;    
    for (int i = 0; i < layer_0.size(); ++i) {    
        if (i % columns == 0) {
            // std::cout << "\nRow " << i / 8 << ":" << std::endl;   
        }    
        struct maze_element element = layer_0[i];                     
        // std::cout << element.type << " (" << element.position.x << " " << element.position.y <<  " " << element.position.z << ") " "(" << element.walls[0] << " " << element.walls[1] <<  " " << element.walls[2] << " " << element.walls[3] << ")"  << std::endl;        
    }
}


// Loading the textures used when rendering
void load_textures() {
    // Texture 1 -> Room
    glGenTextures(1, &texture_1);
    glBindTexture(GL_TEXTURE_2D, texture_1);
    
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* image_bytes = stbi_load("wall.jpg", &width, &height, &nrChannels, 0);
    if (image_bytes) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_bytes);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else    
        std::cout << "Failed to load texture" << std::endl;
    
    stbi_image_free(image_bytes);
    
    // Texture 2 -> Room
    glGenTextures(1, &texture_2);
    glBindTexture(GL_TEXTURE_2D, texture_2);
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load image, create texture and generate mipmaps
    image_bytes = stbi_load("emoji.png", &width, &height, &nrChannels, 0);
    if (image_bytes) {
        // Note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_bytes);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else    
        std::cout << "Failed to load texture" << std::endl;    
    
    stbi_image_free(image_bytes);

    // Texture 3 -> Sphere
    glGenTextures(1, &texture_3);
    glBindTexture(GL_TEXTURE_2D, texture_3);
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load image, create texture and generate mipmaps
    image_bytes = stbi_load("cash.jpg", &width, &height, &nrChannels, 0);
    if (image_bytes) {        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_bytes);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else    
        std::cout << "Failed to load texture" << std::endl;    
    
    stbi_image_free(image_bytes);

    // Texture 4 -> Elevator UP
    glGenTextures(1, &texture_4);
    glBindTexture(GL_TEXTURE_2D, texture_4);
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load image, create texture and generate mipmaps
    image_bytes = stbi_load("sky.jpg", &width, &height, &nrChannels, 0);
    if (image_bytes) {        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_bytes);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else    
        std::cout << "Failed to load texture" << std::endl;    
    
    stbi_image_free(image_bytes);

    // Texture 5 -> Elevator Down
    glGenTextures(1, &texture_5);
    glBindTexture(GL_TEXTURE_2D, texture_5);
    // Set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load image, create texture and generate mipmaps
    image_bytes = stbi_load("fire.jpg", &width, &height, &nrChannels, 0);
    if (image_bytes) {        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image_bytes);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else    
        std::cout << "Failed to load texture" << std::endl;    
    
    stbi_image_free(image_bytes);
}


// Binding the textures before each draw
void bind_textures(unsigned int t_1, unsigned int t_2) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t_1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, t_2);
}


// Checking if the player won the game
void player_status() {
    if (collectables == total_collectables && current_element_position == initial_element_position) {
        std::cout << "\n- You Won! :P -" << std::endl;        
        load_maze();
        draw_maze_2d();
        collectables = 0;
    }
}


// Player movement using WSAD keys
void process_input(GLFWwindow *window) {     
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); 

    // Restarts the game and all layers        
    if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        std::cout << "\n- Maze Restarted! :) -" << std::endl;        
        load_maze();
        draw_maze_2d();
        collectables = 0;        
    }
        
    // Camera speed
    const float camera_speed = 40.25f * delta_time;     
    // Colision in Y-axis   
    glm::vec3 camera_front_aux = glm::vec3((float)camera_front.x, 0.0f, (float)camera_front.z);          
    // Colision in X-axis and Z-axis
    if (current_element.walls[0] == 1 && camera_pos.x <= 100 * current_element_position.first - 49.) {
        camera_pos = glm::vec3(camera_pos.x + 0.05, camera_pos.y, camera_pos.z);   
        return;          
    } 

    if (current_element.walls[1] == 1 && camera_pos.x >= 100 * current_element_position.first + 49.) {
        camera_pos = glm::vec3(camera_pos.x - 0.05, camera_pos.y, camera_pos.z);             
        return;
    }

    if (current_element.walls[2] == 1 && camera_pos.z <= 100 * current_element_position.second - 49.) {        
        camera_pos = glm::vec3(camera_pos.x, camera_pos.y, camera_pos.z + 0.05);             
        return;
    }
    
    if (current_element.walls[3] == 1 && camera_pos.z >= 100 * current_element_position.second + 49.) {
        camera_pos = glm::vec3(camera_pos.x, camera_pos.y, camera_pos.z - 0.05);             
        return;
    }    

    // When the player enter in another maze element
    int draw_maze_2d_flag = 0;
    if (current_element.walls[0] == 0 && camera_pos.x <= 100 * current_element_position.first - 49.8) {
        current_element_position.first -= 1;   
        current_element = layer_matrix[current_element_position.second][current_element_position.first];
        draw_maze_2d_flag = 1;        
    } 
        
    if (current_element.walls[1] == 0 && camera_pos.x >= 100 * current_element_position.first + 49.8) {
        current_element_position.first += 1;      
        current_element = layer_matrix[current_element_position.second][current_element_position.first];
        draw_maze_2d_flag = 1;        
    }
        
    if (current_element.walls[2] == 0 && camera_pos.z <= 100 * current_element_position.second - 49.8) {        
        current_element_position.second -= 1;         
        current_element = layer_matrix[current_element_position.second][current_element_position.first];   
        draw_maze_2d_flag = 1;        
    }
        
    if (current_element.walls[3] == 0 && camera_pos.z >= 100 * current_element_position.second + 49.8) {
        current_element_position.second += 1;             
        current_element = layer_matrix[current_element_position.second][current_element_position.first];
        draw_maze_2d_flag = 1;        
    }
    
    // When the player pick up a collectable item
    if (current_element.type == 4) {            
        if (camera_pos.x <= 100 * current_element_position.first + 5 && camera_pos.x >= 100 * current_element_position.first - 5 && camera_pos.z <= 100 * current_element_position.second + 5 && camera_pos.z >= 100 * current_element_position.second - 5)  {            
            layers[current_layer][current_element_position.second * rows_per_layer + current_element_position.first].type = 0; // Transforming in an empty room
            current_element = layers[current_layer][current_element_position.second * rows_per_layer + current_element_position.first];
            collectables += 1;
            draw_maze_2d_flag = 1;        
        }
    }   

    // When the player pass through an elevator
    // Elevator to go up
    if (current_element.type == 2)
    {
         if (camera_pos.x <= 100 * current_element_position.first + 5 && camera_pos.x >= 100 * current_element_position.first - 5 && camera_pos.z <= 100 * current_element_position.second + 5 && camera_pos.z >= 100 * current_element_position.second - 5)  {
            current_layer = current_layer + 1;
            current_element = layers[current_layer][current_element_position.second * rows_per_layer + current_element_position.first]; // The element in the top layer has the same coordinates as the current one                       
            draw_maze_2d_flag = 1;                 
        }
    }

    // Elevator to go down
    if (current_element.type == 3) {
         if (camera_pos.x <= 100 * current_element_position.first + 5 && camera_pos.x >= 100 * current_element_position.first - 5 && camera_pos.z <= 100 * current_element_position.second + 5 && camera_pos.z >= 100 * current_element_position.second - 5)  {
            current_layer = current_layer - 1;
            current_element = layers[current_layer][current_element_position.second * rows_per_layer + current_element_position.first]; // The element in the top layer has the same coordinates as the current one                   
            draw_maze_2d_flag = 1;                 
        }        
    }

    if (draw_maze_2d_flag)
        draw_maze_2d();

    // Player movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)                      
        camera_pos += (float)(1.5 * camera_speed) * camera_front_aux;    
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_pos -= (float)(1.5 * camera_speed) * camera_front_aux;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_pos -= glm::normalize(glm::cross(camera_front_aux, camera_up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_pos += glm::normalize(glm::cross(camera_front_aux, camera_up)) * camera_speed;        
    
}


// Update the player "vision" 
void update_view_proj(Shader shader) {
    // View
    glm::mat4 view_matrix = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
    shader.setMat4("view", view_matrix);

    // Projection     
    glm::mat4 projection_matrix = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
    shader.setMat4("projection", projection_matrix);
}


// Resize the game window
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


// Creating the player "vision"
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (first_mouse)
    {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }
  
    float x_offset = xpos - last_x;
    float y_offset = last_y - ypos; 
    last_x = xpos;
    last_y = ypos;

    float sensitivity = 0.085f;
    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw   += x_offset;
    pitch += y_offset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 camera_direction;
    camera_direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera_direction.y = sin(glm::radians(pitch));
    camera_direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera_front = glm::normalize(camera_direction);
}


// Possibility to zooming
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}