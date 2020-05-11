//
// CS247_Assignment5
// AMCS/CS247 Programming Assignment 5
//

#include <cstdio>

#define _USE_MATH_DEFINES

#include <cmath>
#include <string>

#include "main.h"

#define EULER      0
#define RK2        1
#define RK4        2

int sampling_rate = 20;
bool draw_glyphs = false;
bool dynamic_glyphs = false;
bool draw_streamline = false;
bool draw_pathline = false;
int data_size;
std::vector<int> seed_point;
std::vector<int> streamline_point;
std::vector<int> pathline_point;
int integration = EULER;
float dt = 1;
float threshold = 0.001;
int max_iter = 80000;

int printOglError(char *file, int line) {
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

void getGlVersion(int *major, int *minor) {
    const char *verstr = (const char *) glGetString(GL_VERSION);
    if ((verstr == NULL) || (sscanf(verstr, "%d.%d", major, minor) != 2)) {
        *major = *minor = 0;
        fprintf(stderr, "Invalid GL_VERSION format!!!\n");
    }
}


/*
rotating background color
*/
static void NextClearColor(void) {
    static int color = 0;

    switch (color++) {
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
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw everything here as needed
    // you can use normalized coordinates as the viewport is scaled to 0..1 for x and y


    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, scalar_field_texture);


    glBegin(GL_QUADS);
    glColor3f(1.0, 1.0, 1.0);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 0);
    glVertex2f(1, 0);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(0, 1);
    glEnd();


    glDisable(GL_TEXTURE_2D);
    if (draw_glyphs) {
        drawGlyphs();
    }
    if (draw_streamline) {
        for (int i = streamline_point.size() / 2; i < seed_point.size(); i += 2) {
            computeStreamline(seed_point[i], seed_point[i + 1]);
        }
        glLineWidth(2);
        glBegin(GL_LINES);
        glColor3f(0.0, 1.0, 0.0);
        for (int i = 0; i < streamline_point.size(); i += 4) {
            glVertex2f(streamline_point[i + 0] / (float) vol_dim[0], streamline_point[i + 1] / (float) vol_dim[1]);
            glVertex2f(streamline_point[i + 2] / (float) vol_dim[0], streamline_point[i + 3] / (float) vol_dim[1]);
        }
        glEnd();
    }
    if (draw_pathline) {
        for (int i = pathline_point.size() / 2; i < seed_point.size(); i += 2) {
            computePathline(seed_point[i], seed_point[i + 1]);
        }
        glLineWidth(2);
        glBegin(GL_LINES);
        glColor3f(1.0, 0.0, 0.0);
        for (int i = 0; i < pathline_point.size(); i += 4) {
            glVertex2f(pathline_point[i + 0] / (float) vol_dim[0], pathline_point[i + 1] / (float) vol_dim[1]);
            glVertex2f(pathline_point[i + 2] / (float) vol_dim[0], pathline_point[i + 3] / (float) vol_dim[1]);
        }
        glEnd();
    }
    glFlush();
    glutSwapBuffers();

}


static
void play(void) {
    int thisTime = glutGet(GLUT_ELAPSED_TIME);

    glutPostRedisplay();
}


