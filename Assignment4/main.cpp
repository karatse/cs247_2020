//
// CS247_prog3
// AMCS/CS247 Programming Assignment 4
//


#include "main.h"

#include "tf_tables.h"

#include <stdio.h>
#include <math.h>
#include <algorithm>

#define BUFFER_WIDTH 1024
#define BUFFER_HEIGHT 1024

unsigned min_iso = -1;
unsigned max_iso;

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


// ==========================================================================
// Parse GL_VERSION and return the major and minor numbers in the supplied
// integers.
// If it fails for any reason, major and minor will be set to 0.
// Assumes a valid OpenGL context.
void getGlVersion(int* major, int* minor)
{
	const char* verstr = (const char*)glGetString(GL_VERSION);
	if ((verstr == NULL) || (sscanf_s(verstr, "%d.%d", major, minor) != 2)) {
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

static
void display(void)
{
	// reshape for offscreen render pass
	// set viewport
	glViewport(0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// set perspective projection
	glFrustum(-1.1, 1.1, -1.1, 1.1, 3.0, 10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// object translation
	glTranslatef(0.0, 0.0, -5.0);

	// object rotation
	glRotatef(fYDiff, 1, 0, 0);
	glRotatef(fXDiff, 0, 1, 0);
	glRotatef(fZDiff, 0, 0, 1);

	// =============================================================
	// TODO: perform raycasting steps here
	// i.e. render frontfaces and backfaces, then cast
	// =============================================================
	float max_dim = vol_dim[0];
	if (vol_dim[1] > max_dim) {
		max_dim = vol_dim[1];
	}
	if (vol_dim[2] > max_dim) {
		max_dim = vol_dim[2];
	}
	float x = vol_dim[0] / max_dim;
	float y = vol_dim[1] / max_dim;
	float z = vol_dim[2] / max_dim;
	x = y = z = 1;
	RenderBackFaces(x, y, z);
	RenderFrontFaces(x, y, z);

	RenderRaycastPass();


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
	switch (keyPressed) {
	case '1':
		LoadData("../Datasets/lobster.dat");
		break;
	case '2':
		LoadData("../Datasets/skewed_head.dat");
		break;
	case '3':
		LoadData("../Datasets/dataset1.dat");
		break;
	case '4':
		LoadData("../Datasets/dataset2.dat");
		break;
	case 'o':
		enable_lighting = 1 - enable_lighting;
		if (enable_lighting) fprintf(stderr, "lighting is now enabled\n");
		else fprintf(stderr, "lighting is now disabled\n");
		break;
	case 'g':
		enable_gm_scaling = 1 - enable_gm_scaling;
		if (enable_gm_scaling) fprintf(stderr, "gradient magnitude scaled shading is now enabled\n");
		else fprintf(stderr, "gradient magnitude scaled shading is now disabled\n");
		break;
	case 'f':
		use_dvr = 1 - use_dvr;
		if (use_dvr) fprintf(stderr, "DVR Rendering\n");
		else fprintf(stderr, "Iso Value Rendering\n");
		break;
	case 'i':
		iso_value += 0.05f;
		if (iso_value > 1.0f) {
			iso_value = 1.0f;
		}
		fprintf(stderr, "increasing iso value to: %f\n", iso_value);
		break;
	case 'k':
		iso_value -= 0.05f;
		if (iso_value < 0.0f) {
			iso_value = 0.0f;
		}
		fprintf(stderr, "decreasing iso value to: %f\n", iso_value);
		break;
	case 'a':
		ambient += 0.05f;
		if (ambient > 1.0f) {
			ambient = 1.0f;
		}
		fprintf(stderr, "increasing ambient brightness value to: %f\n", ambient);
		break;
	case 'z':
		ambient -= 0.05f;
		if (ambient < 0.0f) {
			ambient = 0.0f;
		}
		fprintf(stderr, "decreasing ambient brightness value to: %f\n", ambient);
		break;
	case 'b':
		NextClearColor();
		break;
	case ' ':
		Rotate = !Rotate;

		if (!Rotate) {
			fXInertiaOld = fXInertia;
			fYInertiaOld = fYInertia;
		}
		else {
			fXInertia = fXInertiaOld;
			fYInertia = fYInertiaOld;

			// to prevent confusion, force some rotation
			if ((fXInertia == 0.0) && (fYInertia == 0.0)) {
				fXInertia = -0.5;
			}
		}
		break;
	case 'r': //return to inital state/view
		fXDiff = 0;
		fYDiff = 35;
		fZDiff = 0;
		xLastIncr = 0;
		yLastIncr = 0;
		fXInertia = -0.5;
		fYInertia = 0;
		break;
	case '+':
		step_size /= 1.5f;
		fprintf(stderr, "stepsize is now: %f\n", step_size);
		break;
	case '-':
		step_size *= 1.5f;
		fprintf(stderr, "stepsize is now: %f\n", step_size);
		break;
	case 'w':
		tf_win_min = std::min(tf_win_min + 1, tf_win_max);
		UpdateTransferfunction();
		fprintf(stderr, "lower window boundary increased to: %d\n", tf_win_min);
		break;
	case 's':
		tf_win_min = std::max(tf_win_min - 1, 0);
		UpdateTransferfunction();
		fprintf(stderr, "lower window boundary decreased to: %d\n", tf_win_min);
		break;
	case 'e':
		tf_win_max = std::min(tf_win_max + 1, 127);
		UpdateTransferfunction();
		fprintf(stderr, "lower window boundary increased to: %d\n", tf_win_max);
		break;
	case 'd':
		tf_win_max = std::max(tf_win_max - 1, tf_win_min);
		UpdateTransferfunction();
		fprintf(stderr, "lower window boundary decreased to: %d\n", tf_win_max);
		break;
	case '5':
		DownloadTransferFunctionTexture(0);
		break;
	case '6':
		DownloadTransferFunctionTexture(1);
		break;
	case '7':
		DownloadTransferFunctionTexture(2);
		break;
	case '8':
		DownloadTransferFunctionTexture(3);
		break;
	case '9':
		DownloadTransferFunctionTexture(4);
		break;
	case '0':
		DownloadTransferFunctionTexture(5);
		break;
	case 'q':
	case 27:
		exit(0);
		break;
	default:
		fprintf(stderr, "\nKeyboard commands:\n\n"
			"1 - Load lobster dataset\n"
			"2 - Load head dataset\n"
			"p - Load test dataset\n"
			"b - Toggle among background clear colors\n"
			"o - en-/disable lighting\n"
			"g - use gradient magnitude for shading (highlight edges)\n"
			"f - switch between DVR and Iso value rendering\n"
			"i, k - in-/decrease iso value\n"
			"a, z - in-/decrease ambient brightness\n"
			"w, s - in-/decrease lower tf window value\n"
			"e, d - in-/decrease upper tf window value\n"
			"5,6,7,8,9,0 - select transferfunctions\n"
			"<home>	 - reset zoom and rotation\n"
			"r - reset rotation\n"
			"<space> or <click> - stop rotation\n"
			"<+>, <-> - in-/decrease sampling rate\n"
			"<ctrl + drag> - lock rotation around z-axis\n"
			"<arrow keys> or <drag> - rotate model\n"
			"q, <esc> - Quit\n");
		break;
	}
}



static
void timer(int value)
{

	/* Increment wrt inertia */
	if (Rotate)
	{
		fXDiff = fXDiff + fXInertia;
		fYDiff = fYDiff + fYInertia;
	}

	/* Callback */
	glutTimerFunc(TIMER_FREQUENCY_MILLIS, timer, 0);
}


static
void mouse(int button, int state, int x, int y)
{
	bmModifiers = glutGetModifiers();

	if (button == GLUT_LEFT_BUTTON) { //rotate 

		if (state == GLUT_UP) {
			xLast = -1;
			yLast = -1;

			if (xLastIncr > INERTIA_THRESHOLD) {
				fXInertia = (xLastIncr - INERTIA_THRESHOLD) * INERTIA_FACTOR;
			}

			if (-xLastIncr > INERTIA_THRESHOLD) {
				fXInertia = (xLastIncr + INERTIA_THRESHOLD) * INERTIA_FACTOR;
			}

			if (yLastIncr > INERTIA_THRESHOLD) {
				fYInertia = (yLastIncr - INERTIA_THRESHOLD) * INERTIA_FACTOR;
			}

			if (-yLastIncr > INERTIA_THRESHOLD) {
				fYInertia = (yLastIncr + INERTIA_THRESHOLD) * INERTIA_FACTOR;
			}

		}
		else {
			fXInertia = 0;
			fYInertia = 0;
		}

		xLastIncr = 0;
		yLastIncr = 0;
	}
}


static
void motion(int x, int y)
{
	if ((xLast != -1) || (yLast != -1)) { //zoom
		xLastIncr = x - xLast;
		yLastIncr = y - yLast;
		if (bmModifiers & GLUT_ACTIVE_CTRL) {
			if (xLast != -1) {
				fZDiff += xLastIncr;
				fScale += yLastIncr * SCALE_FACTOR;
			}
		}
		else {
			if (xLast != -1) { //rotate
				fXDiff += xLastIncr;
				fYDiff += yLastIncr;
			}
		}
	}

	xLast = x;
	yLast = y;
}


static
void reshape(int wid, int ht)
{
	view_width = wid;
	view_height = ht;

	float vp = 0.8f;
	float aspect = (float)wid / (float)ht;

	// set viewport
	glViewport(0, 0, wid, ht);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// set perspective projection
	glFrustum(-vp, vp, (-vp / aspect), (vp / aspect), 3, 10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -5.0);
}


static
void special(int key, int x, int y)
{
	//handle special keys
	switch (key) {
	case GLUT_KEY_HOME: //return to inital state/view
		fXDiff = 0;
		fYDiff = 35;
		fZDiff = 0;
		xLastIncr = 0;
		yLastIncr = 0;
		fXInertia = -0.5;
		fYInertia = 0;
		fScale = 1.0;
		break;
	case GLUT_KEY_LEFT:
		fXDiff--;
		break;
	case GLUT_KEY_RIGHT:
		fXDiff++;
		break;
	case GLUT_KEY_UP:
		fYDiff--;
		break;
	case GLUT_KEY_DOWN:
		fYDiff++;
		break;
	default:
		break;
	}
}


/*
 * load .dat dataset
 */
void LoadData(char* filename)
{
	fprintf(stderr, "loading data %s\n", filename);

	FILE* fp;
	fopen_s(&fp, filename, "rb"); // open file, read only, binary mode 
	if (fp == NULL) {
		fprintf(stderr, "Cannot open file %s for reading.\n", filename);
		return;
	}

	memset(vol_dim, 0, sizeof(unsigned short) * 3);

	//read volume dimensions from file
	fread(&vol_dim[0], sizeof(unsigned short), 1, fp);
	fread(&vol_dim[1], sizeof(unsigned short), 1, fp);
	fread(&vol_dim[2], sizeof(unsigned short), 1, fp);

	fprintf(stderr, "volume dimensions: x: %i, y: %i, z:%i \n", vol_dim[0], vol_dim[1], vol_dim[2]);

	if (data_array != NULL) {
		delete[] data_array;
	}

	size_t size = (size_t)vol_dim[0] * vol_dim[1] * vol_dim[2];
	// 1D array for storing volume data
	data_array = new unsigned short[size];
	// read volume data
	fread(data_array, sizeof(unsigned short), size, fp);
	// close file
	fclose(fp);
	// shift volume data by 4 bit (converting 12 bit data to 16 bit data)
	for (int i = 0; i < size; i++) {
		data_array[i] <<= 4;
		if (max_iso < data_array[i]) {
			max_iso = data_array[i];
		}
		if (data_array[i] != 0 && min_iso > data_array[i]) {
			min_iso = data_array[i];
		}
	}

	// download data into texture
	DownloadVolumeAsTexture();

	data_loaded = true;
}


/*
 * download data to 3D texture
 */
void DownloadVolumeAsTexture()
{
	fprintf(stderr, "downloading volume to 3D texture\n");

	glEnable(GL_TEXTURE_3D);

	// generate and bind 3D texture
	glGenTextures(1, &vol_texture);
	glBindTexture(GL_TEXTURE_3D, vol_texture);

	// set necessary texture parameters
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	//download texture in correct format (our data only has one channel of unsigned short data, therefore we use GL_INTENSITY16)
	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY16, vol_dim[0], vol_dim[1], vol_dim[2], 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, data_array);

	glDisable(GL_TEXTURE_3D);
}


/*
 * update tf based on defined window
 */
void UpdateTransferfunction(void)
{

	// =============================================================
	// TODO: update custom transferfunction tf0 here
	// =============================================================
	float size = tf_win_max - tf_win_min + 1;
	for (int i = 0; i < 128; i++)
	{
		float val;
		if (i < tf_win_min) {
			val = 0;
		}
		else if (i >= tf_win_max) {
			val = 1;
		}
		else {
			val = i / size;
		}

		tf0[4 * i + 0] = val;
		tf0[4 * i + 1] = val;
		tf0[4 * i + 2] = val;
		tf0[4 * i + 3] = val;
	}

	DownloadTransferFunctionTexture(0);
}


/*
 * download tf data to 1D texture
 */
void DownloadTransferFunctionTexture(int tf_id)
{
	fprintf(stderr, "downloading transfer function to 1D texture\n");

	glEnable(GL_TEXTURE_1D);

	// generate and bind 1D texture
	glGenTextures(1, &tf_texture);
	glBindTexture(GL_TEXTURE_1D, tf_texture);

	// set necessary texture parameters
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	//download texture in correct format
	switch (tf_id) {
	case 0:
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 128, 0, GL_RGBA, GL_FLOAT, tf0);
		break;
	case 1:
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 2, 0, GL_RGBA, GL_FLOAT, tf1);
		break;
	case 2:
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 4, 0, GL_RGBA, GL_FLOAT, tf2);
		break;
	case 3:
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 12, 0, GL_RGBA, GL_FLOAT, tf3);
		break;
	case 4:
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 12, 0, GL_RGBA, GL_FLOAT, tf4);
		break;
	case 5:
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 12, 0, GL_RGBA, GL_FLOAT, tf5);
		break;
	default:
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 128, 0, GL_RGBA, GL_FLOAT, tf0);
		break;
	}

	glDisable(GL_TEXTURE_1D);
}

