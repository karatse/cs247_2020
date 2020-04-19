#ifndef __CS247_ASSIGNMENT6_h__
#define __CS247_ASSIGNMENT6_h__

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

struct vec3 {
	float	x;
	float	y;
	float	z;
};

struct ivec3 {
	int	x;
	int	y;
	int	z;
};

struct vec2 {
	float	x;
	float	y;
};

struct ivec2 {
	int	x;
	int	y;
};

//////////////////////
//  Global defines  //
//////////////////////
#define TIMER_FREQUENCY_MILLIS  20

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
unsigned short vol_dim[3];

vec3* vector_fields;
vec3* vector_fields_tex;
float* scalar_fields;
float* scalar_bounds;
float* noise_field;
float* pink_noise_fields;

GLuint scalar_field_texture_3D;
GLuint vector_field_texture_3D;
GLuint advection_texture;
GLuint noise_field_texture;
GLuint noise_field_texture_3D;
GLuint lic_shader;
GLuint lic_program;

int num_scalar_fields;
int scalar_field_id;

int num_timesteps;
int current_timestep;
float timestep;

int loaded_file;

int view_width, view_height; // height and width of entire view

int toggle_xy;

int enable_autocycle;

int scalar_overlay;

////////////////
// Prototypes //
////////////////

int printOglError(char* file, int line);

#define printOpenGLError() printOglError((char *)__FILE__, __LINE__)

void NextTimestep(void);

void LoadData(const char* base_filename);
void LoadVectorData(const char* filename);
void CreateNoiseField(void);

void InitTextures(void);
void FreeTextures(void);
void DownloadScalarFieldAs3DTexture(void);
void DownloadNoiseFieldAs2DTexture(void);
void DownloadVectorFieldAs3DTexture(void);
void initGL(void);

void ResetRenderingProperties(void);

#endif // __CS247_ASSIGNMENT6_h__
