//
// CS247_Assignment6
// AMCS/CS247 Programming Assignment 6
//

#include <cstdio>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>

#include "main.h"

int printOglError(char* file, int line)
{
	//
	// Returns 1 if an OpenGL error occurred, 0 otherwise.
	//
	GLenum glErr;
	int retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}

/***************************************************************************/
/* Parse GL_VERSION and return the major and minor numbers in the supplied
 * integers.
 * If it fails for any reason, major and minor will be set to 0.
 * Assumes a valid OpenGL context.
*/

void getGlVersion(int* major, int* minor)
{
	const char* verstr = (const char*)glGetString(GL_VERSION);
	if ((verstr == NULL) || (sscanf(verstr, "%d.%d", major, minor) != 2)) {
		*major = *minor = 0;
		fprintf(stderr, "Invalid GL_VERSION format!!!\n");
	}
}

/*
rotating background color
*/
static void NextClearColor(void)
{
	static int color = 0;

	switch (color++)
	{
	case 0:
		clear_color[0] = 0.0f;
		clear_color[1] = 0.0f;
		clear_color[2] = 0.0f;
		clear_color[3] = 1.0f;
		break;
	case 1:
		clear_color[0] = 0.2f;
		clear_color[1] = 0.2f;
		clear_color[2] = 0.3f;
		clear_color[3] = 1.0f;
		break;
	default:
		clear_color[0] = 0.7f;
		clear_color[1] = 0.7f;
		clear_color[2] = 0.7f;
		clear_color[3] = 1.0f;
		color = 0;
		break;
	}
	glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
}

/******************************************************************************/
/*
 * GLUT glue
 *
 ******************************************************************************/
void enableLIC(void) {

	glUseProgram(lic_program);

	int scalar_field_3D_location = glGetUniformLocation(lic_program, "scalar_field_3D");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, scalar_field_texture_3D);
	glUniform1i(scalar_field_3D_location, 1);

	int noise_field_location = glGetUniformLocation(lic_program, "noise_field");
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noise_field_texture);
	glUniform1i(noise_field_location, 2);

	int vector_field_3D_location = glGetUniformLocation(lic_program, "vector_field_3D");
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_3D, vector_field_texture_3D);
	glUniform1i(vector_field_3D_location, 3);

	glActiveTexture(GL_TEXTURE0);

	// TODO: =======================================================================
	//
	// Add needed uniforms here (you probably need at least the size of a texel, 
	// a flag whether to overlay a scalar field or not, the LIC kernel size ...
	//
	// =============================================================================

}

void disableLIC(void) {

	glUseProgram(0);
}

static
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	enableLIC();

	glBegin(GL_QUADS);
	glColor3f(0.8f, 0.0f, 0.0f);
	glTexCoord2f(0, 0); glVertex2f(0, 0);
	glTexCoord2f(1, 0); glVertex2f(1, 0);
	glTexCoord2f(1, 1); glVertex2f(1, 1);
	glTexCoord2f(0, 1); glVertex2f(0, 1);
	glEnd();

	disableLIC();

	glFlush();
	glutSwapBuffers();
}

static
void play(void)
{
	int thisTime = glutGet(GLUT_ELAPSED_TIME);

	glutPostRedisplay();
}

