// CS247_Assignment2.cpp : Defines the entry point for the console application.

#include <glew.h>
#include <glut.h>
#include <freeglut_ext.h>

#include <iostream>
#include <algorithm>

#include "main.h"

std::vector<Vertex> contour;
std::vector<Vertex> vertices;
std::vector<Vertex> normals;
int min;
int max;

unsigned short data(int x, int y, int z) {
	x = x == -1 ? 0 : x == vol_dim[0] ? x - 1 : x;
	y = y == -1 ? 0 : y == vol_dim[1] ? y - 1 : y;
	z = z == -1 ? 0 : z == vol_dim[2] ? z - 1 : z;
	return data_array[x + (y * vol_dim[0]) + (z * vol_dim[0] * vol_dim[1])];
}

Vertex midpointDiff(int x, int y, int z) {
	Vertex n;
	n.x = 0.5 * (data(x + 1, y, z) - data(x - 1, y, z));
	n.y = 0.5 * (data(x, y + 1, z) - data(x, y - 1, z));
	n.z = 0.5 * (data(x, y, z + 1) - data(x, y, z - 1));
	return n;
}

void GetLinePoints(float isoValue, int x, int y, int dimx, int dimy, double a, double b, double c, double d) {
	int cell = 0;
	if (a < isoValue) cell |= 8;
	if (b < isoValue) cell |= 4;
	if (c < isoValue) cell |= 2;
	if (d < isoValue) cell |= 1;
	char edge = edgeTable2D[cell];
	Vertex p1;
	Vertex p2;
	switch (edge) {
	case 1:
		p1.x = x;
		p1.y = y + (isoValue - a) / (d - a);
		p2.x = x + (isoValue - d) / (c - d);
		p2.y = y + 1;
		break;
	case 2:
		p1.x = x + 1;
		p1.y = y + (isoValue - b) / (c - b);
		p2.x = x + (isoValue - d) / (c - d);
		p2.y = y + 1;
		break;
	case 3:
		p1.x = x;
		p1.y = y + (isoValue - a) / (d - a);
		p2.x = x + 1;
		p2.y = y + (isoValue - b) / (c - b);
		break;
	case 4:
		p1.x = x + (isoValue - a) / (b - a);
		p1.y = y;
		p2.x = x + 1;
		p2.y = y + (isoValue - b) / (c - b);
		break;
	case 5:
		p1.y = y + (isoValue - a) / (b - a);
		p1.x = x;
		p2.x = x + (isoValue - b) / (c - b);
		p2.y = y + 1;

		p1.x = p1.x * 2 / dimx - 1;
		p1.y = p1.y * 2 / dimy - 1;
		p2.x = p2.x * 2 / dimx - 1;
		p2.y = p2.y * 2 / dimy - 1;

		contour.push_back(p1);
		contour.push_back(p2);
		p1.x = x;
		p1.y = y + (isoValue - a) / (d - a);
		p2.x = x + (isoValue - d) / (c - d);
		p2.y = y + 1;
		break;
	case 6:
		p1.x = x + (isoValue - a) / (b - a);
		p1.y = y;
		p2.x = x + (isoValue - d) / (c - d);
		p2.y = y + 1;
		break;
	case 7:
		p1.x = x + (isoValue - a) / (b - a);
		p1.y = y;
		p2.x = x;
		p2.y = y + (isoValue - a) / (d - a);
		break;
	default:
		return;
	}

	p1.x = p1.x * 2 / dimx - 1;
	p1.y = p1.y * 2 / dimy - 1;
	p2.x = p2.x * 2 / dimx - 1;
	p2.y = p2.y * 2 / dimy - 1;

	contour.push_back(p1);
	contour.push_back(p2);
}

// ==========================================================================
// Handle OpenGL errors
int printOglError(char* file, int line)
{
	// Returns 1 if an OpenGL error occurred, 0 otherwise.
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}
#define printOpenGLError() printOglError((char *)__FILE__, __LINE__)

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

// ==========================================================================
// Cycle clear colors
static void NextClearColor(void)
{
	clearColor = (++clearColor) % 3;
}

static void SetClearColor(void)
{
	switch (clearColor)
	{
	case 0:
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		break;
	case 1:
		glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
		break;
	default:
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		break;
	}
}