static
void key(unsigned char keyPressed, int x, int y) // key handling
{
    const char *status[2];
    status[0] = "disabled";
    status[1] = "enabled";

    switch (keyPressed) {
        case '1':
            toggle_xy = 0;
            LoadData(filenames[0]);
            loaded_file = 0;
            fprintf(stderr, "Loading %s dataset.\n", filenames[0]);
            break;
        case '2':
            toggle_xy = 0;
            LoadData(filenames[1]);
            loaded_file = 1;
            fprintf(stderr, "Loading %s dataset.\n", filenames[1]);
            break;
        case '3':
            toggle_xy = 1;
            LoadData(filenames[2]);
            loaded_file = 2;
            fprintf(stderr, "Loading %s dataset.\n", filenames[2]);
            break;
        case '4':
            draw_glyphs = !draw_glyphs;
            fprintf(stderr, "Draw glyphs %s.\n", status[draw_glyphs]);
            break;
        case '5':
            draw_streamline = !draw_streamline;
            fprintf(stderr, "%s streamline status\n", status[draw_streamline]);
            break;
        case '6':
            draw_pathline = !draw_pathline;
            fprintf(stderr, "%s pathline status\n", status[draw_pathline]);
            break;
        case 'j':
            integration = EULER;
            streamline_point.clear();
            pathline_point.clear();
            fprintf(stderr, "Switch to Euler\n");
            break;
        case 'k':
            integration = RK2;
            streamline_point.clear();
            pathline_point.clear();
            fprintf(stderr, "Switch to RK2\n");
            break;
        case 'l':
            integration = RK4;
            streamline_point.clear();
            pathline_point.clear();
            fprintf(stderr, "Switch to RK4\n");
            break;
        case 'i':
            dt += 0.005;
            fprintf(stderr, "Increase dt to %f.\n", dt);
            break;
        case 'o':
            dt -= 0.005;
            fprintf(stderr, "Decrease dt to %f.\n", dt);
            break;
        case 'h':
            for (int i = 0; i < vol_dim[0]; i += sampling_rate) {
                seed_point.push_back(i);
                seed_point.push_back(vol_dim[1] / 2);
            }
            fprintf(stderr, "Release multiple streamline seeds in horizontal rake\n");
            break;
        case 'v':
            for (int j = 0; j < vol_dim[1]; j += sampling_rate) {
                seed_point.push_back(vol_dim[0] / 2);
                seed_point.push_back(j);
            }
            fprintf(stderr, "Release multiple streamline seeds in vertical rake\n");
            break;
        case 'd':
            dynamic_glyphs = !dynamic_glyphs;
            fprintf(stderr, "Dynamic glyphs %s.\n", status[dynamic_glyphs]);
            break;
        case '0':
            if (num_timesteps > 1) {
                streamline_point.clear();
                pathline_point.clear();
                loadNextTimestep();
                fprintf(stderr, "Timestep %d.\n", loaded_timestep);
            }
            break;
        case 's':

            //current_scalar_field = (current_scalar_field + 1)%num_scalar_fields;
            //DownloadScalarFieldAsTexture();
            //fprintf( stderr, "Scalar field changed.\n");
            break;
        case 'b':
            NextClearColor();
            fprintf(stderr, "Next clear color.\n");
            break;
        case 'q':
        case 27:
            exit(0);
            break;
        case '+':
            sampling_rate++;
            fprintf(stderr, "Increase sample rate to %d.\n", sampling_rate);
            break;
        case '-':
            if (sampling_rate > 1)
                sampling_rate--;
            fprintf(stderr, "Decrease sample rate to %d.\n", sampling_rate);
            break;
        default:
            fprintf(stderr, "\nKeyboard commands:\n\n"
                            "1, load %s dataset\n"
                            "2, load %s dataset\n"
                            "3, load %s dataset\n"
                            "4, toggle draw glyphs\n"
                            "5, toggle streamline status\n"
                            "6, toggle pathline status\n"
                            "j, switch to Euler integration\n"
                            "k, switch to RK2 integration\n"
                            "l, switch to RK4 integration\n"
                            "h, Release multiple streamline seeds in horizontal rake\n"
                            "v, Release multiple streamline seeds in vertical rake\n"
                            "i, increase dt\n"
                            "o, decrease dt\n"
                            "0, cycle through timesteps\n"
                            "-, decrease sample rate\n"
                            "+, increase sample rate\n"
                            "b, switch background color\n"
                            "q, <esc> - Quit\n",
                    filenames[0], filenames[1], filenames[2]);
            break;
    }
}


static
void timer(int value) {
    glutTimerFunc(TIMER_FREQUENCY_MILLIS, timer, 0);
}