static
void key(unsigned char keyPressed, int x, int y) // key handling
{
	char* status[2];
	status[0] = "disabled";
	status[1] = "enabled";

	switch (keyPressed) {
	case '1':
		toggle_xy = 0;
		LoadData(filenames[0]);
		loaded_file = 0;
		fprintf(stderr, "Loading ");
		fprintf(stderr, filenames[0]);
		fprintf(stderr, " dataset.\n");
		break;
	case '2':
		toggle_xy = 0;
		LoadData(filenames[1]);
		loaded_file = 1;
		fprintf(stderr, "Loading ");
		fprintf(stderr, filenames[1]);
		fprintf(stderr, " dataset.\n");
		break;
	case '3':
		toggle_xy = 1;
		LoadData(filenames[2]);
		loaded_file = 2;
		fprintf(stderr, "Loading ");
		fprintf(stderr, filenames[2]);
		fprintf(stderr, " dataset.\n");
		break;
	case '0':
		if (num_timesteps > 1) {
			NextTimestep();
			fprintf(stderr, "Timestep %d.\n", current_timestep);
		}
		break;
	case 'r':
		scalar_overlay = 1 - scalar_overlay;
		fprintf(stderr, "Scalar field in background %s.\n", status[scalar_overlay]);
		break;
	case 'i':
		enable_autocycle = 1 - enable_autocycle;
		fprintf(stderr, "Autocycling %s.\n", status[enable_autocycle]);
		break;
	case 'n':
		scalar_field_id = (scalar_field_id + 1) % num_scalar_fields;
		DownloadScalarFieldAs3DTexture();
		fprintf(stderr, "Next scalar field.\n");
		break;
	case 'v':
		FreeTextures();
		ResetRenderingProperties();
		InitTextures();
		break;
	case 'b':
		NextClearColor();
		fprintf(stderr, "Next clear color.\n");
		break;
	case 'q':
	case 27:
		exit(0);
		break;
	default:
		fprintf(stderr, "\nKeyboard commands:\n\n"
			"1, load %s dataset\n"
			"2, load %s dataset\n"
			"3, load %s dataset\n"
			"0, cycle through timesteps\n"
			"i, toggle autocycling between timesteps\n"
			"r, enable scalar field visualization\n"
			"n, next scalar field\n"
			"b, switch background color\n"
			"q, <esc> - Quit\n",
			filenames[0], filenames[1], filenames[2]);
		break;
	}
}

static
void timer(int value)
{
	if (enable_autocycle && num_timesteps > 1) {

		NextTimestep();
	}

	/* Callback */
	glutTimerFunc(TIMER_FREQUENCY_MILLIS, timer, 0);
}

static
void mouse(int button, int state, int x, int y)
{
}

static
void motion(int x, int y)
{
}

static
void reshape(int wid, int ht)
{
	view_width = wid;
	view_height = ht;

	// set viewport
	glViewport(0, 0, wid, ht);

	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	gluOrtho2D(0.0, 1.0, 0.0, 1.0);
}

static
void special(int key, int x, int y)
{
	//handle special keys
	switch (key) {
	case GLUT_KEY_HOME: //return to inital state/view
		break;
	case GLUT_KEY_LEFT:
		break;
	case GLUT_KEY_RIGHT:
		break;
	case GLUT_KEY_UP:
		break;
	case GLUT_KEY_DOWN:
		break;
	default:
		break;
	}
}

void NextTimestep(void)
{
	if (num_timesteps == 0) return;

	current_timestep = (current_timestep + 1) % num_timesteps;
}

/*
 * load .gri dataset
 */