// ==========================================================================
// GLUT
static void display2D(void)
{
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (data_loaded) {

		glEnable(GL_TEXTURE_3D);

		glActiveTexture(vol_texture);
		glBindTexture(GL_TEXTURE_3D, vol_texture);

		float slice = current_slice[current_axis] / (float)vol_dim[current_axis];

		float w = 1.0f;
		float h = 1.0f;

		glColor3f(1.0f, 1.0f, 1.0f);
		if (current_axis == 0) {

			float aspect_viewport_voldim_x = viewport_x_2D / (float)vol_dim[1];
			float aspect_viewport_voldim_y = viewport_y_2D / (float)vol_dim[2];

			float aspect = (float)vol_dim[1] / (float)vol_dim[2];

			if (aspect_viewport_voldim_x < aspect_viewport_voldim_y) {
				//maximize w
				h /= aspect;
				h /= (viewport_y_2D / (float)viewport_x_2D);

			}
			else {
				//maximize h
				w *= aspect;
				w /= (viewport_x_2D / (float)viewport_y_2D);

			}

			glBegin(GL_QUADS);
			glTexCoord3f(slice, 0.0f, 0.0f);
			glVertex3f(-w, -h, 0.0f);

			glTexCoord3f(slice, 1.0f, 0.0f);
			glVertex3f(w, -h, 0.0f);

			glTexCoord3f(slice, 1.0f, 1.0f);
			glVertex3f(w, h, 0.0f);

			glTexCoord3f(slice, 0.0f, 1.0f);
			glVertex3f(-w, h, 0.0f);
			glEnd();

		}
		else if (current_axis == 1) {

			float aspect_viewport_voldim_x = viewport_x_2D / (float)vol_dim[0];
			float aspect_viewport_voldim_y = viewport_y_2D / (float)vol_dim[2];

			float aspect = (float)vol_dim[0] / (float)vol_dim[2];

			if (aspect_viewport_voldim_x < aspect_viewport_voldim_y) {
				//maximize w
				h /= aspect;
				h /= (viewport_y_2D / (float)viewport_x_2D);

			}
			else {
				//maximize h
				w *= aspect;
				w /= (viewport_x_2D / (float)viewport_y_2D);

			}

			glBegin(GL_QUADS);
			glTexCoord3f(0.0f, slice, 0.0f);
			glVertex3f(-w, -h, 0.0f);

			glTexCoord3f(1.0f, slice, 0.0f);
			glVertex3f(w, -h, 0.0f);

			glTexCoord3f(1.0f, slice, 1.0f);
			glVertex3f(w, h, 0.0f);

			glTexCoord3f(0.0f, slice, 1.0f);
			glVertex3f(-w, h, 0.0f);
			glEnd();

		}
		else {

			float aspect_viewport_voldim_x = viewport_x_2D / (float)vol_dim[0];
			float aspect_viewport_voldim_y = viewport_y_2D / (float)vol_dim[1];

			float aspect = (float)vol_dim[0] / (float)vol_dim[1];

			if (aspect_viewport_voldim_x < aspect_viewport_voldim_y) {
				//maximize w
				h /= aspect;
				h /= (viewport_y_2D / (float)viewport_x_2D);

			}
			else {
				//maximize h
				w *= aspect;
				w /= (viewport_x_2D / (float)viewport_y_2D);

			}

			glBegin(GL_QUADS);
			glTexCoord3f(0.0f, 0.0f, slice);
			glVertex3f(-w, -h, 0.0f);

			glTexCoord3f(1.0f, 0.0f, slice);
			glVertex3f(w, -h, 0.0f);

			glTexCoord3f(1.0f, 1.0f, slice);
			glVertex3f(w, h, 0.0f);

			glTexCoord3f(0.0f, 1.0f, slice);
			glVertex3f(-w, h, 0.0f);
			glEnd();
		}

		glDisable(GL_TEXTURE_3D);

		// Overlay iso contour
		if (show_contour) {
			DrawContour(w, h);
		}
	}

	glFlush();
	glutSwapBuffers();
}