static
void mouse(int button, int state, int x, int y) {
    bmModifiers = glutGetModifiers();

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

        // seed streamlines & pathlines using mouseclicks
        seed_point.push_back(x);
        seed_point.push_back(y);
    }
}


static
void motion(int x, int y) {
}


static
void reshape(int wid, int ht) {
    view_width = wid;
    view_height = ht;

    // set viewport
    glViewport(0, 0, wid, ht);

    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    gluOrtho2D(0, 1, 0, 1);
}


static
void special(int key, int x, int y) {
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

float vector_value(size_t x, size_t y, int dim, int timestep) {
    x %= vol_dim[0];
    y %= vol_dim[1];
    int index = y * vol_dim[0] + x;
    return vector_array[3 * index + dim + 3 * timestep * data_size];
}

void drawGlyphs() {
    for (int i = 0; i < vol_dim[0]; i += sampling_rate) {
        for (int j = 0; j < vol_dim[1]; j += sampling_rate) {
            float x0 = i / (float) vol_dim[0];
            float y0 = j / (float) vol_dim[1];

            float vx = vector_value(i, j, 0, loaded_timestep);
            float vy = vector_value(i, j, 1, loaded_timestep);

            float s = sqrt(vx * vx + vy * vy);

            if (s < threshold) continue;

            glColor3f(1.0, 1.0, 0.0);
            glPushMatrix();
            glTranslatef(x0, y0, 0);
            glRotatef(atan2(vy, vx) * 180 / M_PI, 0, 0, 1);
            s = dynamic_glyphs ? 0.03 / (1 + exp(-0.03 * s)) : 0.03;
            glScalef(s, s, 1);
            glBegin(GL_TRIANGLES);
            glVertex2f(0, 0.75);
            glVertex2f(1, 0.5);
            glVertex2f(0, 0.25);
            glEnd();
            glPopMatrix();
        }
    }
}

float bilinearInterpolation(float x, float y, int dim, float timestep) {
    float xf = floor(x);
    float xc = ceil(x);
    float yf = floor(y);
    float yc = ceil(y);

    float v1 = vector_value(xf, yc, dim, timestep) * (xc - x) * (y - yf);
    float v2 = vector_value(xc, yc, dim, timestep) * (x - xf) * (y - yf);
    float v3 = vector_value(xf, yf, dim, timestep) * (xc - x) * (yc - y);
    float v4 = vector_value(xc, yf, dim, timestep) * (x - xf) * (yc - y);
    return v1 + v2 + v3 + v4;
}

void streamlineIntegration(int x, int y, bool backward = false) {
    float x0 = x;
    float y0 = y;
    if (x0 < 0 || vol_dim[0] <= x0 || y0 < 0 || vol_dim[1] <= y0) return;

    float vx = vector_value(x0, y0, 0, loaded_timestep);
    float vy = vector_value(x0, y0, 1, loaded_timestep);
    float s = sqrt(vx * vx + vy * vy);
    if (s < threshold) return;
    if (backward) {
        vx = -vx;
        vy = -vy;
    }

    // normalize
    vx /= s;
    vy /= s;

    for (int i = 0; i < max_iter; i++) {
        float x1, y1;
        float ax, ay, bx, by, cx, cy, dx, dy;
        switch (integration) {
            case RK2:
                // next postion
                x1 = x0 + vx * dt / 2;
                y1 = y0 + vy * dt / 2;
                if (x1 < 0 || vol_dim[0] <= x1 || y1 < 0 || vol_dim[1] <= y1) return;

                // bilinear interpolation
                vx = bilinearInterpolation(x1, y1, 0, loaded_timestep);
                vy = bilinearInterpolation(x1, y1, 1, loaded_timestep);
                s = sqrt(vx * vx + vy * vy);
                if (s < threshold) return;
                if (backward) {
                    vx = -vx;
                    vy = -vy;
                }

                // normalize
                vx /= s;
                vy /= s;

                // next postion
                x1 = x0 + vx * dt;
                y1 = y0 + vy * dt;
                break;
            case EULER:
                // next postion
                x1 = x0 + vx * dt;
                y1 = y0 + vy * dt;
                break;
            case RK4:
                ax = vx * dt;
                ay = vy * dt;
                bx = bilinearInterpolation(x0 + ax / 2, y0 + ay / 2, 0, loaded_timestep) * dt;
                by = bilinearInterpolation(x0 + ax / 2, y0 + ay / 2, 1, loaded_timestep) * dt;
                if (backward) {
                    bx = -bx;
                    by = -by;
                }
                cx = bilinearInterpolation(x0 + bx / 2, y0 + by / 2, 0, loaded_timestep) * dt;
                cy = bilinearInterpolation(x0 + bx / 2, y0 + by / 2, 1, loaded_timestep) * dt;
                if (backward) {
                    cx = -cx;
                    cy = -cy;
                }
                dx = bilinearInterpolation(x0 + cx, y0 + cy, 0, loaded_timestep) * dt;
                dy = bilinearInterpolation(x0 + cx, y0 + cy, 1, loaded_timestep) * dt;
                if (backward) {
                    dx = -dx;
                    dy = -dy;
                }
                x1 = x0 + (ax + 2 * bx + 2 * cx + dx) / 6;
                y1 = y0 + (ay + 2 * by + 2 * cy + dy) / 6;
                break;
        }

        // save line
        streamline_point.push_back(x0);
        streamline_point.push_back(y0);
        streamline_point.push_back(x1);
        streamline_point.push_back(y1);

        if (x1 < 0 || vol_dim[0] <= x1 || y1 < 0 || vol_dim[1] <= y1) return;

        // bilinear interpolation
        vx = bilinearInterpolation(x1, y1, 0, loaded_timestep);
        vy = bilinearInterpolation(x1, y1, 1, loaded_timestep);
        s = sqrt(vx * vx + vy * vy);
        if (s < threshold) return;
        if (backward) {
            vx = -vx;
            vy = -vy;
        }

        // normalize
        vx /= s;
        vy /= s;

        x0 = x1;
        y0 = y1;
    }
}

void computeStreamline(int x, int y) {
    // compute streamlines starting from x,y position
    // enable switching between euler and runge kutta
    streamlineIntegration(x, y);
    streamlineIntegration(x, y, true);
}

float trilinearInterpolation(float x, float y, int dim, float timestep) {
    int timestepf = floor(timestep);
    int timestepc = ceil(timestep);

    float vf = bilinearInterpolation(x, y, dim, timestepf);
    float vc = bilinearInterpolation(x, y, dim, timestepc);

    return vf * (timestepc - timestep) + vc * (timestep - timestepf);
}

void pathlineIntegration(int x, int y, bool backward = false) {
    float x0 = x;
    float y0 = y;
    if (x0 < 0 || vol_dim[0] <= x0 || y0 < 0 || vol_dim[1] <= y0) return;
    float timestep = loaded_timestep;

    float vx = vector_value(x0, y0, 0, loaded_timestep);
    float vy = vector_value(x0, y0, 1, loaded_timestep);
    float vz = vector_value(x0, y0, 2, loaded_timestep);
    float s = sqrt(vx * vx + vy * vy + vz * vz);
    if (s < threshold) return;
    if (backward) {
        vx = -vx;
        vy = -vy;
        vz = -vz;
    }

    // normalize
    vx /= s;
    vy /= s;
    vz /= s;

    for (int i = 0; i < max_iter; i++) {
        float x1, y1, timestep1;
        switch (integration) {
            case RK2:
                x1 = x0 + vx * dt / 2;
                y1 = y0 + vy * dt / 2;
                if (x1 < 0 || vol_dim[0] <= x1 || y1 < 0 || vol_dim[1] <= y1) return;
                timestep1 = timestep + vz * dt / 2;
                if (timestep1 < 0 || num_timesteps <= timestep1) return;

                vx = trilinearInterpolation(x1, y1, 0, timestep1);
                vy = trilinearInterpolation(x1, y1, 1, timestep1);
                vz = trilinearInterpolation(x1, y1, 2, timestep1);
                s = sqrt(vx * vx + vy * vy + vz * vz);
                if (s < threshold) return;
                if (backward) {
                    vx = -vx;
                    vy = -vy;
                    vz = -vz;
                }

                // normalize
                vx /= s;
                vy /= s;
                vz /= s;

                // next postion
                x1 = x0 + vx * dt;
                y1 = y0 + vy * dt;
                break;
            case EULER:
                // next postion
                x1 = x0 + vx * dt;
                y1 = y0 + vy * dt;
                break;
            case RK4:
                x1 = x0;
                y1 = y0;
                break;
        }

        // save line
        pathline_point.push_back(x0);
        pathline_point.push_back(y0);
        pathline_point.push_back(x1);
        pathline_point.push_back(y1);

        if (x1 < 0 || vol_dim[0] <= x1 || y1 < 0 || vol_dim[1] <= y1) return;
        timestep += vz * dt;
        if (timestep < 0 || num_timesteps <= timestep) return;

        vx = trilinearInterpolation(x1, y1, 0, timestep);
        vy = trilinearInterpolation(x1, y1, 1, timestep);
        vz = trilinearInterpolation(x1, y1, 2, timestep);
        s = sqrt(vx * vx + vy * vy + vz * vz);
        if (s < threshold) return;
        if (backward) {
            vx = -vx;
            vy = -vy;
            vz = -vz;
        }

        // normalize
        vx /= s;
        vy /= s;
        vz /= s;

        x0 = x1;
        y0 = y1;
    }
}

void computePathline(int x, int y) {
    // compute pathlines starting from x,y position and time step t
    // enable switching between euler and runge kutta
    pathlineIntegration(x, y);
    pathlineIntegration(x, y, true);
}


void loadNextTimestep(void) {
    loaded_timestep = (loaded_timestep + 1) % num_timesteps;
    DownloadScalarFieldAsTexture();
}


/*
 * load .gri dataset
 * This only reads the header information and calls the dat loader
 * For now we ignore the grid data and assume a rectangular grid
 */
void LoadData(const char *base_filename) {
    //reset
    reset_rendering_props();

    char filename[80];
    strcpy(filename, base_filename);
    strcat(filename, ".gri");

    fprintf(stderr, "loading grid file %s\n", filename);

    // open grid file, read only, binary mode
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open file %s for reading.\n", filename);
        return;
    }

    // read header
    fscanf(fp, "SN4DB %hu %hu %hu %d %d %f",
           &vol_dim[0], &vol_dim[1], &vol_dim[2],
           &num_scalar_fields, &num_timesteps, &timestep);

    fprintf(stderr, "dimensions: x: %d, y: %d, z: %d.\n", vol_dim[0], vol_dim[1], vol_dim[2]);
    fprintf(stderr, "additional info: # scalar fields: %d, # timesteps: %d, timestep: %f.\n", num_scalar_fields,
            num_timesteps, timestep);

    // read data
/*	if ( vector_array != NULL ) {
		delete[] vector_array;
	}

	int data_size = vol_dim[ 0 ] * vol_dim[ 1 ] * vol_dim[ 2 ];

	// 1D array for storing volume data
	grid_array = new vec[ data_size ];

	// read grid data
	fread( grid_array, sizeof( float ), ( data_size * 3 ), fp );

	// close file
	fclose( fp );
*/
    char dat_filename[80];
    strcpy(dat_filename, base_filename);

    if (num_timesteps <= 1) {

        strcat(dat_filename, ".dat");

    } else {

        strcat(dat_filename, ".00000.dat");

    }

    loaded_timestep = 0;
    LoadVectorData(base_filename);

    glutReshapeWindow(vol_dim[0], vol_dim[1]);

    grid_data_loaded = true;
}