void LoadData(char* base_filename)
{
	//reset
	ResetRenderingProperties();

	// free textures
	FreeTextures();

	//reset data
	if (scalar_fields != NULL) {

		delete[] scalar_fields;
		scalar_fields = NULL;

		delete[] scalar_bounds;
		scalar_bounds = NULL;
	}

	if (vector_fields != NULL) {

		delete[] vector_fields;
		vector_fields = NULL;
		delete[] vector_fields_tex;
		vector_fields_tex = NULL;
	}

	char filename[80];
	strcpy(filename, base_filename);
	strcat(filename, ".gri");

	fprintf(stderr, "loading grid file %s\n", filename);

	// open grid file, read only, binary mode 
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open file %s for reading.\n", filename);
		return;
	}

	// read header
	char header[40];
	fread(header, sizeof(char), 40, fp);
	sscanf(header, "SN4DB %d %d %d %d %d %f",
		&vol_dim[0], &vol_dim[1], &vol_dim[2],
		&num_scalar_fields, &num_timesteps, &timestep);

	fprintf(stderr, "dimensions: x: %d, y: %d, z: %d.\n", vol_dim[0], vol_dim[1], vol_dim[2]);
	fprintf(stderr, "additional info: # scalar fields: %d, # timesteps: %d, timestep: %f.\n", num_scalar_fields, num_timesteps, timestep);

	scalar_fields = new float[vol_dim[0] * vol_dim[1] * num_timesteps * num_scalar_fields];
	scalar_bounds = new float[2 * num_timesteps * num_scalar_fields];
	vector_fields = new vec3[vol_dim[0] * vol_dim[1] * num_timesteps];
	vector_fields_tex = new vec3[vol_dim[0] * vol_dim[1] * num_timesteps];

	if (num_timesteps <= 1) {

		current_timestep = 0;

		char dat_filename[80];
		strcpy(dat_filename, base_filename);
		strcat(dat_filename, ".dat");

		LoadVectorData(dat_filename);

	}
	else {

		for (int i = 0; i < num_timesteps; i++) {

			current_timestep = i;

			char dat_filename[80];
			sprintf(dat_filename, "%s.%.5d.dat", base_filename, current_timestep);

			LoadVectorData(dat_filename);
		}

		current_timestep = 0;
	}

	CreateNoiseField();

	InitTextures();

	glutReshapeWindow(vol_dim[0], vol_dim[1]);

	grid_data_loaded = true;
}

/*
 * load .dat dataset
 */
void LoadVectorData(const char* filename)
{
	fprintf(stderr, "loading scalar file %s\n", filename);

	// open data file, read only, binary mode 
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open file %s for reading.\n", filename);
		return;
	}

	int data_size = vol_dim[0] * vol_dim[1];

	int num_total_fields = num_scalar_fields + 3;

	// read scalar data
	float* tmp = new float[data_size * num_total_fields];
	fread(tmp, sizeof(float), (data_size * (num_total_fields)), fp);

	// close file
	fclose(fp);

	// vec min max;
	float min_val = 99999.9f;
	float max_val = 0.0f;

	int t_step_off = current_timestep * data_size;

	// copy vector data
	for (int i = 0; i < data_size; i++) {

		vec3 val;
		if (toggle_xy) {
			val.x = tmp[i * num_total_fields + 1];  //toggle x and y components in vector field
			val.y = tmp[i * num_total_fields + 0];
			val.z = tmp[i * num_total_fields + 2];

		}
		else {
			val.x = tmp[i * num_total_fields + 0];
			val.y = tmp[i * num_total_fields + 1];
			val.z = tmp[i * num_total_fields + 2];
		}

		vector_fields[t_step_off + i] = val;

		min_val = std::min(std::min(std::min(val.x, val.y), val.z), min_val);
		max_val = std::max(std::max(std::max(val.x, val.y), val.z), max_val);
	}

	// scale vectors		
	// scale between [0..1] where 1 is original zero
	// the boundary of the bigger abs border will be used to scale
	// meaning one boundary will likely not be hit i.e real scale
	// for -50..100 will be [0.25..1.0]
	if (min_val < 0.0 && max_val > 0.0) {

		float scale = 0.5f / std::max(-min_val, max_val);

		for (int j = t_step_off; j < t_step_off + data_size; j++) {

			vector_fields_tex[j].x = 0.5f + vector_fields[j].x * scale;
			vector_fields_tex[j].y = 0.5f + vector_fields[j].y * scale;
			vector_fields_tex[j].z = 0.5f + vector_fields[j].z * scale;
		}
		// scale between [0..1]
	}
	else {

		float sign = max_val <= 0.0 ? -1.0f : 1.0f;
		float scale = 1.0f / (max_val - min_val) * sign;

		for (int j = t_step_off; j < t_step_off + data_size; j++) {

			vector_fields_tex[j].x = (vector_fields[j].x - min_val) * scale;
			vector_fields_tex[j].y = (vector_fields[j].y - min_val) * scale;
			vector_fields_tex[j].z = (vector_fields[j].z - min_val) * scale;
		}
	}


	// copy scalar data
	int sc_field_size = data_size * num_timesteps;
	for (int i = 0; i < num_scalar_fields; i++) {

		int off = i * sc_field_size + t_step_off;

		min_val = 99999.9f;
		max_val = 0.0f;

		for (int j = 0; j < data_size; j++) {

			float val = tmp[j * num_total_fields + 3 + i];

			scalar_fields[off + j] = val;

			min_val = std::min(val, min_val);
			max_val = std::max(val, max_val);
		}

		scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 0] = min_val;
		scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 1] = max_val;
	}


	// normalize scalars
	for (int i = 0; i < num_scalar_fields; i++) {

		int off = i * sc_field_size + t_step_off;

		float lower_bound = scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 0];
		float upper_bound = scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 1];

		// scale between [0..1] where 1 is original zero
		// the boundary of the bigger abs border will be used to scale
		// meaning one boundary will likely not be hit i.e real scale
		// for -50..100 will be [0.25..1.0]
		if (lower_bound < 0.0 && upper_bound > 0.0) {

			float scale = 0.5f / std::max(-lower_bound, upper_bound);

			for (int j = 0; j < data_size; j++) {

				scalar_fields[off + j] = 0.5f + scalar_fields[off + j] * scale;
			}
			scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 0] = 0.5f + scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 0] * scale;
			scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 1] = 0.5f + scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 1] * scale;


			// scale between [0..1]
		}
		else {

			float sign = upper_bound <= 0.0 ? -1.0f : 1.0f;

			float scale = 1.0f / (upper_bound - lower_bound) * sign;

			for (int j = 0; j < data_size; j++) {

				scalar_fields[off + j] = (scalar_fields[off + j] - lower_bound) * scale;
			}
			scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 0] = (scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 0] + lower_bound) * scale; //should be 0.0
			scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 1] = (scalar_bounds[i * 2 * num_timesteps + 2 * current_timestep + 1] + lower_bound) * scale; //should be 1.0
		}
	}

	delete[] tmp;

	scalar_data_loaded = true;
}