void drawQuads(float dim_x, float dim_y, float dim_z)
{
	float points[][3] = {
		{dim_x, dim_y, dim_z},
		{dim_x, dim_y, -dim_z},
		{dim_x, -dim_y, -dim_z},
		{dim_x, -dim_y, dim_z},
		{-dim_x, dim_y, dim_z},
		{-dim_x, dim_y, -dim_z},
		{-dim_x, -dim_y, -dim_z},
		{-dim_x, -dim_y, dim_z}
	};
	int quads[][4] = {
		{3, 2, 1, 0},
		{6, 7, 4, 5},
		{2, 6, 5, 1},
		{7, 3, 0, 4},
		{4, 0, 1, 5},
		{6, 2, 3, 7}
	};

	glBegin(GL_QUADS);
	for (size_t i = 0; i < 6; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			float* v = points[quads[i][j]];
			glColor3fv(v);
			glMultiTexCoord3fv(GL_TEXTURE1, v);
			glVertex3fv(v);
		}
	}
	glEnd();
}

/*
 * render backfaces to start raycasting
 */
void RenderBackFaces(float dim_x, float dim_y, float dim_z)
{
	enableRenderToBuffer(backface_buffer);

	// =============================================================
	// TODO: render your backfaces here
	// result gets piped to backface_buffer automatically
	// =============================================================
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	drawQuads(dim_x, dim_y, dim_z);
	glDisable(GL_CULL_FACE);

	disableRenderToBuffer();
}