/*
 * load .dat dataset
 * loads vector and scalar fields
 */
void LoadVectorData(const char *filename) {
    fprintf(stderr, "loading scalar file %s\n", filename);

    // open data file, read only, binary mode
    char ts_name[80];
    if (num_timesteps > 1) {
        sprintf(ts_name, "%s.%.5d.dat", filename, 0);
    } else
        sprintf(ts_name, "%s.dat", filename);

    FILE *fp = fopen(ts_name, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open file %s for reading.\n", filename);
        return;
    } else {
        fclose(fp);
    }

    data_size = vol_dim[0] * vol_dim[1] * vol_dim[2];

    if (vector_array != NULL) {
        delete[] vector_array;
        vector_array = NULL;
    }
    vector_array = new float[data_size * 3 * num_timesteps];

    // read data
    if (scalar_fields != NULL) {
        delete[] scalar_fields;
        scalar_fields = NULL;
        delete[] scalar_bounds;
        scalar_bounds = NULL;
    }
    scalar_fields = new float[data_size * num_scalar_fields * num_timesteps];
    scalar_bounds = new float[2 * num_scalar_fields * num_timesteps];

    int num_total_fields = num_scalar_fields + 3;
    float *tmp = new float[data_size * num_total_fields * num_timesteps];

    for (int k = 0; k < num_timesteps; k++) {
        char times_name[80];
        if (num_timesteps > 1) {
            sprintf(times_name, "%s.%.5d.dat", filename, k);
            fp = fopen(times_name, "rb");
        } else
            fp = fopen(ts_name, "rb");
        // read scalar data
        fread(&tmp[k * data_size * num_total_fields], sizeof(float), (data_size * num_total_fields), fp);

        // close file
        fclose(fp);

        // copy and scan for min and max values
        for (int i = 0; i < num_scalar_fields; i++) {

            float min_val = 99999.9f;
            float max_val = 0.0f;

            float avg = 0.0;

            int offset = i * data_size * num_timesteps;

            for (int j = 0; j < data_size; j++) {

                float val = tmp[j * num_total_fields + 3 + i + k * data_size * num_total_fields];

                scalar_fields[j + k * data_size + offset] = val;

                if (toggle_xy) {
                    // overwrite
                    if (i == 0) {
                        vector_array[3 * j + 0 + 3 * k * data_size] = tmp[j * num_total_fields + 1 + k * data_size *
                                                                                                     num_total_fields];//toggle x and y components in vector field
                        vector_array[3 * j + 1 + 3 * k * data_size] = tmp[j * num_total_fields + 0 +
                                                                          k * data_size * num_total_fields];
                        vector_array[3 * j + 2 + 3 * k * data_size] = tmp[j * num_total_fields + 2 +
                                                                          k * data_size * num_total_fields];
                    }
                } else {
                    // overwrite
                    if (i == 0) {
                        vector_array[3 * j + 0 + 3 * k * data_size] = tmp[j * num_total_fields + 0 +
                                                                          k * data_size * num_total_fields];
                        vector_array[3 * j + 1 + 3 * k * data_size] = tmp[j * num_total_fields + 1 +
                                                                          k * data_size * num_total_fields];
                        vector_array[3 * j + 2 + 3 * k * data_size] = tmp[j * num_total_fields + 2 +
                                                                          k * data_size * num_total_fields];
                    }
                }

                min_val = std::min(val, min_val);
                max_val = std::max(val, max_val);

                avg += scalar_fields[offset + j + k * data_size] / data_size;
            }
            scalar_bounds[2 * i + k * num_scalar_fields * 2] = min_val;
            scalar_bounds[2 * i + 1 + k * num_scalar_fields * 2] = max_val;
        }

        // normalize
        for (int i = 0; i < num_scalar_fields; i++) {

            int offset = i * data_size * num_timesteps;

            float lower_bound = scalar_bounds[2 * i + k * num_scalar_fields * 2];
            float upper_bound = scalar_bounds[2 * i + 1 + k * num_scalar_fields * 2];

            // scale between [0..1] where 1 is original zero
            // the boundary of the bigger abs border will be used to scale
            // meaning one boundary will likely not be hit i.e real scale
            // for -50..100 will be [0.25..1.0]
            if (lower_bound < 0.0 && upper_bound > 0.0) {

                float scale = 0.5f / std::max(-lower_bound, upper_bound);

                for (int j = 0; j < data_size; j++) {

                    scalar_fields[offset + j + k * data_size] =
                            0.5f + scalar_fields[offset + j + k * data_size] * scale;
                }
                scalar_bounds[2 * i + k * num_scalar_fields * 2] =
                        0.5f + scalar_bounds[2 * i + k * num_scalar_fields * 2] * scale;
                scalar_bounds[2 * i + 1 + k * num_scalar_fields * 2] =
                        0.5f + scalar_bounds[2 * i + 1 + k * num_scalar_fields * 2] * scale;


                // scale between [0..1]
            } else {

                float sign = upper_bound <= 0.0 ? -1.0f : 1.0f;

                float scale = 1.0f / (upper_bound - lower_bound) * sign;

                for (int j = 0; j < data_size; j++) {

                    scalar_fields[offset + j + k * data_size] =
                            (scalar_fields[offset + j + k * data_size] - lower_bound) * scale;
                }
                scalar_bounds[2 * i + k * num_scalar_fields * 2] =
                        (scalar_bounds[2 * i + k * num_scalar_fields * 2] + lower_bound) * scale; //should be 0.0
                scalar_bounds[2 * i + 1 + k * num_scalar_fields * 2] =
                        (scalar_bounds[2 * i + 1 + k * num_scalar_fields * 2] + lower_bound) * scale; //should be 1.0
            }
        }
    }
    delete[] tmp;
    DownloadScalarFieldAsTexture();

    scalar_data_loaded = true;
}


