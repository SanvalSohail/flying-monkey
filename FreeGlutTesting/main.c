/*******************************************************************
           Multi-Part Model Construction and Manipulation
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>
#include <malloc.h>
#include "Vector3D.h"
#include "QuadMesh.h"
#include "Cubemesh.h"

#define M_PI 3.14159265358979323846

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;
typedef unsigned short ushort;
typedef unsigned long  ulong;

typedef struct RGB
{
	byte r, g, b;
} RGB;

typedef struct RGBpixmap
{
	int nRows, nCols;
	RGB *pixel;
} RGBpixmap;

RGBpixmap pix[7];

const int meshSize = 16;    // Default Mesh Size
const int vWidth = 650;     // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels
static int currentButton;
static unsigned char currentKey;
const int droneScale = 0.5;
int txWidth;
int txHeight;
int torX;
int torZ;
bool droneCam = false;
bool fireMissile = false;
GLuint texBufferID;
GLuint texCoordID, texID;
GLfloat* uvs;
GLuint texture;
// Lighting/shading and material properties for drone - upcoming lecture - just copy for now

// Light properties
static GLfloat light_position0[] = { -6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_position1[] = { 6.0F, 12.0F, 0.0F, 1.0F };
static GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
static GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Material properties
static GLfloat drone_mat_ambient[] = { 0.4F, 0.2F, 0.0F, 1.0F };
static GLfloat drone_mat_specular[] = { 0.1F, 0.1F, 0.0F, 1.0F };
static GLfloat drone_mat_diffuse[] = { 0.9F, 0.5F, 0.0F, 1.0F };
static GLfloat drone_mat_shininess[] = { 0.0F };
//A quad mesh repersenting the streets
static QuadMesh streetMesh[10];
// A quad mesh representing the ground
static QuadMesh groundMesh;
//building cube mesh
static CubeMesh buildings[10];

// Structure defining a bounding box, currently unused
//struct BoundingBox {
//    Vector3D min;
//    Vector3D max;
//} BBox;

// Prototypes for functions in this module
void drawHead();
void drawStreets();
void drawPropeller2Quad();
void drawPropeller3Quad();
void drawBodyQuad();
void drawPropeller1Quad();
void drawBuildings();
void initOpenGL(int w, int h);
void moveCamera();
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void readBMPFile(RGBpixmap *pm, char *file);
void setTexture(RGBpixmap *p, GLuint textureID);
void launchMissile();
void moveMissile();
ulong getLong(FILE *fp);
ushort getShort(FILE *fp);
void fskip(FILE *fp, int num_bytes);
void makeCheckerboard(RGBpixmap *p);

Vector3D ScreenToWorld(int x, int y);
 GLfloat spin = 0.0;
 GLfloat move = 0.0;
 GLfloat height = 0.0;
 GLfloat turn = 0.0;
 GLfloat zMove = 0.0;
 GLfloat xMove = 0.0;
 GLfloat angle = 0.0f;
 GLfloat lx = 6.3f, lz = 8.3f, ly = 0.0f;
 GLfloat camX = 0.0f, camZ = 0.3f, camH = 1.1f;

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Assignment 1");
	// Initialize GL
	initOpenGL(vWidth, vHeight);
	// Register callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);
	// Start event loop, never returns
	glutMainLoop();
	return 0;
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw Drone
	//drawTest();
	// Set drone material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, drone_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, drone_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, drone_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, drone_mat_shininess);
	// Apply transformations to move drone
	// ...
	moveCamera();
	drawBodyQuad();
	drawHead();
	drawPropeller1Quad();
	drawPropeller2Quad();
	drawPropeller3Quad();
	if (fireMissile == true)
	{
		launchMissile();
		moveMissile();
	}
	//Draw City Here
	drawBuildings();
	drawStreets();
	// Draw ground mesh
	DrawMeshQM(&groundMesh, meshSize);
	glutSwapBuffers();   // Double buffering, swap buffers
}

// Set up OpenGL. For viewport and projection setup see reshape(). */
void initOpenGL(int w, int h)
{
	//setsup a texture to load
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	readBMPFile(&pix[0], "professor.bmp");
	setTexture(&pix[0], 2000);          // assign a unique identifier
	readBMPFile(&pix[1], "road.bmp");  // read texture 
	setTexture(&pix[1], 2001);
	readBMPFile(&pix[2], "clover01.bmp");  // read texture 
	setTexture(&pix[2], 2002);           // assign a unique identifier
	readBMPFile(&pix[3], "road2.bmp");  // read texture 
	setTexture(&pix[3], 2003);
	readBMPFile(&pix[4], "plank01.bmp");         // make texture 
	setTexture(&pix[4], 2004);           // assign a unique identifier
	readBMPFile(&pix[5], "residential.bmp"); // read texture 
	setTexture(&pix[5], 2005);
	readBMPFile(&pix[6], "glass.bmp"); // read texture 
	setTexture(&pix[6], 2006);
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);   // This light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.6F, 0.6F, 0.6F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective


	// Set up ground quad mesh
	Vector3D origin = NewVector3D(-8.0f, 0.0f, 8.0f);
	Vector3D dir1v = NewVector3D(1.0f, 0.0f, 0.0f);
	Vector3D dir2v = NewVector3D(0.0f, 0.0f, -1.0f);
	groundMesh = NewQuadMesh(meshSize);
	InitMeshQM(&groundMesh, meshSize, origin, 20.0, 20.0, dir1v, dir2v);

	Vector3D ambient = NewVector3D(0.0f, 0.05f, 0.0f);
	Vector3D diffuse = NewVector3D(0.4f, 0.8f, 0.4f);
	Vector3D specular = NewVector3D(0.04f, 0.04f, 0.04f);
	SetMaterialQM(&groundMesh, ambient, diffuse, specular, 0.2);

	for (int i = 0;i < 10;i++)
	{
		buildings[i] = newCube();
	}
	Vector3D streetOrigin = NewVector3D(-8.0f, 0.1f, 8.0f);
	Vector3D dir1S = NewVector3D(0.0f, 0.0f, -1.0f);
	Vector3D dir2S = NewVector3D(1.0f, 0.0f, 0.0f);
	for (int i = 0;i < 10;i++)
	{
		streetMesh[i] = NewQuadMesh(10);
		InitMeshQM(&streetMesh[i],10,streetOrigin,20.0,20.0,dir1S,dir2S);
	}


	// Set up the bounding box of the scene
	// Currently unused. You could set up bounding boxes for your objects eventually.
	//Set(&BBox.min, -8.0f, 0.0, -8.0);
	//Set(&BBox.max, 8.0f, 6.0,  8.0);
}
void drawBodyQuad()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glPushMatrix();
	glScalef(0.4f, 0.4f, 0.4f);
	glTranslatef(xMove, height, zMove);
	glRotatef(turn, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 4.0, 1.0);
	glRotatef(45.0, 0.0, 1.0, 0.0);
	glScalef(1.0, 1.0, 2.0);
	glTranslatef(0.0, -4.0, -1.0);
	glTranslatef(0.0, 4.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, 2000); 
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void drawPropeller1Quad()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	
	//---------------------------------------------Antenna------------------------------------------------------/
	glPushMatrix();
	glScalef(0.4f, 0.4f, 0.4f);
	glTranslatef(xMove, height, zMove);
	glRotatef(turn, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 6.0, 0.0);
	glScalef(0.125, 1.0, 0.125);
	glBindTexture(GL_TEXTURE_2D, 2002); // top face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();
	glFlush();
	glPopMatrix();
	//---------------------------------------------Antenna------------------------------------------------------/
	//---------------------------------------------Propeller------------------------------------------------------/
	glPushMatrix();
	glScalef(0.4f, 0.4f, 0.4f);
	glTranslatef(xMove, height, zMove);
	glRotatef(spin, 0, 1.0, 0);
	glRotatef(turn, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 7.0, 0.0);
	glScalef(-0.125, -0.125, 0.5);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//---------------------------------------------Propeller------------------------------------------------------/
}