static void display3D(void)
{
	glShadeModel(GL_SMOOTH);	// gouraud shading

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_NORMALIZE); // enable OpenGL to normalize the normals

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	// specify light properties
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// object translation
	glTranslatef(0.0, 0.0, -5.0);

	// object rotation
	glRotatef(fYDiff, 1, 0, 0);
	glRotatef(fXDiff, 0, 1, 0);
	glRotatef(fZDiff, 0, 0, 1);

	float zscale = 1.0;

	if (data_loaded) {
		zscale = (vol_dim[2] / (float)vol_dim[0]);
	}

	// object scaling - assuming that x and y dimensions of dataset are the same, correct scaling of z
	glScalef(fScale, fScale, (fScale * zscale));

	// draw iso-surface that was extracted with marching cubes
	DrawSurface();

	// draw bounding box of volume
	DrawBoundingBox();

	glFlush();
	glutSwapBuffers();

	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
}

static void play(void)
{
	int currentWindow = glutGetWindow();

	if (currentWindow == window3D)
		glutSetWindow(window2D);
	else
		glutSetWindow(window3D);

	glutPostRedisplay();

	glutSetWindow(currentWindow);
	glutPostRedisplay();
}

static void key(unsigned char keyPressed, int x, int y)
{
	int currentWindow = glutGetWindow();

	switch (keyPressed) {

	case 'w':
		current_slice[current_axis] = std::min((current_slice[current_axis] + 1), vol_dim[current_axis] - 1);
		fprintf(stderr, "increasing current slice: %i\n", current_slice[current_axis]);
		MarchingSquares();
		break;
	case 's':
		current_slice[current_axis] = std::max((current_slice[current_axis] - 1), 0);
		fprintf(stderr, "decreasing current slice: %i\n", current_slice[current_axis]);
		MarchingSquares();
		break;
	case 'a':
		current_axis = ((current_axis + 1) % 3);
		fprintf(stderr, "toggling viewing axis to: %i\n", current_axis);
		MarchingSquares();
		break;
	case 'c':
		show_contour = !show_contour;
		fprintf(stderr, "toggle showing contour: %i\n", show_contour);
		MarchingSquares();
		break;
	case 'i':
		if (current_iso_value <= 0.99f) {
			current_iso_value += 0.01f;
		}
		else {
			current_iso_value = 1.0f;
		}
		fprintf(stderr, "increasing current iso value to: %f\n", current_iso_value);
		MarchingSquares();
		MarchingCubes();
		break;
	case 'k':
		if (current_iso_value >= 0.01f) {
			current_iso_value -= 0.01f;
		}
		else {
			current_iso_value = 0.0f;
		}
		fprintf(stderr, "decreasing current iso value to: %f\n", current_iso_value);
		MarchingSquares();
		MarchingCubes();
		break;
	case 'o':
		if (current_iso_value <= 0.9f) {
			current_iso_value += 0.1f;
		}
		else {
			current_iso_value = 1.0f;
		}
		fprintf(stderr, "increasing current iso value to: %f\n", current_iso_value);
		MarchingSquares();
		MarchingCubes();
		break;
	case 'l':
		if (current_iso_value >= 0.1f) {
			current_iso_value -= 0.1f;
		}
		else {
			current_iso_value = 0.0f;
		}
		fprintf(stderr, "decreasing current iso value to: %f\n", current_iso_value);
		MarchingSquares();
		MarchingCubes();
		break;
	case '1':
		LoadData("../datasets/lobster.dat");
		break;
	case '2':
		LoadData("../datasets/skewed_head.dat");
		break;
	case '3':
		LoadData("../datasets/dataset1.dat");
		break;
	case '4':
		LoadData("../datasets/dataset2.dat");
		break;
	case 'b':
		NextClearColor();
		glutSetWindow(window2D);
		SetClearColor();
		glutSetWindow(window3D);
		SetClearColor();
		break;
	case 27:
		exit(0);
		break;
	default:
		fprintf(stderr, "\nKeyboard commands:\n\n"
			"b - Toggle among background clear colors\n"
			"w - Increase current slice\n"
			"s - Decrease current slice\n"
			"a - Toggle viewing axis\n"
			"c - Toggle contour\n"
			"i - Increase iso-value by 0.01\n"
			"o - Increase iso-value by 0.1\n"
			"k - Decrease iso-value by 0.01\n"
			"l - Decrease iso-value by 0.1\n"
			"1 - Load lobster dataset\n"
			"2 - Load head dataset\n");
		break;
	}
}

