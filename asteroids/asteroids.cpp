//
// asteriods.cpp: classic arcade game
//
#include <iostream>
#include <windows.h>  // For MS Windows
#include <GL/glut.h>  // GLUT, includes glu.h and gl.h


// windows size
#define SIZEX 1000
#define SIZEY 1000

#define PI 3.14

#define ROTL 'a'
#define ROTR 's'


// define the ship
struct ship {
	float x, y;
	float topx, topy;
	float leftx, lefty;
	float bottomx, bottomy;
	float rightx, righty;
	float rottopx, rottopy;
	float rotleftx, rotlefty;
	float rotbottomx, rotbottomy;
	float rotrightx, rotrighty;
	int tip;
};


// variables
ship s;

// functions
void initvars() {
	s.x = SIZEX / 2;
	s.y = SIZEY / 2;
	s.topx = 0;
	s.topy = 30;
	s.rightx = 20;
	s.righty = -30;
	s.bottomx = 0;
	s.bottomy = -40;
	s.leftx = -20;
	s.lefty = -30;
	s.rottopx = s.topx;
	s.rottopy = s.topy;
	s.rotrightx = s.rightx;
	s.rotrighty = s.righty;
	s.rotbottomx = s.bottomx;
	s.rotbottomy = s.bottomy;
	s.rotleftx = s.leftx;
	s.rotlefty = s.lefty;
	s.tip = 0;
}


// Handler for window-repaint event. Call back when the window first appears and
// whenever the window needs to be re-painted.
void display() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer

	//int angle;
	//for (angle = 0; angle < 90; angle = angle + 10) {

	// include s.tip rotation
	// xprime = x * cos( angle ) - y * sin( angle )
	// yprime = x * sin( angle ) + y * cos( angle )
	
	glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
	glColor3f(1.0f, 1.0f, 1.0f);    // White

	glVertex2f(s.x+s.rottopx, s.y+s.rottopy);    // x, y
	glVertex2f(s.x+s.rotrightx, s.y+s.rotrighty);
	glVertex2f(s.x+s.rotbottomx, s.y+s.rotbottomy);
	glVertex2f(s.x+s.rotleftx, s.y+s.rotlefty);

	glEnd();
	
	glFlush();  // Render now
}


void processNormalKeys(unsigned char key, int a, int b) {

	boolean anglechange;
	float angle;

	// work happens here -
	std::cout << "in Keys\n";
	anglechange = FALSE;
	if (key == ROTL) {
		s.tip -= 2;
		anglechange = TRUE;
	}
	if (key == ROTR) {
		s.tip += 2;
		anglechange = TRUE;

	}

	if (s.tip < 0) {
		s.tip = 360;
		anglechange = TRUE;
	}
	if (s.tip > 360) {
		s.tip = 0;
		anglechange = TRUE;
	}

	if (anglechange) {
		angle = s.tip * PI / 180;
		// rotate the ship - https://www.tutorialspoint.com/computer_graphics/2d_transformation.htm
		s.rottopx = s.topx * cos(angle) - s.topy * sin(angle);
		s.rottopy = s.topx * sin(angle) + s.topy * cos(angle);
		s.rotleftx = s.leftx * cos(angle) - s.lefty * sin(angle);
		s.rotlefty = s.leftx * sin(angle) + s.lefty * cos(angle);
		s.rotbottomx = s.bottomx * cos(angle) - s.bottomy * sin(angle);
		s.rotbottomy = s.bottomx * sin(angle) + s.bottomy * cos(angle);
		s.rotrightx = s.rightx * cos(angle) - s.righty * sin(angle);
		s.rotrighty = s.rightx * sin(angle) + s.righty * cos(angle);
	}
	display();
}


/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {

	initvars();

	glutInit(&argc, argv);                           // Initialize GLUT

	glutCreateWindow("OpenGL Asteroids v1");            // Create a window with the given title
	glutInitWindowSize(SIZEX, SIZEY);                // Set the window's initial width & height
	glutInitWindowPosition(0, 0);                    // Position the window's initial top-left corner
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, SIZEX, 0, SIZEY);
	glutKeyboardFunc(processNormalKeys);             // register the callback for keypress

	glutDisplayFunc(display); // Register display callback handler for window re-paint
	glutMainLoop();           // Enter the infinitely event-processing loop
	return 0;
}

