
#ifndef __CS247_Assignment5_h__
#define __CS247_Assignment5_h__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include <algorithm>
#include <vector>

#ifdef WIN32 
#include <io.h>
#endif

#ifndef WIN32
#define GLEW_STATIC 1
#endif
#include <glew.h>
#include <glut.h>

////////////////
// Structures //
////////////////

//////////////////////
//  Global defines  //
//////////////////////
#define TIMER_FREQUENCY_MILLIS  50

//////////////////////
// Global variables //
//////////////////////

// Handle of the window we're rendering to
static GLint window;

char bmModifiers;	// keyboard modifiers (e.g. ctrl,...)

float clear_color[4]; // clearcolor for shader

// data handling
const char* filenames[3];
bool grid_data_loaded;
bool scalar_data_loaded;
unsigned short vol_dim[3]; //data dimentions 
float* vector_array;
float* scalar_fields;
float* scalar_bounds;

GLuint scalar_field_texture;
GLuint color_code_scalarfield_shader;
GLuint color_code_scalarfield_program;

int num_scalar_fields;
int num_timesteps; //stores number of time steps

int loaded_file;
int loaded_timestep;
float timestep;

int view_width, view_height; // height and width of entire view

GLuint displayList_idx;

int toggle_xy;

////////////////
// Prototypes //
////////////////

int printOglError(char* file, int line);

#define printOpenGLError() printOglError((char *)__FILE__, __LINE__)

void computeStreamline( /* start point */);

void computePathline( /* start point */);

void loadNextTimestep(void);

void LoadData(const char* base_filename);
void LoadVectorData(const char* filename);

void DownloadScalarFieldAsTexture(void);
void initGL(void);

void reset_rendering_props(void);

#endif // __CS247_Assignment5_h__
