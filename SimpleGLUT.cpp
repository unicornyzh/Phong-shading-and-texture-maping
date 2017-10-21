#include "stdafx.h"

/// Windows libraries needed for images
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#define NOMINMAX
// standard
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

// glut
#include <GL/glut.h>

// source
#include <math/vec3.h>
#include <model.h>
using namespace std;
//================================
// global variables
//================================
// screen size
int g_screenWidth  = 0;
int g_screenHeight = 0;

// frame index
int g_frameIndex = 0;
GLuint m_wall_texture_id;
GLuint m_floor_texture_id;
GLuint m_ceiling_texture_id;
// model
Model g_model;
Model g_model2;
Model g_model3;

// surface material attributes
GLfloat material_Ka[] = { 0.11f, 0.06f, 0.11f, 1.0f };
GLfloat material_Kd[] = { 0.43f, 0.47f, 0.54f, 1.0f };
GLfloat material_Ks[] = { 0.55f, 0.33f, 0.52f, 1.0f };
GLfloat material_Ke[] = { 0.10f, 0.00f, 0.10f, 1.0f };
GLfloat material_Se = 10;

//Light source move parameters
GLfloat step = 0.03;

//keyboard control for camera
GLfloat cx=0, cy=0, cz=0;      //position of camera
GLfloat lx=0, ly=0, lz=0;      //position of center where the camera look at
GLfloat angle = 0;             //angle of rotation
GLfloat cstep = 0.05;           //the distance for camera movement 

//Light
/** attributes of light sources */
//global ambient
GLfloat LightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };    /**< Ambient */
//light1 spot light
GLfloat Spot_Direction[] = { 0.0f, -1.0f, 0.0f };
GLfloat LightAmbient2[] = { 1.5f, 1.5f, 1.5f, 1.0f };    /**< Ambient */
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };    /**<Diffuse */
GLfloat LightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };   /**<Specular */
GLfloat LightPosition[] = { 0.0f, 2.5f, 5.0f, 1.0f };   /**< Light source 1 position, and it's a spot light */
//light2 point light
GLfloat LightPosition1[] = { 0.0f, 1.5f, 1.0f, 1.0f };  /**< Light source  2 position, and it's a point light */
GLfloat LightAmbient1[] = { 0.7f, 0.8f, 0.9f, 1.0f };  /**< Light source  2 ambient*/

GLint flag = 0;
/**********************************************************************
* Open an image and return a texture ID or 0 on failure. There is also
* some gdi startup/shutdown code in main that is required.
**********************************************************************/
GLuint getTextureFromFile(const char* file_name)
{
	GLuint   tex_id;

	// Convert the image filename to unicode for the Bitmap constructor function:
	int lenA = lstrlenA(file_name);
	int lenW;
	BSTR unicodestr;

	lenW = ::MultiByteToWideChar(CP_ACP, 0, file_name, lenA, 0, 0);
	if (lenW > 0)
	{
		// Check whether conversion was successful
		unicodestr = ::SysAllocStringLen(0, lenW);
		::MultiByteToWideChar(CP_ACP, 0, file_name, lenA, unicodestr, lenW);
	}
	else
	{
		//unicode error..
		return 0;
	}

	// Read in the image:
	Gdiplus::Bitmap img(unicodestr);

	// when done, free the BSTR
	::SysFreeString(unicodestr);

	/// Get all pixels from the image:
	int w = img.GetWidth();
	int h = img.GetHeight();
	unsigned char* bits = (unsigned char*)malloc(w*h * 3 * 2);

	unsigned char* addr = bits;
	for (int y = 0; y<h; ++y) {
		for (int x = 0; x<w; ++x) {
			addr = bits + y*w * 3 + x * 3;

			Gdiplus::Color color;
			img.GetPixel(x, y, &color);
			unsigned char r, g, b;
			r = color.GetR();
			g = color.GetG();
			b = color.GetB();

			*addr = r; ++addr;
			*addr = g; ++addr;
			*addr = b;
		}
	}

	if (w == 0 || h == 0) {
		fprintf(stderr, "Error loding texture: %s\n", file_name);
		return 0;
	}

	// Create a texture from the image:
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGB,
		w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, bits);
	return tex_id;

}
//================================
// init, initial setting of lighting and model texture
//================================
void init( void ) {
	// init something before main loop...
	cx = 0, cy = 0, cz = 0;
	lx = 0, ly = 0, lz = 0;
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//light1
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, Spot_Direction);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0f);
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient2);     /**< set ambinet light */
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);     /**< set duffuse light */
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);   /**< set specular light */
	//light2
	glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbient1);      /**< set ambinet light */
	glLightfv(GL_LIGHT2, GL_DIFFUSE, LightDiffuse);      /**< set duffuse light */
	glLightfv(GL_LIGHT2, GL_SPECULAR, LightSpecular);   /**< set specular light */
	// load model_1
	g_model.LoadModel("data/cow.d");
	// load model_2
	g_model2.LoadModel("data/cow.d");
	// load model_3
	g_model3.LoadModel("data/cow.d");
	//load textures for wall,floor and ceiling
	string ifile =string("brick_color_map.png");
	m_wall_texture_id = getTextureFromFile(ifile.c_str());
	ifile= string("cement_brick_wall_texture.png");
	m_floor_texture_id= getTextureFromFile(ifile.c_str());
	m_ceiling_texture_id= getTextureFromFile(ifile.c_str());
}

