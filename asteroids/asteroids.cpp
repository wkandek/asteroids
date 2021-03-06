//
// asteriods.cpp: classic arcade game
//
// Structure:
// function main gets called first and mainly initializes GLUT the graphics library
// GLUT works the following way:
// 1. initializes the window
// 2. add a "display" function. This function draws objects in the window. Here: rocks, the ship and torpedos. 
//    If a new object gets introduced, for example the alien saucer that shows up every once in a while
//    this function would have to be changed
// 3. add an "idle" function. This function animates the objects. It caluluates a new position for the rocks by adding the speed to the old position
//    It checks whether rocks are colliding with each other and determines how to bounce them of each other. It checks for collision of rocks with the ship
//    It checks whether torpedos are active and need to be animated. 
//    It also animates the ship with its current speed and if it is under thrust adds a little flame
//    Further it checks whether any object is "out of bounds" and wraps it around or erases it.
// 4. add a "keyboard" function. This function reacts to keypresses. For example 'a' and 's' rotate the ship, so a new position get calculated here.
//

#include <iostream>   // to print debug messages
#include <windows.h>  // For MS Windows
#include <GL/glut.h>  // GLUT, includes glu.h and gl.h
#include <chrono>     // to wait milliseconds, used in display
#include <thread>
#include <ctime>      // time based functions, here used to seed rand()


// windows size
#define SIZEX 1000
#define SIZEY 1000

// for angle radian converion - C sin() and cos() want radians, we think here in angles
#define PI 3.14

// maximum nr of rocks, nodes in a rock and torpedos
#define MAXNODES 12
#define MAXROCKS 200
#define MAXTORPEDOS 50

// types of rock
#define LARGEROCK 3
#define MEDIUMROCK 2
#define SMALLROCK 1

// keys that cause actions - rotatte left, right, move forward, shoot torpedo, make frame rate faster, slower and reset to orginal state
#define ROTL 'a'
#define ROTR 's'
#define FWD 'w'
#define FASTROTL 'A'
#define FASTROTR 'S'
#define FASTFWD 'W'
#define SHOT ' '
#define FASTER '+'
#define SLOWER '-'
#define RESET 'r'


#define SHIPSPEED 2
#define SHIPSPEEDTURBO 6


// define the ship - all coordinates to define ship, rot* gets drawn, as they represent the rotated coordinates
// tip which is an angle and tells us where the ship's tip is pointing to 
// and radius which is used to detected a collsion with the ship
// and xpseed and yspeed that have the current speed
struct ship {
	float x, y;
	int radius;
	int tip;                                    // where is the tip pointing to? 0 = up, 90 is left
	int xspeed, yspeed;                         // current speed of ship in x and y terms that get added to teh position of the ship each cycle in idle function
	float topx, topy;
	float leftx, lefty;
	float bottomx, bottomy;
	float rightx, righty;
	float flamerightx, flamerighty;
	float flameleftx, flamelefty;
	float flamebottomx, flamebottomy;
	float rottopx, rottopy;
	float rotleftx, rotlefty;
	float rotbottomx, rotbottomy;
	float rotrightx, rotrighty;
	float rotflamerightx, rotflamerighty;
	float rotflameleftx, rotflamelefty;
	float rotflamebottomx, rotflamebottomy;
};

// define the torpedo coordinates and speed, radius to for collsion detection and an 'active' attribute
// active is need because there can be more than one torpedo and while they start out active when fired, they can indivdually become inactive
// for example: players fired 3 torpedos, the second one hits a rock, it will then become inactive while 1 and 3 need to continue to be animated.
struct torpedo {
	int x, y;
	int radius;
	int xspeed, yspeed;
	boolean active;
};

// similar concepts to the torpedos, expect conatins a ploygon that is used to make it look like a rock
struct rock {
	int x;
	int y;
	int xspeed;
	int yspeed;
	int radius;
	int size;
	boolean active;
	int nodes;
	int nodex[MAXNODES];
	int nodey[MAXNODES];
};


// variables
ship s;
torpedo t[MAXTORPEDOS];
rock rocks[MAXROCKS];
int nrofrocks;
int nroftorpedos;
int frames;
int score;
boolean shipcollision;
boolean flameon = FALSE;


// functions

