#ifndef READ_OBJ_H
#define READ_OBJ_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>


struct vertex {
	float x;
	float y;
	float z;
};

struct normal {
	float x;
	float y;
	float z;
};

struct texture {
	float u;
	float v;		
};

struct vert_norm_text {
	unsigned int v, n, t;
};

struct face {
	std::vector<vert_norm_text> face_vnt;		
};


class BlenderObject {	
	private:
		std::vector<vertex> vertices;
		std::vector<texture> textures;
		std::vector<normal> normals;
		std::vector<face> faces;
	public:
		std::vector<vertex> get_vertices() {
			return vertices;
		}
		std::vector<texture> get_textures() {
			return textures;
		}
		std::vector<normal> get_normals() {
			return normals;
		}
		std::vector<face> get_faces() {
			return faces;
		}
		void read_file(const char * file_name);		
		void show_data();
		std::vector<float> vertices_data(std::vector<vertex> vertices, std::vector<texture> textures, std::vector<normal> normals, std::vector<face> faces);						
};


void BlenderObject::read_file(const char* file_name) {
	std::ifstream my_file(file_name);
	std::string line;	

	while(std::getline(my_file, line)) {		
		std::string element = line.substr(0, line.find(" "));
		std::string parameters = line.substr(line.find(" ") + 1);			

		if (element == "v") {			
			std::stringstream parameters_stream(parameters);	
			vertex v;
			parameters_stream >> v.x;
			parameters_stream >> v.y;
			parameters_stream >> v.z;			
			this->vertices.push_back(v);
		}		

		else if (element == "vn") {
			std::stringstream parameters_stream(parameters);	
			normal n;
			parameters_stream >> n.x;
			parameters_stream >> n.y;
			parameters_stream >> n.z;
			this->normals.push_back(n);
		}

		else if (element == "vt") {
			std::stringstream parameters_stream(parameters);	
			texture t;
			parameters_stream >> t.u;
			parameters_stream >> t.v;			
			this->textures.push_back(t);
		}

		else if (element == "f") {
			std::replace(parameters.begin(), parameters.end(), '/' , ' ');
			std::stringstream parameters_stream(parameters);	
			
            std::stringstream temp(parameters);	
            int number_of_vertices = 0;
            std::string temp_2;
            while (temp >> temp_2)
  				++number_of_vertices;            
            number_of_vertices = number_of_vertices / 3;                   

			face f;			
			for (int i = 0; i < number_of_vertices; ++i) {
				vert_norm_text vnt;
				parameters_stream >> vnt.v;
				parameters_stream >> vnt.t;
				parameters_stream >> vnt.n;	
				f.face_vnt.push_back(vnt);						
			}   
			faces.push_back(f);		       		
		}
	}
}


std::vector<float> BlenderObject::vertices_data(std::vector<vertex> vertices, std::vector<texture> textures, std::vector<normal> normals, std::vector<face> faces) {
		std::vector<float> vertices_;		
	 	for(int i = 0; i < faces.size(); ++i) {   		                      
	 		face f = faces[i];								 		
			vertices_.push_back(vertices[f.face_vnt[0].v - 1].x);
			vertices_.push_back(vertices[f.face_vnt[0].v - 1].y);
			vertices_.push_back(vertices[f.face_vnt[0].v - 1].z);
			vertices_.push_back(textures[f.face_vnt[0].t - 1].u);
			vertices_.push_back(textures[f.face_vnt[0].t - 1].v);			

			vertices_.push_back(vertices[f.face_vnt[1].v - 1].x);
			vertices_.push_back(vertices[f.face_vnt[1].v - 1].y);
			vertices_.push_back(vertices[f.face_vnt[1].v - 1].z);
			vertices_.push_back(textures[f.face_vnt[1].t - 1].u);
			vertices_.push_back(textures[f.face_vnt[1].t - 1].v);			

			vertices_.push_back(vertices[f.face_vnt[2].v - 1].x);
			vertices_.push_back(vertices[f.face_vnt[2].v - 1].y);
			vertices_.push_back(vertices[f.face_vnt[2].v - 1].z);
			vertices_.push_back(textures[f.face_vnt[2].t - 1].u);
			vertices_.push_back(textures[f.face_vnt[2].t - 1].v);		
	 	}	
		return vertices_;
}


void BlenderObject::show_data() {	
	std::cout << "\tVertices" << std::endl;

	for (int i = 0; i < vertices.size(); ++i) 
		std::cout << vertices[i].x << " " <<  vertices[i].y << " " << vertices[i].z << std::endl;

	std::cout << "\tNormals" << std::endl;
	for (int i = 0; i < normals.size(); ++i) 
		std::cout << normals[i].x << " " <<  normals[i].y << " " << normals[i].z << std::endl;
	
	std::cout << "\tTextures" << std::endl;
	for (int i = 0; i < textures.size(); ++i) 
		std::cout << textures[i].u << " " <<  textures[i].v << " " <<  std::endl;

	std::cout << "\tFaces" << std::endl;	
	for (int i = 0; i < faces.size(); ++i) {		
		for (int j = 0; j < faces[i].face_vnt.size(); ++j) {
			std::cout << "|" << faces[i].face_vnt[j].v << " " <<  faces[i].face_vnt[j].t << " " << faces[i].face_vnt[j].n << "|";		
		}
		std::cout << std::endl;				
	}		
}
#endif