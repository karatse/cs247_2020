// CS247_Assignment1.cpp : Defines the entry point for the console application.

#include <glew.h>
#include <glut.h>

#include <iostream>
#include <algorithm>

// ==========================================================================
// Declarations
int printOglError(char *file, int line);
void LoadData(char* filename);
void DownloadVolumeAsTexture();

unsigned short* data_array;
unsigned short vol_dim[ 3 ];
GLuint vol_texture;
int viewport_x, viewport_y;
int current_axis;
int current_slice[ 3 ];
bool data_loaded;

// ==========================================================================
// Handle of the window we're rendering to
static GLint window;

// ==========================================================================
// Handle OpenGL errors
int printOglError(char *file, int line)
{
	// Returns 1 if an OpenGL error occurred, 0 otherwise.
	GLenum glErr;
	int retCode = 0;

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
void getGlVersion( int *major, int *minor )
{
	const char* verstr = (const char*)glGetString( GL_VERSION );
	if( (verstr == NULL) || (sscanf_s( verstr, "%d.%d", major, minor ) != 2) ) {
		*major = *minor = 0;
		fprintf( stderr, "Invalid GL_VERSION format!!!\n" );
	}
}

// ==========================================================================
// Cycle clear colors
static void NextClearColor(void)
{
	static int color = 0;

	switch( color++ )
	{
		case 0:
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			break;
		case 1:
			glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
			break;
		default:
			glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
			color = 0;
			break;
	}
}

// ==========================================================================
// GLUT
static void display(void)
{
	glLoadIdentity();

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// ================================================================================
	// TODO: Draw with OpenGL
	// ================================================================================



	glFlush();
	glutSwapBuffers();
}

static void play(void)
{
	int thisTime = glutGet(GLUT_ELAPSED_TIME);

	glutPostRedisplay();
}

static void key(unsigned char keyPressed, int x, int y)
{
	switch ( keyPressed ) {

		case 'w':
			current_slice[ current_axis ] = std::min( (current_slice[ current_axis ] + 1),  vol_dim[ current_axis ] - 1 );
			fprintf( stderr, "increasing current slice: %i\n", current_slice[ current_axis ] );
			break;
		case 's':
			current_slice[ current_axis ] = std::max( (current_slice[ current_axis ] - 1), 0 );
			fprintf( stderr, "decreasing current slice: %i\n", current_slice[ current_axis ] );
			break;
		case 'a': // optional
			current_axis = ( (current_axis + 1) % 3 );
			fprintf( stderr, "toggling viewing axis to: %i\n", current_axis );
			break;
		case '1':
			LoadData("../../Datasets/lobster.dat");
			break;
		case '2':
			LoadData("../../Datasets/skewed_head.dat");
			break;
		case 'b':
			NextClearColor();
			break;
		default:
			fprintf(stderr, "\nKeyboard commands:\n\n"
				"b - Toggle among background clear colors\n"
				"w - Increase current slice\n"
				"s - Decrease current slice\n"
				"a - Toggle viewing axis\n"
				"1 - Load lobster dataset\n"
				"2 - Load head dataset\n" );
			break;
	}
}

static void mouse(int button, int state, int x, int y){}

static void motion(int x, int y){}

static void reshape(int w, int h)
{
	float aspect = (float) w / (float) h;

	viewport_x = w;
	viewport_y = h;

	glViewport(0, 0, viewport_x, viewport_y);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glViewport(0, 0, viewport_x, viewport_y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -5.0);
}

static void special(int key, int x, int y)
{
	switch (key) {

		case GLUT_KEY_HOME:
		break;

		case GLUT_KEY_LEFT:
		break;

		case GLUT_KEY_RIGHT:
		break;

		case GLUT_KEY_UP:
		break;

		case GLUT_KEY_DOWN:
		break;
	}
}

// ==========================================================================
// Data
void LoadData(char* filename)
{
	fprintf(stderr, "loading data %s\n", filename );

	FILE* fp;
	fopen_s(&fp, filename, "rb" ); // open file, read only, binary mode 
	if ( fp == NULL ) {
		fprintf(stderr, "Cannot open file %s for reading.\n", filename );
		return;
	}
	
	memset( vol_dim, 0, sizeof( unsigned short ) * 3 );
	
	//read volume dimension
	fread( &vol_dim[ 0 ], sizeof( unsigned short ), 1, fp );
	fread( &vol_dim[ 1 ], sizeof( unsigned short ), 1, fp );
	fread( &vol_dim[ 2 ], sizeof( unsigned short ), 1, fp );
	
	fprintf(stderr, "volume dimensions: x: %i, y: %i, z:%i \n", vol_dim[ 0 ], vol_dim[ 1 ], vol_dim[ 2 ] );
	
	if ( data_array != NULL ) {
		delete[] data_array;
	}

	data_array = new unsigned short[ vol_dim[ 0 ] * vol_dim[ 1 ] * vol_dim[ 2 ] ]; //for intensity volume
	
	for( int z = 0; z < vol_dim[ 2 ]; z++ ) {
		for( int y = 0; y < vol_dim[ 1 ]; y++ ) { 	
			for( int x = 0; x < vol_dim[ 0 ]; x++ ) { 	

				fread( &data_array[  x + (y * vol_dim[ 0 ]) + (z * vol_dim[ 0 ] * vol_dim[ 1 ]) ], sizeof(unsigned short), 1, fp ); 
				data_array[  x + (y * vol_dim[ 0 ]) + (z * vol_dim[ 0 ] * vol_dim[ 1 ]) ] = data_array[  x + (y * vol_dim[ 0 ]) + (z * vol_dim[ 0 ] * vol_dim[ 1 ]) ] << 4;
			} 	
		} 	
	} 

	fclose( fp );
	
	current_slice[ 0 ] = vol_dim[ 0 ] / 2;
	current_slice[ 1 ] = vol_dim[ 1 ] / 2;
	current_slice[ 2 ] = vol_dim[ 2 ] / 2;

	DownloadVolumeAsTexture();

	data_loaded = true;
} 

void DownloadVolumeAsTexture()
{
	fprintf(stderr, "downloading volume to 3D texture\n" );

	// ================================================================================
	// TODO: Set up and download texture to GPU
	// ================================================================================
}

int main(int argc, char* argv[])
{
	data_array = NULL;
	viewport_x = 0;
	viewport_y = 0;
	current_slice[ 0 ] = 0;
	current_slice[ 1 ] = 0;
	current_slice[ 2 ] = 0;
	current_axis = 2;
	data_loaded = false;

	int gl_major, gl_minor;

	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );
	glutInitWindowSize( 512, 512 );
	window = glutCreateWindow( "AMCS/CS247 Scientific Visualization - Slice Viewer" );

	glutIdleFunc(play);
	glutDisplayFunc(display);
	glutKeyboardFunc(key);
	glutReshapeFunc(reshape);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutSpecialFunc(special);

	// Initialize the "OpenGL Extension Wrangler" library
	glewInit();

	// query and display OpenGL version
	getGlVersion(&gl_major, &gl_minor);
	printf("GL_VERSION major=%d minor=%d\n", gl_major, gl_minor);

	NextClearColor();

	// display help
	key('?', 0, 0);

	glutMainLoop();

	return 0;
}