void CreateNoiseField(void)
{
	if (noise_field != NULL) {

		delete[] noise_field;
		noise_field = NULL;
	}
	int img_size = vol_dim[0] * vol_dim[1];
	noise_field = new float[img_size];

	// TODO: =======================================================================
	//
	// Initialize white noise field
	//
	// =============================================================================

}

void InitTextures(void) {

	DownloadVectorFieldAs3DTexture();
	DownloadScalarFieldAs3DTexture();
	DownloadNoiseFieldAs2DTexture();
}

void FreeTextures(void) {

	glDeleteTextures(1, &scalar_field_texture_3D);
	glDeleteTextures(1, &vector_field_texture_3D);
	glDeleteTextures(1, &noise_field_texture);
}

void DownloadScalarFieldAs3DTexture(void)
{
	fprintf(stderr, "downloading scalar field to 3D texture\n");

	// generate and bind 2D texture
	glGenTextures(1, &scalar_field_texture_3D);
	glBindTexture(GL_TEXTURE_3D, scalar_field_texture_3D);

	// set necessary texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	int scalar_field_off = scalar_field_id * vol_dim[0] * vol_dim[1] * num_timesteps;// + vol_dim[ 0 ] * vol_dim[ 1 ] * current_timestep;

	//download texture in correct format
	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY16, vol_dim[0], vol_dim[1], num_timesteps, 0, GL_LUMINANCE, GL_FLOAT, &(scalar_fields[scalar_field_off]));
}