static void mouse2D(int button, int state, int x, int y) {}

static void mouse3D(int button, int state, int x, int y)
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

static void motion(int x, int y)
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

static void reshape2D(int w, int h)
{
	float aspect = (float)w / (float)h;

	viewport_x_2D = w;
	viewport_y_2D = h;

	glViewport(0, 0, viewport_x_2D, viewport_y_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, viewport_x_2D, viewport_y_2D);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -5.0);
}

static void reshape3D(int w, int h)
{
	viewport_x_3D = w;
	viewport_y_3D = h;

	float vp = 0.8f;
	float aspect = (float)viewport_x_3D / (float)viewport_y_3D;

	// set viewport
	glViewport(0, 0, viewport_x_3D, viewport_y_3D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// set perspective projection
	glFrustum(-vp, vp, (-vp / aspect), (vp / aspect), 3, 10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -5.0);
}

static void special(int key, int x, int y)
{
	//handle special keys
	switch (key) {
	case GLUT_KEY_HOME: //return to inital state/view
		fXDiff = 0;
		fYDiff = 0;
		fZDiff = 0;
		xLastIncr = 0;
		yLastIncr = 0;
		fXInertia = 0;
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

// ==========================================================================
// Data
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

	//read volume dimension
	fread(vol_dim, sizeof(unsigned short), 3, fp);
	fprintf(stderr, "volume dimensions: x: %i, y: %i, z:%i \n", vol_dim[0], vol_dim[1], vol_dim[2]);

	if (data_array != NULL) {
		delete[] data_array;
	}

	size_t size = (size_t) vol_dim[0] * vol_dim[1] * vol_dim[2];
	data_array = new unsigned short[size]; //for intensity volume
	memset(data_array, 0, sizeof(unsigned short) * size);
	fread(data_array, sizeof(unsigned short), size, fp);
	fclose(fp);

	for (int i = 0; i < size; i++) {
		data_array[i] <<= 4;
		if (max < data_array[i]) {
			max = data_array[i];
		}
		if (min == 0 || min > data_array[i]) {
			min = data_array[i];
		}
	}

	current_slice[0] = vol_dim[0] / 2;
	current_slice[1] = vol_dim[1] / 2;
	current_slice[2] = vol_dim[2] / 2;

	DownloadVolumeAsTexture();

	data_loaded = true;

	MarchingSquares();
	MarchingCubes();
}

void DownloadVolumeAsTexture()
{
	// Texture only needed in 2D window, so we switch to 2D context
	int currentWindow = glutGetWindow();
	glutSetWindow(window2D);

	fprintf(stderr, "downloading volume to 3D texture\n");

	glEnable(GL_TEXTURE_3D);

	glGenTextures(1, &vol_texture);
	glBindTexture(GL_TEXTURE_3D, vol_texture);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY16, vol_dim[0], vol_dim[1], vol_dim[2], 0, GL_LUMINANCE, GL_UNSIGNED_SHORT, data_array);

	// Switch back to original context
	glutSetWindow(currentWindow);
}

int main(int argc, char* argv[])
{
	data_array = NULL;

	viewport_x_2D = 0;
	viewport_y_2D = 0;

	viewport_x_3D = 0;
	viewport_y_3D = 0;

	current_slice[0] = 0;
	current_slice[1] = 0;
	current_slice[2] = 0;
	current_axis = 2;

	fXDiff = 0;
	fYDiff = 0;
	fZDiff = 0;
	xLastIncr = 0;
	yLastIncr = 0;
	fXInertia = 0;
	fYInertia = 0;
	fXInertiaOld;
	fYInertiaOld;
	fScale = 1.0;
	xLast = -1;
	yLast = -1;
	bmModifiers = 0;
	Rotate = 1;

	data_loaded = false;

	clearColor = 0;

	show_contour = true;
	current_iso_value = 0.5f;

	vol_texture = 0;
	displayList_idx = 0;

	int gl_major, gl_minor;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	window2D = glutCreateWindow("AMCS/CS247 Scientific Visualization - Assignment 2 - 2D");
	glutPositionWindow(40, 40);

	glutIdleFunc(play);
	glutDisplayFunc(display2D);
	glutKeyboardFunc(key);
	glutReshapeFunc(reshape2D);
	glutMouseFunc(mouse2D);

	window3D = glutCreateWindow("AMCS/CS247 Scientific Visualization - Assignment 3 - 3D");
	glutPositionWindow(560, 40);

	glutIdleFunc(play);
	glutDisplayFunc(display3D);
	glutKeyboardFunc(key);
	glutSpecialFunc(special);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouse3D);
	glutMotionFunc(motion);

	// Initialize the "OpenGL Extension Wrangler" library
	glewInit();

	// query and display OpenGL version
	getGlVersion(&gl_major, &gl_minor);
	printf("GL_VERSION major=%d minor=%d\n", gl_major, gl_minor);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	NextClearColor();

	// display help
	key('?', 0, 0);

	glutMainLoop();

	return 0;
}

