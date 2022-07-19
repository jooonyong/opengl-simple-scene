#pragma once

#include <math.h>
#include <iostream>
#include <fstream>
#include "gl\freeglut.h"		// OpenGL header files
#include <list>
#include <vector>
#include <string>
#include <math.h>
#include <stdio.h>
//#define TIME_CHECK_
#define DEPTH_CALIB_
#pragma warning(disable:4996)
#define BLOCK 4
#define READ_SIZE 110404//5060//110404

#define bedscale 150
#define sofascale 8
#define refscale 100
#define tablescale 3
//#define TEST 14989
//#define STATIC_

using namespace std;

struct Vertex {
	float X;
	float Y;
	float Z;
	int index_1;
	int index_2;
	int index_3;
};

// variables for GUI
const float TRACKBALLSIZE = 0.8f;
const int RENORMCOUNT = 97;

GLint drag_state = 0;
GLint button_state = 0;

GLint rot_x = 0;
GLint rot_y = 0;
GLint trans_x = 0;
GLint trans_y = 0;
GLint trans_z = 0;

GLubyte mytexels[276][276][3];
GLubyte wall[512][512][3];
GLubyte sofa[1080][1920][3];
GLubyte bed[1080][1920][3];
GLubyte refrig[1280][1920][3];
GLubyte table[1024][1024][3];

int add_depth_flag = 0;
int model_flag = 0;
int depth_display_flag = 0;
int geodesic_skel[23][5] = { 0 };
int trcon = 0;
float zmin = 100000, zmax = -100000;

int side_status[50] = { 0 };

float quat[4] = { 0 };
float t[3] = { 0 };

Vertex skt[23];
BOOLEAN bTracked = false;
bool checkt = false;

GLuint tex_ids[7];

vector<Vertex> vertex;
vector<Vertex> temp_vertex;
vector<Vertex> temp_uvs;
vector<Vertex> temp_normals;

vector<Vertex> BedVertex;
vector<Vertex> BedNormal;
vector<Vertex> BedTexture;

vector<Vertex>SofaVertex;
vector<Vertex>SofaNormal;
vector<Vertex>SofaTexture;

vector<Vertex>RefVertex;
vector<Vertex>RefNormal;
vector<Vertex>RefTexture;

vector<Vertex>TableVertex;
vector<Vertex>TableNormal;
vector<Vertex>TableTexture;

std::vector< int > vertexIndices, uvIndices, normalIndices;

bool recheck;

// variables for display OpenGL based point viewer
int dispWindowIndex = 0;
GLuint dispBindIndex = 0;
const float dispPointSize = 2.0f;

// variables for display text
string dispString = "";
const string dispStringInit = "Depth Threshold: D\nInfrared Threshold: I\nNonlocal Means Filter: N\nPick BodyIndex: P\nAccumulate Mode: A\nSelect Mode: C,B(select)\nSave: S\nReset View: R\nQuit: ESC";
string frameRate;

HANDLE hMutex;
//KinectBasic kinect;

// functions for GUIs
void InitializeWindow(int argc, char* argv[]);

// high-level functions for GUI
void draw_center();
void idle();
void display();
void close();
void special(int, int, int) {}
void keyboard(unsigned char, int, int);
void mouse(int, int, int, int);
void reshape(int, int);
void motion(int, int);

float touv(float a) {
	double n;
	float result = modf(a, &n);
	if (result < 0) {
		result = result * (-1);
	}
	return result;
}
// basic functions for computation/GUI
// trackball codes were imported from those of Gavin Bell
// which appeared in SIGGRAPH '88
void vzero(float*);
void vset(float*, float, float, float);
void vsub(const float*, const float*, float*);
void vcopy(const float*, float*);
void vcross(const float* v1, const float* v2, float* cross);
float vlength(const float* v);
void vscale(float* v, float div);
void vnormal(float* v);
float vdot(const float* v1, const float* v2);
void vadd(const float* src1, const float* src2, float* dst);

