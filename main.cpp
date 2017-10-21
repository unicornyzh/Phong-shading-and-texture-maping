
/**********************************************************************
 *
 *  Texture map loading example (Mac)
 *  Assignment 4
 *  Fall 2012, CS 4554
 *
 **********************************************************************/
#include "stdafx.h"
/// Windows libraries needed for images
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#define NOMINMAX
#include <GL/glut.h>

#include <stdio.h>
#include <sys/stat.h>

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <comutil.h>
#include <math.h>
#include <cmath>




using namespace std;
/* ascii codes for special keys */
#define ESCAPE 27

/**********************************************************************
 * Configuration
 **********************************************************************/

#define INITIAL_WIDTH 600
#define INITIAL_HEIGHT 600
#define INITIAL_X_POS 100
#define INITIAL_Y_POS 100

#define WINDOW_NAME     "Assignment 3"

// Change this to the directory where you have the .d2 files on your
// computer, or put the files in your programs default start up directory.
#define RESOURCE_DIR   "D:\\computer Graphic\\Yan_LAB3\\resources\\"

/**********************************************************************
 * Globals
 **********************************************************************/

GLsizei window_width;
GLsizei window_height;

GLuint m_wall_texture_id;


/**********************************************************************
 * Set the new size of the window
 **********************************************************************/
void resize_scene(GLsizei width, GLsizei height)
{
    glViewport(0, 0, width, height);  /* reset the current viewport and 
                                       * perspective transformation */
    window_width  = width;
    window_height = height;
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    /* For the purposes of this assignment, we want the world coordinate
     * system to match the dimensions of the models to be displayed..*/
    gluOrtho2D(0, window_width, 0, window_height);
    glMatrixMode(GL_MODELVIEW);
    
}

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
    unsigned char* bits = (unsigned char*)malloc(w*h*3*2);  

    unsigned char* addr = bits;
    for (int y=0; y<h; ++y) {
        for (int x=0; x<w; ++x) {
            addr = bits + y*w*3 + x*3;

			Gdiplus::Color color;
            img.GetPixel(x,y, &color);
            unsigned char r,g,b;
            r = color.GetR();
            g = color.GetG(); 
            b = color.GetB();           

            *addr = r; ++addr;
            *addr = g; ++addr;
            *addr = b;
        }
    }

	if (w == 0 || h == 0) {
		fprintf(stderr,"Error loding texture: %s\n",file_name);
		return 0; 
	}

	// Create a texture from the image:
    glGenTextures (1, &tex_id);
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


/**********************************************************************
 * any program initialization (set initial OpenGL state, 
 **********************************************************************/
void init()
{
    // Load any models you will need during initialization
    // For the first project I have provided square.d2
    // and car.d2.  
    string ifile =string("brick_color_map.png");
    
    m_wall_texture_id = getTextureFromFile(ifile.c_str());
    
    // Set any OpenGL options that will not change during the program
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
}

/**********************************************************************
 * The main drawing functions. 
 **********************************************************************/
void draw_scene(void)
{
    /* clear the screen and the depth buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    /* reset modelview matrix */
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
	glColor3f(1.0, 1.0, 1.0f);
    
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_wall_texture_id);
    
    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(window_width, 0.0f, 0.0f);
    
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(window_width, window_height, 0.0f);
    
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(0.0f, window_height, 0.0f);
    glEnd();

    
    
    /* since this is double buffered, swap the
     * buffers to display what just got drawn */
    glutSwapBuffers();
}

/**********************************************************************
 * this function is called whenever a key is pressed
 **********************************************************************/
void key_press(unsigned char key, int x, int y) 
{
    switch (key) {
        case ESCAPE: /* exit the program...normal termination. */
            exit(0);
    }
}

/**********************************************************************
 * this function is called whenever the mouse is moved
 **********************************************************************/

void handle_mouse_motion(int x, int y)
{
    // If you do something here that causes a change to what the user
    // would see, call glutPostRedisplay to force a redraw
    //glutPostRedisplay();
}

/**********************************************************************
 * this function is called whenever a mouse button is pressed or released
 **********************************************************************/

void handle_mouse_click(int btn, int state, int x, int y)
{
    switch (btn) {
        case GLUT_LEFT_BUTTON:
            break;
    }
}

/**********************************************************************
 * this function is called for non-standard keys like up/down/left/right
 * arrows.
 **********************************************************************/
void special_key(int key, int x, int y)
{
    switch (key) {
            
        case GLUT_KEY_UP: //up arrow
            //glutPostRedisplay();
            break;
        case GLUT_KEY_DOWN: //down arrow
            //glutPostRedisplay();
            break;
        default:      
            break;
    }
}


int main(int argc, char * argv[]) 
{  
    
  	/* Initialize GLUT */
    glutInit(&argc, argv);  
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);  
    glutInitWindowSize(INITIAL_WIDTH, INITIAL_HEIGHT);  
    glutInitWindowPosition(INITIAL_X_POS, INITIAL_Y_POS);  
    glutCreateWindow(WINDOW_NAME); 

	// Initialize GDI+. (for image loading)
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    /* Register callback functions */
	glutDisplayFunc(draw_scene);     
    glutReshapeFunc(resize_scene);       //Initialize the viewport when the window size changes.
    glutKeyboardFunc(key_press);         //handle when the key is pressed
    glutMouseFunc(handle_mouse_click);   //check the Mouse Button(Left, Right and Center) status(Up or Down)
    glutMotionFunc(handle_mouse_motion); //Check the Current mouse position when mouse moves
	glutSpecialFunc(special_key);        //Special Keyboard Key fuction(For Arrow button and F1 to F10 button)
    
    /* OpenGL and other program initialization */
    init();
    
    /* Enter event processing loop */
    glutMainLoop();  
    
    return 1;
}