/*
 * render frontfaces to start raycasting
 */
void RenderFrontFaces(float dim_x, float dim_y, float dim_z)
{
	enableRenderToBuffer(frontface_buffer);

	// =============================================================
	// TODO: render your backfaces here
	// result gets piped to frontface_buffer automatically
	// =============================================================
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	drawQuads(dim_x, dim_y, dim_z);
	glDisable(GL_CULL_FACE);

	disableRenderToBuffer();
}


void RenderRaycastPass(void)
{
	// switch for dvr and iso shader programs
	GLuint shader_program = use_dvr ? raycast_dvr_shader_program : raycast_iso_shader_program;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// enable shader
	glUseProgram(shader_program);

	// example for a texture uniform
	int tf_location = glGetUniformLocation(shader_program, "transferfunction");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, tf_texture);
	glUniform1i(tf_location, 0);

	// example for a scalar/vector uniform
	int enable_lighting_loc = glGetUniformLocation(shader_program, "enable_lighting");
	glUniform1i(enable_lighting_loc, enable_lighting);

	// =============================================================
	// TODO: add additional uniforms as needed
	// i.e. front and backface buffers ( frontface_buffer, backface_buffer )
	// =============================================================
	int size_step_loc = glGetUniformLocation(shader_program, "step_size");
	glUniform1f(size_step_loc, step_size);

	int brightness_loc = glGetUniformLocation(shader_program, "brightness");
	glUniform1f(brightness_loc, ambient);

	int isoval_loc = glGetUniformLocation(shader_program, "iso_value");
	glUniform1f(isoval_loc, iso_value);

	int vol_texture_loc = glGetUniformLocation(shader_program, "vol_texture");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, vol_texture);
	glUniform1i(vol_texture_loc, 1);

	int front_texture_loc = glGetUniformLocation(shader_program, "front_texture");
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, frontface_buffer);
	glUniform1i(front_texture_loc, 2);

	int back_texture_loc = glGetUniformLocation(shader_program, "back_texture");
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, backface_buffer);
	glUniform1i(back_texture_loc, 3);

	// Render screen filling quad
	// use squared centered quad to not disturb ratio
	int x[2] = { 0, 0 };
	int y[2] = { 0, 0 };
	int dim, off;
	if (view_width < view_height) {

		dim = view_width;
		off = (view_height - view_width) / 2;

		x[0] = 0;
		x[1] = dim;
		y[0] = off;
		y[1] = dim + off;

	}
	else {

		dim = view_height;
		off = (view_width - view_height) / 2;

		x[0] = off;
		x[1] = dim + off;
		y[0] = 0;
		y[1] = dim;
	}

	glViewport(0, 0, view_width, view_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, view_width, 0, view_height);
	glMatrixMode(GL_MODELVIEW);

	//draw_fullscreen_quad
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_QUADS);

	// red screen means your shader is broken
	glColor3f(1.0, 0.0, 0.0);

	glTexCoord2i(0, 0); glVertex2i(x[0], y[0]);
	glTexCoord2i(1, 0); glVertex2i(x[1], y[0]);
	glTexCoord2i(1, 1); glVertex2i(x[1], y[1]);
	glTexCoord2i(0, 1); glVertex2i(x[0], y[1]);

	glEnd();
	glEnable(GL_DEPTH_TEST);

	//disable shader
	glUseProgram(0);
}