void trackball(float q[4], float, float, float, float);
//void add_quats(float*, float*, float*);
void axis_to_quat(float a[3], float phi, float q[4]);
void normalize_quat(float q[4]);
float tb_project_to_sphere(float, float, float);
void build_rotmatrix(float m[4][4], float q[4]);

void LoadObj(const char* filename, vector<Vertex>& ObjVertex, vector<Vertex>& ObjTex, vector<Vertex>& ObjNorm, const int scale) {
	// read File
	ifstream openFile(filename);
	if (openFile.is_open()) {
		string line;
		while (getline(openFile, line)) {
			float x, y, z;
			char lineHeader[128];
			sscanf(line.c_str(), "%s %f %f %f", lineHeader, &x, &y, &z);
			if (strcmp(lineHeader, "v") == 0) {
				Vertex tempVertex;
				tempVertex.X = x / scale;
				tempVertex.Y = y / scale;
				tempVertex.Z = z / scale;
				temp_vertex.push_back(tempVertex);
			}
			else if (strcmp(lineHeader,"vn")==0) {
				Vertex tempVertex;
				tempVertex.X = x;
				tempVertex.Y = y;
				tempVertex.Z = z;
				temp_normals.push_back(tempVertex);
			}
			else if (strcmp(lineHeader, "vt") == 0) {
				Vertex tempVertex;
				tempVertex.X = x;
				tempVertex.Y = y;
				tempVertex.Z = z;
				temp_uvs.push_back(tempVertex);
			}
			else if (strcmp(lineHeader, "f") == 0) {
				int vertexindex[4], uvindex[4], normalindex[4];
				int matches = sscanf(line.c_str(), "%s %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", lineHeader, &vertexindex[0], &uvindex[0], &normalindex[0], &vertexindex[1], &uvindex[1], &normalindex[1], &vertexindex[2], &uvindex[2], &normalindex[2], &vertexindex[3], &uvindex[3], &normalindex[3]);
				if (matches != 13) {
					printf("File can't be read by our simple parser : ( Try exporting with other options\n");
					//return;
				}
				vertexIndices.push_back(vertexindex[0]);
				vertexIndices.push_back(vertexindex[1]);
				vertexIndices.push_back(vertexindex[2]);
				vertexIndices.push_back(vertexindex[3]);

				uvIndices.push_back(uvindex[0]);
				uvIndices.push_back(uvindex[1]);
				uvIndices.push_back(uvindex[2]);
				uvIndices.push_back(uvindex[3]);

				normalIndices.push_back(normalindex[0]);
				normalIndices.push_back(normalindex[1]);
				normalIndices.push_back(normalindex[2]);
				normalIndices.push_back(normalindex[3]);
			}
		}
		openFile.close();
	}
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		unsigned int vertexIndex = vertexIndices[i];
		Vertex tempVertex = temp_vertex[vertexIndex - 1];
		ObjVertex.push_back(tempVertex);
	}
	for (unsigned int i = 0; i < uvIndices.size(); i++) {
		unsigned int uvIndex = uvIndices[i];
		Vertex tempUv = temp_uvs[uvIndex - 1];
		ObjTex.push_back(tempUv);
	}
	for (unsigned int i = 0; i < normalIndices.size(); i++) {
		unsigned int normalIndex = normalIndices[i];
		Vertex tempNormal = temp_normals[normalIndex - 1];
		ObjNorm.push_back(tempNormal);
	}
	//º¤ÅÍ ºñ¿ì±â
	temp_vertex.clear();
	temp_uvs.clear();
	temp_normals.clear();
	vertexIndices.clear();
	uvIndices.clear();
	normalIndices.clear();

	vector<Vertex>().swap(temp_vertex);
	vector<Vertex>().swap(temp_uvs);
	vector<Vertex>().swap(temp_normals);
	vector<int>().swap(vertexIndices);
	vector<int>().swap(uvIndices);
	vector<int>().swap(normalIndices);
}

void Reader();

void DrawMeshObj();
void Setskt();

//CameraSpacePoint m_SpacePoint[JointType::JointType_Count];
void Track();