// defines the nodes in rock scales down the values by size factor
void setrocksize(int i, int size) {
	int sizefactor[4] = { 0,4,2,1 };

	rocks[i].nodes = 9;
	rocks[i].nodex[0] = 0 / sizefactor[size];
	rocks[i].nodey[0] = 20 / sizefactor[size];
	rocks[i].nodex[1] = 20 / sizefactor[size];
	rocks[i].nodey[1] = 20 / sizefactor[size];
	rocks[i].nodex[2] = 30 / sizefactor[size];
	rocks[i].nodey[2] = 0 / sizefactor[size];
	rocks[i].nodex[3] = 30 / sizefactor[size];
	rocks[i].nodey[3] = -10 / sizefactor[size];
	rocks[i].nodex[4] = 10 / sizefactor[size];
	rocks[i].nodey[4] = -20 / sizefactor[size];
	rocks[i].nodex[5] = -10 / sizefactor[size];
	rocks[i].nodey[5] = -10 / sizefactor[size];
	rocks[i].nodex[6] = -15 / sizefactor[size];
	rocks[i].nodey[6] = -5 / sizefactor[size];
	rocks[i].nodex[7] = -15 / sizefactor[size];
	rocks[i].nodey[7] = -0 / sizefactor[size];
	rocks[i].nodex[8] = -10 / sizefactor[size];
	rocks[i].nodey[8] = 10 / sizefactor[size];
}


// set inital variables
void initvars() {
	srand(time(NULL));
	s.x = SIZEX / 2;
	s.y = SIZEY / 2;
	s.topx = 0;
	s.topy = 30;
	s.radius = 50;
	s.rightx = 20;
	s.righty = -30;
	s.bottomx = 0;
	s.bottomy = -20;
	s.leftx = -20;
	s.lefty = -30;
	s.flamerightx = 4;
	s.flamerighty = -4;
	s.flameleftx = -4;
	s.flameleftx = -4;
	s.flamebottomx = 0;
	s.flamebottomy = -10;
	s.rottopx = s.topx;
	s.rottopy = s.topy;
	s.rotrightx = s.rightx;
	s.rotrighty = s.righty;
	s.rotbottomx = s.bottomx;
	s.rotbottomy = s.bottomy;
	s.rotleftx = s.leftx;
	s.rotlefty = s.lefty;
	s.rotflamerightx = s.flamerightx;
	s.rotflamerighty = s.flamerighty;
	s.rotflameleftx = s.flameleftx;
	s.rotflamelefty = s.flamelefty;
	s.rotflamebottomx = s.flamebottomx;
	s.rotflamebottomy = s.flamebottomy;
	s.tip = 0;
	s.xspeed = 0;
	s.yspeed = 0;

	nrofrocks = 4;
	rocks[0].x = 100;
	rocks[0].y = 800;
	rocks[0].xspeed = 10;
	rocks[0].yspeed = -10;
	rocks[0].radius = 30;
	rocks[0].size = LARGEROCK;
	rocks[0].active = TRUE;
	setrocksize(0, rocks[0].size);

	rocks[1].x = 100;
	rocks[1].y = 100;
	rocks[1].xspeed = 10;
	rocks[1].yspeed = 10;
	rocks[1].radius = 30;
	rocks[1].size = LARGEROCK;
	rocks[1].active = TRUE;
	setrocksize(1, rocks[1].size);

	rocks[2].x = 500;
	rocks[2].y = 300;
	rocks[2].xspeed = 10;
	rocks[2].yspeed = 10;
	rocks[2].radius = 30;
	rocks[2].size = LARGEROCK;
	rocks[2].active = TRUE;
	setrocksize(2, rocks[2].size);

	rocks[3].x = 800;
	rocks[3].y = 800;
	rocks[3].xspeed = 10;
	rocks[3].yspeed = 10;
	rocks[3].radius = 15;
	rocks[3].size = LARGEROCK;
	rocks[3].active = TRUE;
	setrocksize(3, rocks[2].size);

	nroftorpedos = 0;
	frames = 80;
	shipcollision = FALSE;
	score = 0;
}


// "deletes" active torpeods from the data structure, necessary because there will be many thousands of shots
// in a game and they will overflow the allocated space, so we need to clean up spent torpedos (out of bounds or hits, i.e. active - FALSE)
void compacttorpedos() {
	int i, j;

	for (i = 0; i < nroftorpedos; i++) {
		if (!t[i].active) {
			for (j = i; j < nroftorpedos - 1; j++) {
				t[i] = t[i + 1];
			}
			nroftorpedos--;
		}
	}
	// std::cout << nroftorpedos << " ";
}