void DownloadNoiseFieldAs2DTexture(void)
{
	fprintf(stderr, "downloading noise field to 2D texture\n");

	// generate and bind 2D texture
	glGenTextures(1, &noise_field_texture);
	glBindTexture(GL_TEXTURE_2D, noise_field_texture);

	// set necessary texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	//download texture in correct format
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE16, vol_dim[0], vol_dim[1], 0, GL_LUMINANCE, GL_FLOAT, noise_field);
}

void DownloadVectorFieldAs3DTexture(void)
{
	fprintf(stderr, "downloading vector field to 3D texture\n");

	// generate and bind 2D texture
	glGenTextures(1, &vector_field_texture_3D);
	glBindTexture(GL_TEXTURE_3D, vector_field_texture_3D);

	// set necessary texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	//download texture in correct format
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16, vol_dim[0], vol_dim[1], num_timesteps, 0, GL_RGB, GL_FLOAT, vector_fields_tex);
}

char* textFileRead(char* fn) {

	FILE* fp;
	char* content = NULL;

	int count = 0;

	if (fn != NULL) {
		fp = fopen(fn, "rt");

		if (fp != NULL) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char*)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}

void LoadAndLinkShader(void) {

	char* fs = NULL;

	lic_shader = glCreateShader(GL_FRAGMENT_SHADER);

	fs = textFileRead("LIC.glsl");

	if (fs == NULL) {

		printf("Shader file not found\n");
		exit(1);
	}

	const char* f1 = fs;

	glShaderSource(lic_shader, 1, &f1, NULL);

	free(fs);

	glCompileShader(lic_shader);

	lic_program = glCreateProgram();
	glAttachShader(lic_program, lic_shader);

	glLinkProgram(lic_program);
}

void initGL(void) {

	int gl_major, gl_minor;

	// Initialize the "OpenGL Extension Wrangler" library
	printf("glew init\n");
	GLenum err = glewInit();

	// query and display OpenGL version
	getGlVersion(&gl_major, &gl_minor);
	printf("GL_VERSION major=%d minor=%d\n", gl_major, gl_minor);

	// initialize all the OpenGL extensions 
	if (glewGetExtension("GL_EXT_framebuffer_object"))
		printf("GL_EXT_framebuffer_object support\n");

	if (GL_ARB_multitexture)
		printf("GL_ARB_multitexture support\n");

	if (glewGetExtension("GL_ARB_fragment_shader") != GL_TRUE ||
		glewGetExtension("GL_ARB_shader_objects") != GL_TRUE ||
		glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE)
	{
		printf("Driver does not support OpenGL Shading Language");
		exit(1);
	}

	LoadAndLinkShader();

	glDisable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ResetRenderingProperties(void)
{
	num_scalar_fields = 0;
	scalar_field_id = 0;

	scalar_overlay = 0;

	enable_autocycle = 0;
}

/******************************************************************************/
/*
 * Main
 *
 ******************************************************************************/

int main(int argc, char** argv)
{
	// init variables
	view_width = 0;
	view_height = 0;

	toggle_xy = 0;

	ResetRenderingProperties();

	vector_fields = NULL;
	vector_fields_tex = NULL;
	scalar_fields = NULL;
	scalar_bounds = NULL;
	grid_data_loaded = false;
	scalar_data_loaded = false;

	noise_field = NULL;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	window = glutCreateWindow("AMCS/CS247 Scientific Visualization");

	glutIdleFunc(play);
	glutDisplayFunc(display);
	glutKeyboardFunc(key);
	glutReshapeFunc(reshape);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutSpecialFunc(special);

	glutTimerFunc(TIMER_FREQUENCY_MILLIS, timer, 0);

	// init OpenGL
	initGL();

	NextClearColor();

	filenames[0] = "../Datasets/block/c_block";
	filenames[1] = "../Datasets/tube/tube";
	filenames[2] = "../Datasets/hurricane/hurricane_p_tc";

	LoadData(filenames[0]);
	loaded_file = 0;

	// display help
	key('?', 0, 0);

	glutMainLoop();

	return 0;
}