void DownloadScalarFieldAsTexture(void) {
    fprintf(stderr, "downloading scalar field to 2D texture\n");

    glEnable(GL_TEXTURE_2D);

    // generate and bind 2D texture
    glGenTextures(1, &scalar_field_texture);
    glBindTexture(GL_TEXTURE_2D, scalar_field_texture);

    // set necessary texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    int datasize = vol_dim[0] * vol_dim[1];
    //download texture in correct format
    glTexImage2D(GL_TEXTURE_2D, 0, GL_INTENSITY16, vol_dim[0], vol_dim[1], 0, GL_LUMINANCE, GL_FLOAT,
                 &scalar_fields[loaded_timestep * datasize]);


    glDisable(GL_TEXTURE_2D);
}

char *textFileRead(char *fn) {

    FILE *fp;
    char *content = NULL;

    int count = 0;

    if (fn != NULL) {
        fp = fopen(fn, "rt");

        if (fp != NULL) {

            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0) {
                content = (char *) malloc(sizeof(char) * (count + 1));
                count = fread(content, sizeof(char), count, fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    return content;
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
        //exit(1);
    }


    // set viewport
    glViewport(0, 0, 512, 512);

    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    gluOrtho2D(0, 1, 0, 1);

    glDisable(GL_DEPTH_TEST);
}

void reset_rendering_props(void) {
    seed_point.clear();
    streamline_point.clear();
    pathline_point.clear();
    num_scalar_fields = 0;
}

/******************************************************************************
 *
 * Main
 *
 ******************************************************************************/
int main(int argc, char **argv) {
    // init variables
    view_width = 0;
    view_height = 0;

    toggle_xy = 0;

    reset_rendering_props();

    vector_array = NULL;
    scalar_fields = NULL;
    scalar_bounds = NULL;
    grid_data_loaded = false;
    scalar_data_loaded = false;

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

    DownloadScalarFieldAsTexture();

    // display help
    key('?', 0, 0);

    glutMainLoop();

    return 0;
}