void printScore(int score) {
	char vstr[80];
	int i;

	sprintf_s(vstr, "%07d", score);
	glRasterPos2f(500, 950); // location to start printing text
	for (i = 0; i < strlen(vstr); i++) // loop until i is greater then l
	{
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, vstr[i]); // Print a character on the screen
	}
}


void display() {
	int i, j;
	float fi;
	int circlex, circley;


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer

	// draw the rocks i for each rock, j for each node
	glColor3f(1.0f, 1.0f, 1.0f);    // White
	for (i = 0; i < nrofrocks; i++) {
		if (rocks[i].active) {
			glBegin(GL_LINE_LOOP);              // Each set of 4 vertices form a quad
			for (j = 0; j < rocks[i].nodes; j++) {
				glVertex2f(rocks[i].x + rocks[i].nodex[j], rocks[i].y + rocks[i].nodey[j]);
			}
			glEnd();
		}
	}

	// draw the ship
	glBegin(GL_LINE_LOOP);
	if (nroftorpedos < MAXTORPEDOS / 2) {
		glColor3f(1.0f, 1.0f, 1.0f);    // White
	}
	else {
		// indicate overheating
		glColor3f(1.0f, 0.0f, 0.0f);    // Red
	}
	glVertex2f(s.x + s.rottopx, s.y + s.rottopy);
	glVertex2f(s.x + s.rotrightx, s.y + s.rotrighty);
	glVertex2f(s.x + s.rotbottomx, s.y + s.rotbottomy);
	glVertex2f(s.x + s.rotleftx, s.y + s.rotlefty);
	glEnd();
	if (flameon) {
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0f, 1.0f, 1.0f);    // White
		glVertex2f(s.x + s.rotbottomx, s.y + s.rotbottomy);
		glVertex2f(s.x + s.rotbottomx + s.rotflamerightx, s.y + s.rotbottomy + s.rotflamerighty);
		glVertex2f(s.x + s.rotbottomx + s.rotflamebottomx, s.y + s.rotbottomy + s.rotflamebottomy);
		glVertex2f(s.x + s.rotbottomx + s.rotflameleftx, s.y + s.rotbottomy + s.rotflamelefty);
		glEnd();
		flameon = FALSE;
	}

	// draw the torpedo, if active
	// torpedos go inactive if they hit something, or if they are offscreen
	for (i = 0; i < nroftorpedos; i++) {
		if (t[i].active) {
			glBegin(GL_QUADS);
			glColor3f(1.0f, 1.0f, 1.0f);    // White
			glVertex2f(t[i].x, t[i].y);
			glVertex2f(t[i].x + t[i].radius, t[i].y);
			glVertex2f(t[i].x + t[i].radius, t[i].y + t[i].radius);
			glVertex2f(t[i].x, t[i].y + t[i].radius);
			glEnd();
		}
	}

	if (shipcollision) {
		// draw a circle around
		glBegin(GL_LINE_LOOP);
		glColor3f(1.0f, 1.0f, 1.0f);
		for (fi = 0; fi < 2 * PI; fi += PI / 12) {
			circlex = s.x + cos(fi) * s.radius;
			circley = s.y + sin(fi) * s.radius;
			glVertex2i(circlex, circley);
		}
		glEnd();
		shipcollision = FALSE;
	}

	printScore(score);
	glFlush();  // Render now
}