void MarchingSquares()
{
	// =============================================================================
	// TODO: Extract 2D surface using marching squares algorithm
	// =============================================================================
	contour.clear();
	float isoValue = (max - min) * current_iso_value + min;
	if (current_axis == 0) {
		int x = current_slice[current_axis];
		for (int y = 0; y < vol_dim[1] - 1; y++) {
			for (int z = 0; z < vol_dim[2] - 1; z++) {
				unsigned short a = data(x, y, z);
				unsigned short b = data(x, y + 1, z);
				unsigned short c = data(x, y + 1, z + 1);
				unsigned short d = data(x, y, z + 1);
				GetLinePoints(isoValue, y, z, vol_dim[1], vol_dim[2], a, b, c, d);
			}
		}
	}
	else if (current_axis == 1) {
		int y = current_slice[current_axis];
		for (int x = 0; x < vol_dim[0] - 1; x++) {
			for (int z = 0; z < vol_dim[2] - 1; z++) {
				unsigned short a = data(x, y, z);
				unsigned short b = data(x + 1, y, z);
				unsigned short c = data(x + 1, y, z + 1);
				unsigned short d = data(x, y, z + 1);
				GetLinePoints(isoValue, x, z, vol_dim[0], vol_dim[2], a, b, c, d);
			}
		}
	}
	else if (current_axis == 2) {
		int z = current_slice[current_axis];
		for (int x = 0; x < vol_dim[0] - 1; x++) {
			for (int y = 0; y < vol_dim[1] - 1; y++) {
				unsigned short a = data(x, y, z);
				unsigned short b = data(x + 1, y, z);
				unsigned short c = data(x + 1, y + 1, z);
				unsigned short d = data(x, y + 1, z);
				GetLinePoints(isoValue, x, y, vol_dim[0], vol_dim[1], a, b, c, d);
			}
		}
	}
}

float epsilon = 0.001;

