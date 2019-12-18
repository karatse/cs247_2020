#include <glew.h>
#include <glut.h>

static GLint window;

static void display(void)
{
	//draw the mesh here
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(512, 512);
	window = glutCreateWindow("OpenGL Primer");
	glewInit();

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}