void drawPropeller2Quad()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	//---------------------------------------------Antenna------------------------------------------------------/
	glPushMatrix();
	glScalef(0.4f, 0.4f, 0.4f);
	glTranslatef(xMove, height, zMove);
	glRotatef(turn, 0.0, 1.0, 0.0);
	glTranslatef(1.0, 6.0, 2.5);
	glScalef(0.125, 1.0, 0.125);
	glBindTexture(GL_TEXTURE_2D, 2002); // top face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();
	glFlush();
	glPopMatrix();
	//---------------------------------------------Antenna------------------------------------------------------/
	//---------------------------------------------Propeller------------------------------------------------------/
	glPushMatrix();
	glScalef(0.4f, 0.4f, 0.4f);
	glTranslatef(xMove, height, zMove);
	glRotatef(turn, 0.0, 1.0, 0.0);
	glTranslatef(1.0, 7.0, 2.5);
	glRotatef(spin, 0, 1.0, 0);
	glScalef(-0.125, -0.125, 0.5);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//---------------------------------------------Propeller------------------------------------------------------/
}

void drawPropeller3Quad()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	//---------------------------------------------Antenna------------------------------------------------------/
	glPushMatrix();
	glScalef(0.4f, 0.4f, 0.4f);
	glTranslatef(xMove, height, zMove);
	glRotatef(turn, 0.0, 1.0, 0.0);
	glTranslatef(-1.5, 6.0, 0.0);
	glScalef(0.125, 1.0, 0.125);
	glBindTexture(GL_TEXTURE_2D, 2002); // top face of cube
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();
	glFlush();
	glPopMatrix();
	//---------------------------------------------Antenna------------------------------------------------------/
	//---------------------------------------------Propeller------------------------------------------------------/
	glPushMatrix();
	glScalef(0.4f, 0.4f, 0.4f);
	glTranslatef(xMove, height, zMove);
	glRotatef(turn, 0.0, 1.0, 0.0);
	glTranslatef(-1.5, 7.0, 0.0);
	glRotatef(spin, 0, 1.0, 0);
	glScalef(-0.125, -0.125, 0.5);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-0.5f, 0.5f, -0.5f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//---------------------------------------------Propeller------------------------------------------------------/
}