void processNormalKeys(unsigned char key, int a, int b) {
	boolean anglechange;
	float angle;


	std::cout << "in Keys\n";
	// rotate left or rotate right
	anglechange = FALSE;
	if (key == FWD) {
		// add thrust to current direction, first get the angle, then calc new speed add on
		angle = s.tip * PI / 180;
		s.xspeed += -SHIPSPEED * sin(angle);
		s.yspeed += SHIPSPEED * cos(angle);
		flameon = TRUE;
	}
	if (key == FASTFWD) {
		angle = s.tip * PI / 180;
		s.x += -SHIPSPEEDTURBO * sin(angle);
		s.y += SHIPSPEEDTURBO * cos(angle);
		flameon = TRUE;
	}
	if (key == ROTL) {
		s.tip += 4;
		anglechange = TRUE;
	}
	if (key == ROTR) {
		s.tip -= 4;
		anglechange = TRUE;
	}
	if (key == FASTROTL) {
		s.tip += 24;
		anglechange = TRUE;
	}
	if (key == FASTROTR) {
		s.tip -= 24;
		anglechange = TRUE;
	}
	if (key == FASTER) {
		frames--;
	}
	if (key == SLOWER) {
		frames++;
	}
	if (key == RESET) {
		initvars();
	}
	if (key == SHOT) {
		// define torpedo
		if (nroftorpedos < MAXTORPEDOS) {
			t[nroftorpedos].x = s.x + s.rottopx;
			t[nroftorpedos].y = s.y + s.rottopy;
			// s.tip has direction in degrees 0 = top 90 = left
			angle = s.tip * PI / 180;
			t[nroftorpedos].xspeed = s.xspeed + -1 * 10 * sin(angle);
			t[nroftorpedos].yspeed = s.yspeed + 10 * cos(angle);
			t[nroftorpedos].radius = 2;
			t[nroftorpedos].active = TRUE;
			nroftorpedos++;
			std::cout << s.tip << " ";
		}
	}

	// wrap the degrees so that s.tip is always between 0 and 360
	if (s.tip < 0) { s.tip = 360; };
	if (s.tip > 360) { s.tip = 0; };

	// if there was ship movement calulate the new position
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
		// flame
		s.rotflameleftx = s.flameleftx * cos(angle) - s.flamelefty * sin(angle);
		s.rotflamelefty = s.flameleftx * sin(angle) + s.flamelefty * cos(angle);
		s.rotflamebottomx = s.flamebottomx * cos(angle) - s.flamebottomy * sin(angle);
		s.rotflamebottomy = s.flamebottomx * sin(angle) + s.flamebottomy * cos(angle);
		s.rotflamerightx = s.flamerightx * cos(angle) - s.flamerighty * sin(angle);
		s.rotflamerighty = s.flamerightx * sin(angle) + s.flamerighty * cos(angle);
	}

	display();
}


boolean shipcollisondetected(rock r1) {
	int dist, rad;

	dist = (r1.x - s.x) * (r1.x - s.x) + (r1.y - s.y) * (r1.y - s.y);
	rad = (r1.radius + s.radius) * (r1.radius + s.radius);
	if (dist <= rad)
		return TRUE;
	else
		return FALSE;
}


boolean rockcollisondetected(rock r1, rock r2) {
	int dist, rad;

	dist = (r1.x - r2.x) * (r1.x - r2.x) + (r1.y - r2.y) * (r1.y - r2.y);
	rad = (r1.radius + r2.radius) * (r1.radius + r2.radius);
	if (dist <= rad)
		return TRUE;
	else
		return FALSE;
}


boolean torpedocollisondetected(rock r1, torpedo t) {
	int dist, rad;

	dist = (r1.x - t.x) * (r1.x - t.x) + (r1.y - t.y) * (r1.y - t.y);
	rad = (r1.radius + t.radius) * (r1.radius + t.radius);
	if (dist <= rad)
		return TRUE;
	else
		return FALSE;
}


void changespeed(int i, int j) {
	int c;

	// 33% percent chance of changing only x, only y or both
	// first for rock i then for rock j
	switch (c = rand() % 3) {
	case 0:
		rocks[i].xspeed = -1 * rocks[i].xspeed;
		break;
	case 1:
		rocks[i].yspeed = -1 * rocks[i].yspeed;
		break;
	case 2:
		rocks[i].xspeed = -1 * rocks[i].xspeed;
		rocks[i].yspeed = -1 * rocks[i].yspeed;
		break;
	};

	switch (c = rand() % 3) {
	case 0:
		rocks[j].xspeed = -1 * rocks[j].xspeed;
		break;
	case 1:
		rocks[j].yspeed = -1 * rocks[j].yspeed;
		break;
	case 2:
		rocks[j].xspeed = -1 * rocks[j].xspeed;
		rocks[j].yspeed = -1 * rocks[j].yspeed;
		break;
	}

	// move the first rock out of the radius, avoids collsions right away
	rocks[i].x += rocks[i].xspeed * (rocks[i].radius / rocks[i].xspeed + 1);
	rocks[i].y += rocks[i].yspeed * (rocks[i].radius / rocks[i].xspeed + 1);
}