// for debugging purposes
void RenderBufferToScreen(GLuint buffer) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, buffer);

	// use squared centered quad to not disturb ratio
	int x[2] = { 0, 0 };
	int y[2] = { 0, 0 };
	int dim, off;
	if (view_width < view_height) {

		dim = view_width;
		off = (view_height - view_width) / 2;

		x[0] = 0;
		x[1] = dim;
		y[0] = off;
		y[1] = dim + off;

	}
	else {

		dim = view_height;
		off = (view_width - view_height) / 2;

		x[0] = off;
		x[1] = dim + off;
		y[0] = 0;
		y[1] = dim;
	}

	glViewport(0, 0, view_width, view_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, view_width, 0, view_height);
	glMatrixMode(GL_MODELVIEW);

	//draw_fullscreen_quad
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_QUADS);

	glTexCoord2i(0, 0); /*glColor3f( 0.0f, 0.0f, 1.0f );*/ glVertex2i(x[0], y[0]);
	glTexCoord2i(1, 0); /*glColor3f( 1.0f, 0.0f, 1.0f );*/ glVertex2i(x[1], y[0]);
	glTexCoord2i(1, 1); /*glColor3f( 1.0f, 1.0f, 1.0f );*/ glVertex2i(x[1], y[1]);
	glTexCoord2i(0, 1); /*glColor3f( 0.0f, 1.0f, 1.0f );*/ glVertex2i(x[0], y[1]);

	glEnd();
	glEnable(GL_DEPTH_TEST);

	glDisable(GL_TEXTURE_2D);
}