void drawBuildings()
{
	//-------------------------building 1-------------------------
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glPushMatrix();
	glTranslatef(5.0, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, 2006);
	drawCube(&buildings[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 2-------------------------
	glPushMatrix();
	glTranslatef(0.0, 0.0, -4.5);
	glBindTexture(GL_TEXTURE_2D, 2005);
	drawCube(&buildings[1]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 3-------------------------
	glPushMatrix();
	glTranslatef(-3.0, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, 2006);
	drawCube(&buildings[2]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 4-------------------------
	glPushMatrix();
	glTranslatef(-5.0, 0.0, -3.0);
	glBindTexture(GL_TEXTURE_2D, 2005);
	drawCube(&buildings[3]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 5-------------------------
	glPushMatrix();
	glTranslatef(3.0, 0.0, -3.0);
	glBindTexture(GL_TEXTURE_2D, 2006);
	drawCube(&buildings[4]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 6-------------------------
	glPushMatrix();
	glTranslatef(7.0, 0.0, 5.0);
	glBindTexture(GL_TEXTURE_2D, 2005);
	drawCube(&buildings[5]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 7-------------------------
	glPushMatrix();
	glTranslatef(-6.5, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, 2006);
	drawCube(&buildings[6]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 8-------------------------
	glPushMatrix();
	glTranslatef(-6.0, 0.0, 6.0);
	glBindTexture(GL_TEXTURE_2D, 2005);
	drawCube(&buildings[7]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 9-------------------------
	glPushMatrix();
	glTranslatef(9.5, 0.0, 2.5);
	glBindTexture(GL_TEXTURE_2D, 2006);
	drawCube(&buildings[8]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	//-------------------------building 10-------------------------
	glPushMatrix();
	glTranslatef(8.0, 0.0, -4.0);
	glBindTexture(GL_TEXTURE_2D, 2005);
	drawCube(&buildings[9]);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void drawStreets()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, 2001);

	glPushMatrix();
	glTranslatef(-1.5, 0.0, 0.0);
	glScalef(0.05f, 1.0f, 1.0f);
	DrawMeshQM(&streetMesh[0], 10);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(1.5, 0.0, 0.0);
	glScalef(0.05f, 1.0f, 1.0f);
	DrawMeshQM(&streetMesh[1], 10);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 2003);
	glPushMatrix();
	glTranslatef(4.0, 0.0, 2.0);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.05f, 1.0f, 1.0f);
	DrawMeshQM(&streetMesh[2], 10);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(4.0, 0.0,-8.0);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glScalef(0.05f, 1.0f, 1.0f);
	DrawMeshQM(&streetMesh[3], 10);
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void drawHead()
{
	glPushMatrix();
	glScalef(0.4f, 0.4f, 0.4f);
	glTranslatef(xMove, height, zMove);
	glRotatef(turn, 0.0, 1.0, 0.0);
	glTranslatef(1.7,4.0,2.7);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glBindTexture(GL_TEXTURE_2D, 2004);
	GLUquadricObj *sphere = gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_FILL);
	glPolygonMode(GL_FRONT, GL_FILL);
	gluQuadricNormals(sphere, GLU_SMOOTH);
	glutSolidSphere(1.0, 50.0,50.0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
}

void drawAiDrone()
{

}

void launchMissile()
{
	glPushMatrix();
	glTranslatef(xMove + torX, height + 2, zMove + torZ);
	glutSolidSphere(1,50,50);
	glPopMatrix();
}

void moveMissile()
{
	torX += 0.05 *(float)cos((turn * M_PI / 180) + 45);
	torZ += 0.05 *(float)sin((turn * M_PI / 180) + 45);
	glutPostRedisplay();
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
    // Set up viewport, projection, then change to modelview matrix mode - 
    // display function will then set up camera and do modeling transforms.
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
   // gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void moveCamera()
{
	glLoadIdentity();
	if (droneCam == true)
	{
		gluLookAt(camX, camH, camZ, camX + lx, camH + ly, camZ + lz, 0.0, 1.0, 0.0);
	}
	else
	{
		gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	}
}

void spinDisplay(void)
{
	spin += 2.0;
	if (spin > 360.0)
		spin -= 360.0;
	glutPostRedisplay();
}
void forwardDisplay(void)
{
	zMove += 0.05 *(float)cos((turn * M_PI / 180) + 45);
	xMove += 0.05 *(float)sin((turn * M_PI / 180) + 45);
	spin += 2.0;
	if (spin > 360.0)
		spin -= 360.0;
	glutPostRedisplay();
}

void backwardsDisplay(void)
{
	zMove -= 0.05 *(float)cos((turn * M_PI / 180) + 45);
	xMove -= 0.05 *(float)sin((turn * M_PI / 180) + 45);
	spin += 2.0;
	if (spin > 360.0)
		spin -= 360.0;
	glutPostRedisplay();
}

void upDisplay(void)
{
	height += 0.02;
	spin += 2.0;
	if (spin > 360.0)
		spin -= 360.0;
	glutPostRedisplay();
}

void downDisplay(void)
{
	height += -0.02;
	spin += 2.0;
	if (spin > 360.0)
		spin -= 360.0;
	glutPostRedisplay();
}

void turnRight(void)
{
	turn -= 2.0;
	spin -= 2.0;
	if (spin > -360.0)
		spin += 360.0;
	if (turn < -360.0)
		turn += 360.0;
	glutPostRedisplay();
}

void turnLeft(void)
{
	turn += 2.0;
	spin += 2.0;
	if (spin > 360.0)
		spin -= 360.0;
	if (turn < 360.0)
		turn -= 360.0;
	glutPostRedisplay();
}
// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 's':
		glutIdleFunc(spinDisplay);
        break;
	case 'f':
		camZ += 0.05 *(float)cos((turn * M_PI / 180) + 45);
		zMove += 0.125 *(float)cos((turn * M_PI / 180) + 45);
		camX += 0.05 *(float)sin((turn * M_PI / 180) + 45);
		xMove += 0.125 *(float)sin((turn * M_PI / 180) + 45);
		spin += 2.0;
		if (spin > 360.0)
			spin -= 360.0;
		break;
	case 'b':
		camZ -= 0.05 *(float)cos((turn * M_PI / 180) + 45);
		zMove -= 0.125 *(float)cos((turn * M_PI / 180) + 45);
		camX -= 0.05 *(float)sin((turn * M_PI / 180) + 45);
		xMove -= 0.125 *(float)sin((turn * M_PI / 180) + 45);
		spin += 2.0;
		if (spin > 360.0)
			spin -= 360.0;
		break;
	case 'h':
		printf("s = spin propeller \n");
		printf("f = move forward \n");
		printf("b = move backwards \n");
		printf("r = switch to drone camera \n");
		printf("t = switch to stationary camera \n");
		printf("o = tilt up \n");
		printf("p = tilt down \n");
		printf("m = to fire a missile \n");
		printf("up key = move the drone higher \n");
		printf("down key = move the drone lower \n");
		printf("left key = veer left with drone \n");
		printf("right key = veer right with drone \n");
		break;
	case 'r':
		droneCam = true;
		break;
	case 't':
		droneCam = false;
		break;
	case 'o':
		if (ly > camH) ly = camH;
		ly += 0.2;
		break;
	case 'p':
		if (ly < -25) ly = -25;
		ly -= 0.2;
		break;
	case 'm':
		fireMissile = true;
		break;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
    // Help key
	switch (key)
	{
	case GLUT_KEY_LEFT:
		angle += 0.0357;
		lx = sin(angle - 0.0357);
		lz = cos(angle - 0.0357);
		turn += 2.0;
		spin += 2.0;
		if (spin > 360.0)
			spin -= 360.0;
		if (turn < 360.0)
			turn -= 360.0;
		break;
	case GLUT_KEY_UP:
		height += 0.05;
		camH += 0.02;
		spin += 2.0;
		if (spin > 360.0)
			spin -= 360.0;
		break;
	case GLUT_KEY_DOWN:
		height += -0.05;
		camH += -0.02;
		spin += 2.0;
		if (spin > 360.0)
			spin -= 360.0;
		break;
	case GLUT_KEY_RIGHT:
		angle -= 0.0357;
		lx = sin(angle);
		lz = cos(angle);
		turn -= 2.0;
		spin -= 2.0;
		if (spin > -360.0)
			spin += 360.0;
		if (turn < -360.0)
			turn += 360.0;
		break;
	}
 
    // Do transformations with arrow keys
    //else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
    //{
    //}
    glutPostRedisplay();   // Trigger a window redisplay
}

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
    currentButton = button;

    switch (button)
    {
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
			glutIdleFunc(NULL);
		break;
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN)
			glutIdleFunc(NULL);
		break;
	default:
		break;
    }
    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
    if (currentButton == GLUT_LEFT_BUTTON)
    {
        ;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


Vector3D ScreenToWorld(int x, int y)
{
    // you will need to finish this if you use the mouse
    return NewVector3D(0, 0, 0);
}

void makeCheckerboard(RGBpixmap *p)
{
	long count = 0;
	int i, j, c;

	p->nRows = p->nCols = 64;
	p->pixel = (RGB *)malloc(3 * p->nRows * p->nCols);

	for (i = 0; i < p->nRows; i++)
		for (j = 0; j < p->nCols; j++)
		{
			c = (((i / 8) + (j / 8)) % 2) * 255;
			p->pixel[count].r = c;
			p->pixel[count].g = c;
			p->pixel[count++].b = 0;
		}
}



/**************************************************************************
 *  fskip                                                                 *
 *     Skips bytes in a file.                                             *
 **************************************************************************/

void fskip(FILE *fp, int num_bytes)
{
	int i;
	for (i = 0; i < num_bytes; i++)
		fgetc(fp);
}


/**************************************************************************
 *                                                                        *
 *    Loads a bitmap file into memory.                                    *
 **************************************************************************/

ushort getShort(FILE *fp) //helper function
{ //BMP format uses little-endian integer types
  // get a 2-byte integer stored in little-endian form
	char ic;
	ushort ip;
	ic = fgetc(fp); ip = ic;  //first byte is little one 
	ic = fgetc(fp);  ip |= ((ushort)ic << 8); // or in high order byte
	return ip;
}
//<<<<<<<<<<<<<<<<<<<< getLong >>>>>>>>>>>>>>>>>>>
ulong getLong(FILE *fp) //helper function
{  //BMP format uses little-endian integer types
   // get a 4-byte integer stored in little-endian form
	ulong ip = 0;
	char ic = 0;
	unsigned char uc = ic;
	ic = fgetc(fp); uc = ic; ip = uc;
	ic = fgetc(fp); uc = ic; ip |= ((ulong)uc << 8);
	ic = fgetc(fp); uc = ic; ip |= ((ulong)uc << 16);
	ic = fgetc(fp); uc = ic; ip |= ((ulong)uc << 24);
	return ip;
}


void readBMPFile(RGBpixmap *pm, char *file)
{
	FILE *fp;
	long index;
	int k, row, col, numPadBytes, nBytesInRow;
	char ch1, ch2;
	ulong fileSize;
	ushort reserved1;    // always 0
	ushort reserved2;     // always 0 
	ulong offBits; // offset to image - unreliable
	ulong headerSize;     // always 40
	ulong numCols; // number of columns in image
	ulong numRows; // number of rows in image
	ushort planes;      // always 1 
	ushort bitsPerPixel;    //8 or 24; allow 24 here
	ulong compression;      // must be 0 for uncompressed 
	ulong imageSize;       // total bytes in image 
	ulong xPels;    // always 0 
	ulong yPels;    // always 0 
	ulong numLUTentries;    // 256 for 8 bit, otherwise 0 
	ulong impColors;       // always 0 
	long count;
	char dum;

	/* open the file */
	if ((fp = fopen(file, "rb")) == NULL)
	{
		printf("Error opening file %s.\n", file);
		exit(1);
	}

	/* check to see if it is a valid bitmap file */
	if (fgetc(fp) != 'B' || fgetc(fp) != 'M')
	{
		fclose(fp);
		printf("%s is not a bitmap file.\n", file);
		exit(1);
	}

	fileSize = getLong(fp);
	reserved1 = getShort(fp);    // always 0
	reserved2 = getShort(fp);     // always 0 
	offBits = getLong(fp); // offset to image - unreliable
	headerSize = getLong(fp);     // always 40
	numCols = getLong(fp); // number of columns in image
	numRows = getLong(fp); // number of rows in image
	planes = getShort(fp);      // always 1 
	bitsPerPixel = getShort(fp);    //8 or 24; allow 24 here
	compression = getLong(fp);      // must be 0 for uncompressed 
	imageSize = getLong(fp);       // total bytes in image 
	xPels = getLong(fp);    // always 0 
	yPels = getLong(fp);    // always 0 
	numLUTentries = getLong(fp);    // 256 for 8 bit, otherwise 0 
	impColors = getLong(fp);       // always 0 

	if (bitsPerPixel != 24)
	{ // error - must be a 24 bit uncompressed image
		printf("Error bitsperpixel not 24\n");
		exit(1);
	}
	//add bytes at end of each row so total # is a multiple of 4
	// round up 3*numCols to next mult. of 4
	nBytesInRow = ((3 * numCols + 3) / 4) * 4;
	numPadBytes = nBytesInRow - 3 * numCols; // need this many
	pm->nRows = numRows; // set class's data members
	pm->nCols = numCols;
	pm->pixel = (RGB *)malloc(3 * numRows * numCols);//make space for array
	if (!pm->pixel) return; // out of memory!
	count = 0;
	dum;
	for (row = 0; row < numRows; row++) // read pixel values
	{
		for (col = 0; col < numCols; col++)
		{
			int r, g, b;
			b = fgetc(fp); g = fgetc(fp); r = fgetc(fp); //read bytes
			pm->pixel[count].r = r; //place them in colors
			pm->pixel[count].g = g;
			pm->pixel[count++].b = b;
		}
		fskip(fp, numPadBytes);
	}
	fclose(fp);
}


void setTexture(RGBpixmap *p, GLuint textureID)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, p->nCols, p->nRows, 0, GL_RGB,
		GL_UNSIGNED_BYTE, p->pixel);
}