void idleFunc() {
	int i, j;

	// animate the rock - i for all rocks
	for (i = 0; i < nrofrocks; i++) {
		rocks[i].x += rocks[i].xspeed;
		rocks[i].y += rocks[i].yspeed;
		if (rocks[i].x > SIZEX) { rocks[i].x = 0; }
		if (rocks[i].x < 0) { rocks[i].x = SIZEX; }
		if (rocks[i].y > SIZEY) { rocks[i].y = 0; }
		if (rocks[i].y < 0) { rocks[i].y = SIZEY; }
	}

	// was there a collsion between any 2 rocks
	for (i = 0; i < nrofrocks; i++) {
		for (j = i + 1; j < nrofrocks; j++) {
			if (rocks[i].active && rocks[j].active) {
				if (rockcollisondetected(rocks[i], rocks[j])) {
					changespeed(i, j);
				}
			}
		}
	}

	// was there a collison between ship and rock
	for (i = 0; i < nrofrocks; i++) {
		if (rocks[i].active) {
			if (shipcollisondetected(rocks[i])) {
				score -= rocks[i].size;
				shipcollision = TRUE;
				rocks[i].yspeed = -1 * rocks[i].yspeed;
				// move the first rock out of the radius
				rocks[i].x += rocks[i].xspeed * ((rocks[i].radius / abs(rocks[i].xspeed)) + 1);
				rocks[i].y += rocks[i].yspeed * ((rocks[i].radius / abs(rocks[i].yspeed)) + 1);
			}
		}
	}

	// was there a collison between torpedo and rock
	for (j = 0; j < nroftorpedos; j++) {
		if (t[j].active) {
			for (i = 0; i < nrofrocks; i++) {
				if (rocks[i].active) {
					if (torpedocollisondetected(rocks[i], t[j])) {
						std::cout << "Torpedo Hit\n";
						score = score + 50 - rocks[i].size * 10;
						t[j].active = FALSE;
						// split in smaller rocks
						rocks[i].size--;
						if (rocks[i].size == 0) {
							// destroyed
							rocks[i].active = FALSE;
						}
						else {
							setrocksize(i, rocks[i].size);
							rocks[i].radius = rocks[i].radius / 2;
							rocks[nrofrocks].x = rocks[i].x;
							rocks[nrofrocks].y = rocks[i].y;
							rocks[nrofrocks].xspeed = rocks[i].xspeed * -1;
							rocks[nrofrocks].yspeed = rocks[i].yspeed;
							rocks[nrofrocks].radius = rocks[i].radius;
							rocks[nrofrocks].size = rocks[i].size;
							rocks[nrofrocks].active = TRUE;
							setrocksize(nrofrocks, rocks[nrofrocks].size);
							nrofrocks++;
						}
					}
				}
			}
		}
	}

	// animate ship
	s.x += s.xspeed;
	s.y += s.yspeed;
	if (s.x > SIZEX) { s.x = 0; }
	if (s.x < 0) { s.x = SIZEX; }
	if (s.y > SIZEY) { s.y = 0; }
	if (s.y < 0) { s.y = SIZEY; }

	// animate the torpedo
	for (i = 0; i < nroftorpedos; i++) {
		t[i].x = t[i].x + t[i].xspeed;
		t[i].y = t[i].y + t[i].yspeed;
		if (t[i].x > SIZEX || t[i].x < 0 || t[i].y > SIZEY or t[i].y < 0) {
			t[i].active = FALSE;
		}
	}
	if (nroftorpedos > 0) {
		compacttorpedos();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(frames));
	display();
}


/* Main function: GLUT runs as a console application starting at main()  */
int main(int argc, char** argv) {

	initvars();

	glutInit(&argc, argv);                           // Initialize GLUT

	glutCreateWindow("OpenGL Planetoids v1");        // Create a window with the given title
	glutInitWindowSize(SIZEX, SIZEY);                // Set the window's initial width & height
	glutInitWindowPosition(0, 0);                    // Position the window's initial top-left corner
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, SIZEX, 0, SIZEY);

	glutKeyboardFunc(processNormalKeys);             // register the callback for keypress
	glutIdleFunc(idleFunc);

	glutDisplayFunc(display); // Register display callback handler for window re-paint
	glutMainLoop();           // Enter the infinitely event-processing loop
	return 0;
}