/*
 * enable buffer to render to
 */
void enableRenderToBuffer(GLuint buffer)
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, buffer, 0);
}

/*
 * unbinds buffer
 */
void disableRenderToBuffer()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
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
		glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE) {

		printf("Driver does not support OpenGL Shading Language");
		exit(1);
	}

	// Create the to FBO's one for the backsides and one for the frontsides
	glGenFramebuffersEXT(1, &framebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);

	glGenTextures(1, &backface_buffer);
	glBindTexture(GL_TEXTURE_2D, backface_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backface_buffer, 0);


	glGenTextures(1, &frontface_buffer);
	glBindTexture(GL_TEXTURE_2D, frontface_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, frontface_buffer, 0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	// Create Shader Program
	LoadAndLinkShaders();

	// download initial transfer function
	DownloadTransferFunctionTexture(0);
}

char* textFileRead(char* fn) {

	FILE* fp;
	char* content = NULL;

	int count = 0;

	if (fn != NULL) {
		fopen_s(&fp, fn, "rt");

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


void LoadAndLinkShaders(void) {

	char* fs = NULL;

	raycast_dvr_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	raycast_iso_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	fs = textFileRead("raycast_dvr.glsl");

	if (fs == NULL) {

		printf("DVR shader file not found\n");
		exit(1);
	}

	const char* f1 = fs;

	glShaderSource(raycast_dvr_fragment_shader, 1, &f1, NULL);

	free(fs);

	fs = textFileRead("raycast_iso.glsl");

	if (fs == NULL) {

		printf("ISO shader file not found\n");
		exit(1);
	}

	const char* f2 = fs;

	glShaderSource(raycast_iso_fragment_shader, 1, &f2, NULL);

	free(fs);

	glCompileShader(raycast_dvr_fragment_shader);
	glCompileShader(raycast_iso_fragment_shader);

	raycast_dvr_shader_program = glCreateProgram();
	raycast_iso_shader_program = glCreateProgram();
	glAttachShader(raycast_dvr_shader_program, raycast_dvr_fragment_shader);
	glAttachShader(raycast_iso_shader_program, raycast_iso_fragment_shader);

	glLinkProgram(raycast_dvr_shader_program);
	glLinkProgram(raycast_iso_shader_program);
}


/******************************************************************************/
/*
 * Main
 *
 ******************************************************************************/

int main(int argc, char** argv)
{
	// init variables
	fXDiff = 206;		//rotation
	fYDiff = 16;
	fZDiff = 10;
	xLastIncr = 0;		//last difference in mouse movement
	yLastIncr = 0;
	fXInertia = -0.5;	//inertia for keeping rotating even after mouse movement
	fYInertia = 0;
	fXInertiaOld;		//old inertia
	fYInertiaOld;
	fScale = 1.0;		//scale/zoom factor
	xLast = -1;			//last mouse position
	yLast = -1;
	bmModifiers;		//keyboard modifiers (e.g. ctrl,...)
	Rotate = 1;			//auto-rotate
	view_width = 0;
	view_height = 0;

	use_dvr = 1;

	iso_value = 0.5f;

	enable_lighting = 0;
	enable_gm_scaling = 0;
	ambient = 0.5;
	step_size = 0.0025f;

	tf_win_min = 0;
	tf_win_max = 127;

	for (int i = 0; i < 128; i++) {

		tf0[4 * i + 0] = (float)(i) / 128.0f;
		tf0[4 * i + 1] = (float)(i) / 128.0f;
		tf0[4 * i + 2] = (float)(i) / 128.0f;
		tf0[4 * i + 3] = (float)(i) / 128.0f;
	}

	data_array = NULL;
	data_loaded = false;

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

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	NextClearColor();

	// display help
	key('?', 0, 0);

	glutMainLoop();

	return 0;
}

