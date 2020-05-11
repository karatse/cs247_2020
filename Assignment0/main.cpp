#include <glew.h>
#include <glut.h>
#include <cstdio>

static GLint window;

// Texture
unsigned char data[] = {
        0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0,
        0, 255, 0, 255, 0, 255, 0, 255,
        255, 0, 255, 0, 255, 0, 255, 0,
};

GLfloat coords[][3] = {
        {-1.0f, -1.0f, 1.0f},
        {1.0f,  -1.0f, 1.0f},
        {1.0f,  1.0f,  1.0f},
        {-1.0f, 1.0f,  1.0f},
        {-1.0f, -1.0f, -1.0f},
        {1.0f,  -1.0f, -1.0f},
        {1.0f,  1.0f,  -1.0f},
        {-1.0f, 1.0f,  -1.0f},
};

// Animation
GLfloat x_translation = 0;
GLfloat y_translation = 0;
GLfloat z_translation = -4.5;
GLfloat angle = 0;
bool perspective = true;
GLfloat factor = 1;

static void display(void) {
    //draw the mesh here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();   // Reset the view
    glTranslatef(x_translation, y_translation, z_translation);
    glRotatef(angle, 1.0f, 1.0f, 1.0f);
    glScalef(factor, factor, factor);

    glBegin(GL_QUADS);
    glColor3ub(255, 255, 255);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3fv(coords[0]);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3fv(coords[1]);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3fv(coords[2]);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3fv(coords[3]);
    glColor3ub(191, 40, 44);
    glVertex3fv(coords[4]);
    glVertex3fv(coords[5]);
    glVertex3fv(coords[6]);
    glVertex3fv(coords[7]);
    glColor3ub(255, 221, 198);
    glVertex3fv(coords[2]);
    glVertex3fv(coords[3]);
    glVertex3fv(coords[7]);
    glVertex3fv(coords[6]);
    glColor3ub(223, 223, 223);
    glVertex3fv(coords[0]);
    glVertex3fv(coords[1]);
    glVertex3fv(coords[5]);
    glVertex3fv(coords[4]);
    glColor3ub(225, 106, 77);
    glVertex3fv(coords[1]);
    glVertex3fv(coords[2]);
    glVertex3fv(coords[6]);
    glVertex3fv(coords[5]);
    glColor3ub(252, 171, 129);
    glVertex3fv(coords[0]);
    glVertex3fv(coords[3]);
    glVertex3fv(coords[7]);
    glVertex3fv(coords[4]);
    glEnd();
    glFlush();
}

void x_translate(GLfloat value) {
    x_translation += value;
    printf("translation in X direction by: %g\n", x_translation);
}

void y_translate(GLfloat value) {
    y_translation += value;
    printf("translation in Z direction by: %g\n", y_translation);
}

void z_translate(GLfloat value) {
    z_translation += value;
    printf("translation in Z direction by: %g\n", z_translation);
}

void rotate(int value) {
    angle += value;
    if (angle >= 360) {
        angle -= 360;
    } else if (angle < 0) {
        angle += 360;
    }
    printf("rotation angle is: %f\n", angle);
}

GLdouble aspect = 1;

void setup_projection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (perspective) {
        gluPerspective(60, aspect, 0.1, 30.0);
    } else {
        glOrtho(-2, 2, -2 / aspect, 2 / aspect, 0.1, 30.0);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void toggle_projection() {
    perspective = !perspective;
    setup_projection();
    if (perspective) {
        printf("using perspective projection\n");
    } else {
        printf("using orthographic projection\n");
    }
}

void scale(GLfloat value) {
    factor *= value;
    printf("scale factor is: %f\n", factor);
}

void reshape(GLsizei w, GLsizei h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    aspect = (GLdouble) w / (GLdouble) h;
    setup_projection();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:
            exit(0);
        case 'r':
            rotate(-1);
            break;
        case 't':
            rotate(2);
            break;
        case 'z':
            scale(0.9f);
            break;
        case 'x':
            scale(1.1f);
            break;
        case 'w':
            y_translate(0.1f);
            break;
        case 's':
            y_translate(-0.1f);
            break;
        case 'd':
            x_translate(0.1f);
            break;
        case 'a':
            x_translate(-0.1f);
            break;
        case 'q':
            z_translate(0.1f);
            break;
        case 'e':
            z_translate(-0.1f);
            break;
        case ' ':
            toggle_projection();
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(512, 512);
    window = glutCreateWindow("OpenGL Primer");
    glewInit();

    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    setup_projection();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 8, 8, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glEnable(GL_TEXTURE_2D);

    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}