#pragma region Includes

// includes, system
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#include <iostream>

#include "SPH.h"

#if defined(_WIN32) || defined(_WIN64)
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

// OpenGL Graphics includes
// #include <GL/glew.h>
#if defined (__APPLE__) || defined(MACOSX)
  #include <GLUT/glut.h>
  #ifndef glutCloseFunc
  #define glutCloseFunc glutWMCloseFunc
  #endif
#else
#include <GL/freeglut.h>
#endif

#pragma endregion

//extern "C" {
//	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//}

using namespace std;

// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
float translate_z = -3.0;

bool keypressed = false, simulate = false;

float cubeFaces[24][3] = 
{
	{0.0,0.0,0.0}, {1.0,0.0,0.0}, {1.0,1.0,0.0}, {0.0,1.0,0.0},
	{0.0,0.0,0.0}, {1.0,0.0,0.0}, {1.0,0,1.0}, {0.0,0,1.0},
	{0.0,0.0,0.0}, {0,1.0,0.0}, {0,1.0,1.0}, {0.0,0,1.0},
	{0.0,1.0,0.0}, {1.0,1.0,0.0}, {1.0,1.0,1.0}, {0.0,1.0,1.0},
	{1.0,0.0,0.0}, {1.0,1.0,0.0}, {1.0,1.0,1.0}, {1.0,0.0,1.0},
	{0.0,0.0,1.0}, {0,1.0,1.0}, {1.0,1.0,1.0}, {1.0,0,1.0}
};

SPH *sph;
int winX = 600;
int winY = 600;

void display_cube()
{
	glPushMatrix();
		glColor3f(1.0, 1.0, 1.0);
		glLineWidth(2.0);
		for(int i = 0; i< 6; i++)
		{
			glBegin(GL_LINE_LOOP);
			for(int j = 0; j < 4; j++)
				glVertex3f(cubeFaces[i*4+j][0], cubeFaces[i*4+j][1], cubeFaces[i*4+j][2]);
			glEnd();
		}
	glPopMatrix();
}

void display_points()
{
	glPushMatrix();
		Particle *p = sph->Get_Paticles();
		glColor3f(0.2f, 0.5f, 1.0f);
		glPointSize(2.0f);

		glBegin(GL_POINTS);
		for(int i=0; i<sph->Get_Particle_Number(); i++)
			glVertex3f(p[i].pos.x, p[i].pos.y, p[i].pos.z);
		glEnd();
	glPopMatrix();
}

#pragma region Display and scene control
void display (void)
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float x, y, z, posX, posY, posZ, vx, vy, vz, normMag;

// set view matrix
    glLoadIdentity();
    glTranslatef(0.0, 0.0, translate_z);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 1.0, 0.0);
	
	glPushMatrix();
		glLineWidth(10.0);
		glBegin(GL_LINES);
		glColor3f(0, 0, 1);
		glVertex3f(0, 0, 0);
		glVertex3f(1, 0, 0);

		glColor3f(1, 0, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 1, 0);

		glColor3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, 1);
		glEnd();
	glPopMatrix();

	display_cube();
	display_points();
	
	glutSwapBuffers();
}

void idle(void)
{
	sph->Animation();
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        mouse_buttons |= 1<<button;
    } else if (state == GLUT_UP) {
        mouse_buttons = 0;
    }

    mouse_old_x = x;
    mouse_old_y = y;
}

void motion(int x, int y)
{
    float dx, dy;
    dx = (float)(x - mouse_old_x);
    dy = (float)(y - mouse_old_y);

    if (mouse_buttons & 1) {
        rotate_x += dy * 0.2f;
        rotate_y += dx * 0.2f;
    } else if (mouse_buttons & 4) {
        translate_z += dy * 0.01f;
    }

    mouse_old_x = x;
    mouse_old_y = y;
}

void keys (unsigned char key, int x, int y)
{
	static int toonf = 0;
	switch (key) {
		case 27:
			delete sph;
            exit(0);
			break;
		case 'a':
			sph->add_viscosity(10.0f);
			break;
		case 's':
			sph->add_viscosity(-10.0f);
			break;
		case 32:
			simulate = !simulate;
			cout << "Streaming: " << simulate << endl;
			break;
	}
}

// Setting up the GL viewport and coordinate system 
void reshape (int w, int h)
{
	glViewport (0,0, w,h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	
	gluPerspective (45, w*1.0/h*1.0, 0.01, 400);
	//glTranslatef (0,0,-5);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

#pragma endregion

// glew (extension loading), OpenGL state and CUDA - OpenGL interoperability initialization
void initGL ()
{
	/*GLenum err = glewInit();
	if (GLEW_OK != err) 
		return;
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;*/
	const GLubyte* renderer;
	const GLubyte* version;
	//const GLubyte* glslVersion;

	renderer = glGetString(GL_RENDERER); /* get renderer string */
	version = glGetString(GL_VERSION); /* version as a string */
	//glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	//printf("GLSL version supported %s\n", glslVersion);

	glEnable(GL_DEPTH_TEST);
}

void init(void)
{
	sph = new SPH();
	sph->Init_Fluid();
}

#pragma endregion

int main( int argc, const char **argv ) {

	srand((unsigned int)time(NULL));
	glutInit(&argc, (char**)argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (winX, winY); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ("SPH 3D");
	glutReshapeFunc (reshape);
	glutKeyboardFunc (keys);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	initGL ();
	init();

	glutDisplayFunc(display); 
	glutIdleFunc (idle);
    glutMainLoop();

	return 0;   /* ANSI C requires main to return int. */
}