//================================
// update
//================================
void update( int k) {
	
}

//================================
// render, bulid room and objects, lighting
//================================


void drawGrids( float height ) {
	float step = 0.1f;

	int n = 20;

	float r = step * n;

	glBegin( GL_LINES );
	for ( int i = -n; i <= n; i++ ) {
		glVertex3f( i * step, height, -r );
		glVertex3f( i * step, height, +r );
	}

	for ( int i = -n; i <= n; i++ ) {
		glVertex3f( -r, height, i * step );
		glVertex3f( +r, height, i * step );
	}

	glEnd();
}

void render( void ) {
	// clear color and depth buffer
	glClearColor (0.0, 0.0, 0.0, 0.0);
	glClearDepth ( 1.0 );
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	// enable depth test
	glEnable( GL_DEPTH_TEST );
	// modelview matrix <------------------------------------------
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glPushMatrix();
	gluLookAt(
		cx + 8, cy + 3, cz + 8, // eye
		lx, ly, lz + 5, // center
		0, 1, 0  // up
	);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
	glLightfv(GL_LIGHT2, GL_POSITION, LightPosition1);
	glDisable(GL_LIGHTING);
	//draw light source by a sphere;
	glPushMatrix();
	glDisable(GL_LIGHT1);
	glColor3f(1.0, 1.0, 1.0);
	glTranslatef(LightPosition[0], LightPosition[1], LightPosition[2]);
	glutSolidSphere(0.3, 4, 4);
	glPopMatrix();
	glPushMatrix();
    glDisable(GL_LIGHT2);
	if(flag==1) glColor3f(1.0, 1.0, 1.0);
	if(flag==0)  glColor3f(0, 0, 0);
	glTranslatef(LightPosition1[0], LightPosition1[1], LightPosition1[2]);
	glutSolidSphere(0.3, 4, 4);
	glPopMatrix();
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightAmbient);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	if (flag == 1)
		glEnable(GL_LIGHT2);
	if (flag == 0)
		glDisable(GL_LIGHT2);
	// draw model
	glPushMatrix();
	glLineWidth(1);
	glTranslatef(0.0f, 0.0f, 3.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);
	g_model.DrawFlat();
	glPopMatrix();


	//draw model2
	glPushMatrix();
	glLineWidth(1);
	glTranslatef(0.0f, 0.0f, 6.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);
	g_model2.DrawFlat();
	glPopMatrix();

	//draw model3
	glPushMatrix();
	glLineWidth(1);
	glTranslatef(0.0f, 0.0f, 8.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);
	g_model3.DrawFlat();
	glPopMatrix();

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);
	//draw wall with texture coordinates and backface removal
	glBindTexture(GL_TEXTURE_2D, m_wall_texture_id);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	
	glTexCoord2f(0.0, 0.0); glVertex3f(10.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(10.0, -1.0, 10.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(10.0, 5.0, 10.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(10.0, 5.0, 0);

	glTexCoord2f(0.0, 0.0); glVertex3f(-10.0, -1.0, 10.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-10.0, 5.0, 10.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(10.0, 5.0, 10.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(10.0, -1.0, 10.0);
	glEnd();  
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-10.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-10.0, -1.0, 10.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-10.0, 5.0, 10.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-10.0, 5.0, 0);

	glTexCoord2f(0.0, 0.0); glVertex3f(-10.0, -1.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-10.0, 5.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(10.0, 5.0, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(10.0, -1.0, 0.0);
	glEnd();
	//draw floor with texture coordinates and backface removal
	glDisable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D, m_floor_texture_id);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-10.0, -1.0, 0.0);   
	glTexCoord2f(0.0, 1.0); glVertex3f(-10.0, -1.0, 10.0); 
	glTexCoord2f(1.0, 1.0); glVertex3f(10.0, -1.0, 10.0); 
	glTexCoord2f(1.0, 0.0); glVertex3f(10.0, -1.0, 0);     
	glEnd();

	//draw ceiling with texture coordinates and backface removal
	glDisable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glBindTexture(GL_TEXTURE_2D, m_ceiling_texture_id);
	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material_Ka);
   //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material_Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_Ks);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material_Ke);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material_Se);
	glTexCoord2f(0.0, 0.0); glVertex3f(-10.0, 5.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-10.0, 5.0, 10.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(10.0, 5.0, 10.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(10.0, 5.0, 0.0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
	// swap back and front buffers
	glutSwapBuffers();
}

//================================
// keyboard input  ,camera move and rotate
//================================
void key_press( unsigned char key, int x, int y ) {
	int k;
	switch (key) {
	case 'w':
			cx -= cstep;                                        //camera move ,forwarding into screen
		break;
	case 'a':
			cz += cstep;                                         
		break;
	case 's':                                     //move back
			cx += cstep;
		break;
	case 'd':                                        //move righ
			cz -= cstep;
	    break;
	case 'r':                                        //move up
			cy += cstep;
		break;
	case 'f':                                      //move down
			cy -= cstep;
	    break;
	case 'q':                                                   //rotate camera ,anti-clockwise
		angle -= 0.1f;
		lx = sin(angle);
		lz = -cos(angle);
		break;
	case 'e':                                                  //rotate camera ,clockwise
		angle += 0.1f;
		lx = sin(angle);
		lz = -cos(angle);
		break;
	default:
		break;
    }
}

//================================
// keyboard input  , light sources control
//================================
void special_key( int key, int x, int y ) {
	switch (key) {
	case GLUT_KEY_UP:                  // Light source 1 move up
		LightPosition[1] += step;
		break;
	case GLUT_KEY_DOWN:                 // Light source 1 move down
		LightPosition[1] -= step;
		break;
	case GLUT_KEY_RIGHT:                // Light source 1 move right
		LightPosition[2] -= step;
		break;
	case GLUT_KEY_LEFT:                // Light source 1 move left
		LightPosition[2] += step;
		break;
	case GLUT_KEY_F1:                  //open Light source 2
		flag = 1;
		break;
	case GLUT_KEY_F2:                   //close Light source 2
		flag = 0;
		break;
	default:      
		break;
	}
}

//================================
// reshape : update viewport and projection matrix when the window is resized
//================================
void reshape( int w, int h ) {
	// screen size
	g_screenWidth  = w;
	g_screenHeight = h;	
	
	// viewport
	glViewport( 0, 0, (GLsizei)w, (GLsizei)h );

	// projection matrix <------------------------------------------
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0f, (float)w / (float)h, 0.1f, 100.0f );

}

//================================
// timer : triggered every 16ms ( about 60 frames per second )
//================================
void timer( int value ) {
	// increase frame index
	g_frameIndex++;

	update(0);
	
	// render
	glutPostRedisplay();

	// reset timer
	glutTimerFunc( 16, timer, 0 );
}

//================================
// main
//================================
int main( int argc, char** argv ) {
	// create opengL window
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB |GLUT_DEPTH );
	glutInitWindowSize( 600, 600 ); 
	glutInitWindowPosition( 100, 100 );
	glutCreateWindow( argv[0] );
	// Initialize GDI+. (for image loading)
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	// init
	init();

	// set callback functions
	glutDisplayFunc( render );
	glutReshapeFunc( reshape );
	glutKeyboardFunc( key_press ); 
	glutSpecialFunc( special_key );
	glutTimerFunc( 16, timer, 0 );
	
	// main loop
	glutMainLoop();

	return 0;
}