void MarchingCubes()
{
	// =============================================================================
	// TODO: Extract 3D surface using marching cubes algorithm
	// =============================================================================
	uint8_t points[12][2] = {
			{0, 1}, {1, 2}, {2, 3}, {3, 0},
			{4, 5}, {5, 6}, {6, 7}, {7, 4},
			{0, 4}, {1, 5}, {2, 6}, {3, 7}
	};
	Vertex offset[8] = {
			{0, 0, 0},
			{0, 1, 0},
			{0, 1, 1},
			{0, 0, 1},
			{1, 0, 0},
			{1, 1, 0},
			{1, 1, 1},
			{1, 0, 1},
	};

	float isoValue = (max - min) * current_iso_value + min;
	vertices.clear();
	normals.clear();

	for (int x = 0; x < vol_dim[0] - 1; x++) {
		for (int y = 0; y < vol_dim[1] - 1; y++) {
			for (int z = 0; z < vol_dim[2] - 1; z++) {
				Vertex p[8];
				double val[8];
				uint8_t cubeIndex = 0;
				for (uint8_t i = 0; i < 8; i++) {
					p[i].x = x + offset[i].x;
					p[i].y = y + offset[i].y;
					p[i].z = z + offset[i].z;
					val[i] = data(p[i].x, p[i].y, p[i].z);

					if (val[i] < isoValue - epsilon) {
						cubeIndex |= 1u << i;
					}
				}

				Vertex vertlist[12];
				Vertex normaList[12];
				for (uint8_t i = 0; i < 12; i++) {
					uint16_t bit = 1u << i;
					if (edgeTable3D[cubeIndex] & bit) {
						uint8_t a = points[i][0];
						uint8_t b = points[i][1];

						Vertex& p1 = p[a];
						Vertex& p2 = p[b];
						double v1 = val[a];
						double v2 = val[b];
						Vertex n1 = midpointDiff(p1.x, p1.y, p1.z);
						Vertex n2 = midpointDiff(p2.x, p2.y, p2.z);
						float mu = (isoValue - v1) / (v2 - v1);

						Vertex& vertex = vertlist[i];
						vertex.x = p1.x + mu * (p2.x - p1.x);
						vertex.y = p1.y + mu * (p2.y - p1.y);
						vertex.z = p1.z + mu * (p2.z - p1.z);

						Vertex& normal = normaList[i];
						normal.x = n1.x + mu * (n2.x - n1.x);
						normal.y = n1.y + mu * (n2.y - n1.y);
						normal.z = n1.z + mu * (n2.z - n1.z);
					}
					else {
						vertlist[i] = {};
					}
				}
				for (int i = 0; triangleTable[cubeIndex][i] != -1; i++) {
					Vertex v1 = vertlist[triangleTable[cubeIndex][i]];
					v1.x = v1.x * 2 / vol_dim[0] - 1;
					v1.y = v1.y * 2 / vol_dim[1] - 1;
					v1.z = v1.z * 2 / vol_dim[2] - 1;
					vertices.push_back(v1);
					Vertex& n1 = normaList[triangleTable[cubeIndex][i]];
					normals.push_back(n1);
				}
			}
		}
	}
}

void DrawContour(float w, float h)
{
	// =============================================================================
	// TODO: add drawing code for your 2D contours
	// =============================================================================
	glColor3f(0.2f, 0.5f, 0.7f);
	glBegin(GL_LINES);
	for (int i = 0; i < contour.size(); i++) {
		glVertex2f(contour[i].x * w, contour[i].y * h);
	}
	glEnd();
}

void DrawSurface()
{
	// =============================================================================
	// TODO: Replace this nice teapot that has nothing to do with your
	//		 assignment, with actual drawing code for your 3D surface.
	// =============================================================================
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(0.2f, 0.5f, 0.7f);
	if (!data_loaded) glutSolidTeapot(current_iso_value);
	else {
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < vertices.size(); i++) {
			Vertex& normal = normals[i];
			Vertex& vertex = vertices[i];
			glNormal3f(normal.x, normal.y, normal.z);
			glVertex3f(vertex.x, vertex.y, vertex.z);
		}
		glEnd();
	}
}

void DrawBoundingBox(void)
{
	if (!data_loaded) {
		return;
	}

	float size = 1.0f;

	glColor3f(1.0, 1.0, 1.0);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	float A[3] = { size,  size,  size };
	float B[3] = { size,  size, -size };
	float C[3] = { size, -size, -size };
	float D[3] = { size, -size,  size };
	float E[3] = { -size,  size,  size };
	float F[3] = { -size,  size, -size };
	float G[3] = { -size, -size, -size };
	float H[3] = { -size, -size,  size };

	glBegin(GL_QUADS);

	glVertex3fv(D);
	glVertex3fv(C);
	glVertex3fv(B);
	glVertex3fv(A);

	glVertex3fv(G);
	glVertex3fv(H);
	glVertex3fv(E);
	glVertex3fv(F);

	glVertex3fv(C);
	glVertex3fv(G);
	glVertex3fv(F);
	glVertex3fv(B);

	glVertex3fv(H);
	glVertex3fv(D);
	glVertex3fv(A);
	glVertex3fv(E);

	glVertex3fv(E);
	glVertex3fv(A);
	glVertex3fv(B);
	glVertex3fv(F);

	glVertex3fv(G);
	glVertex3fv(C);
	glVertex3fv(D);
	glVertex3fv(H);

	glEnd();
}