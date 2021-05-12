/*
 * ConnectDots.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks
 *		and draws Bezier curves.
 *
 * Usage:  
 *   Left click to place a control point.
 *	 Right click on a point to move it. 
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press "1" to draw a bezier curve with the De Casteljau algorithm
 *	 Press "2" to draw a bezier curve with the adaptive subdivision method
 *	 Press "3" to draw more bezier curves interactively
 *	 -> in this mode, middle click on a junction point to change its continuity.
 *	 Press escape to exit.
 */  

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef __APPLE__
#include <OpenGL/gl.h> //OS x libs
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define MAX_CP 64			//max control points
#define clickTolerance 20	//10 to 100
#define TPRECISION 0.01		//precision of t parameter
#define TOL 0.01			//TOL valore di tolleranza per una curva flat

int degree = 3;			//max degree of a bezier curve in INTERACTIVE

enum State{
	DE_CASTELJAU,
	SUBDIVISION,
	INTERACTIVE
};
enum State currentState = DE_CASTELJAU; /* global variable that stores the current mode */

enum Continuity {
	C0,
	C1,
	G1
};

/* PROTOTYPES */ 
void removeFirstPoint ();
void removeLastPoint ();
void addNewPoint (float x, float y);
void changeContinuity(int mode);
void movePoint(int x, int y);
void highlightPoint(int x, int y);

/* GLOBAL VARIABLES */ 
float 	controlPoints[MAX_CP][3]; 	//point array
int 	numCP = 0;					//number of points
float 	junctionContinuity[MAX_CP]; //continuity at a junction point
int		numJP = 0;					//number of junction points
bool 	gotPoint = false;			//if a point is clicked
int 	selectedPoint;				//index of the point found on mouse over
bool 	gotRay; 					//for G1 calculation
float 	ray;						//same

int 	WindowHeight;				// Window size in pixels
int 	WindowWidth;

/* FUNCTION DEFINITIONS */ 

// == Point Management == //
float Lerp(float start, float end, float t) {
	return (1 - t) * start + t * end;
}

void removeFirstPoint() {
	int i;
	if (numCP > 0) {
		//if there are no leftpoints (n completed curves)
		if (numCP% degree == 0) {
			numJP--;
			for (i = 0; i < numJP; i++) {
				junctionContinuity[i] = junctionContinuity[i + 1];
			}
		}
		
		// Remove the first point, slide the rest down
		numCP--;
		for (i = 0; i < numCP; i++) {
			controlPoints[i][0] = controlPoints[i + 1][0];
			controlPoints[i][1] = controlPoints[i + 1][1];
		}
	}
}

void removeLastPoint() {
	if (numCP > 0) {

		if (numCP%degree == 0) {
			numJP--;
		}

		numCP--;
		//this avoid a bug on highlight
		controlPoints[numCP][0] = -1;
		controlPoints[numCP][1] = -1;
	}
}

void addNewPoint(float x, float y) {
	if (numCP >= MAX_CP)
		removeFirstPoint();
	
	if (numCP%degree == 0) {
		junctionContinuity[numJP] = C0;
		numJP++;
	}

	//if previous point is a junction point, this point must be added in continuity
	if (currentState == INTERACTIVE && (numCP-1)%degree == 0 && (junctionContinuity[numJP-1]==C1 || junctionContinuity[numJP-1]==G1)) {
		int junctionPoint = numCP-1;
		int pointBeforeJunction = junctionPoint-1;
		//inverts next point in the base of before-junction point (it's a lerp with t=-1)
		controlPoints[numCP][0] = Lerp(controlPoints[junctionPoint][0], controlPoints[pointBeforeJunction][0], -1);
		controlPoints[numCP][1] = Lerp(controlPoints[junctionPoint][1], controlPoints[pointBeforeJunction][1], -1);

	} else {
		controlPoints[numCP][0] = x;
		controlPoints[numCP][1] = y;
	}
	numCP++;
}

void highlightPoint(int x, int y) {
	float xPos = ((float)x) / ((float)(WindowWidth - 1));
	float yPos = ((float)y) / ((float)(WindowHeight - 1));
	yPos = 1.0f - yPos; // Flip value since y position is from top row.

	for (int i = numCP; i >= 0; i--) { //begins from last added point
		if (round(controlPoints[i][0] * clickTolerance) / clickTolerance == round(xPos*clickTolerance) / clickTolerance
			&& round(controlPoints[i][1] * clickTolerance) / clickTolerance == round(yPos*clickTolerance) / clickTolerance) {

			selectedPoint = i;
			break;
		}
		else {
			selectedPoint = -1;
		}
	}

	if (currentState == INTERACTIVE && selectedPoint%degree == 0) { //if is a junction point, is possibile to change continuity
		glutAttachMenu(GLUT_MIDDLE_BUTTON);
	}
	else {
		glutDetachMenu(GLUT_MIDDLE_BUTTON);
	}

	glutPostRedisplay();
}

void changeContinuity(int mode) {
	if (currentState==INTERACTIVE) {
		int completedCurves = (numCP - 1) / degree;

		if (completedCurves > 0 && junctionContinuity[selectedPoint / degree] != mode) {
			if (selectedPoint%degree == 0 && selectedPoint !=0)
				junctionContinuity[selectedPoint / degree] = mode;

			bool isJunctionPoint = (selectedPoint%degree == 0 && selectedPoint != 0 && selectedPoint != numCP - 1);

			if (isJunctionPoint && (mode == C1 || mode == G1)) {
				//inverts next point in previous point base (it's a lerp with t=-1)
				controlPoints[selectedPoint +1][0] = Lerp(controlPoints[selectedPoint][0], controlPoints[selectedPoint - 1][0], -1);
				controlPoints[selectedPoint +1][1] = Lerp(controlPoints[selectedPoint][1], controlPoints[selectedPoint - 1][1], -1);
			}
			glutPostRedisplay();
		}
	}
}

void getRotationPoints(int rotationCenter, float *newX, float *newY) {
	float tempX,tempY;
	int pointToRotate = rotationCenter + (rotationCenter > selectedPoint ? 1 : -1);
	if (gotRay == false) { //become false when drag stops
										
		tempX = -(controlPoints[pointToRotate][0] - controlPoints[rotationCenter][0]);
		tempY = -(controlPoints[pointToRotate][1] - controlPoints[rotationCenter][1]);

		//get the distance -> ray and save it globaly
		ray = sqrtf(tempX*tempX + tempY*tempY); 
		gotRay = true;
	}

	tempX = -(controlPoints[selectedPoint][0] - controlPoints[rotationCenter][0]);
	tempY = -(controlPoints[selectedPoint][1] - controlPoints[rotationCenter][1]);

	double phi = atan2((double)tempX, (double)tempY);

	//get new points
	*newX = (float) ray*sin(phi) + controlPoints[rotationCenter][0];
	*newY = (float) ray*cos(phi) + controlPoints[rotationCenter][1];
}

// == Hardware Management == //
void movePoint(int x, int y) {
		float xPos = ((float)x) / ((float)(WindowWidth - 1));
		float yPos = ((float)y) / ((float)(WindowHeight - 1));
		yPos = 1.0f - yPos; // Flip value since y position is from top row.

		if (gotPoint) {
			float Xvariation = xPos - controlPoints[selectedPoint][0];
			float Yvariation = yPos - controlPoints[selectedPoint][1];
			
			//apply variation at selectedPoint
			controlPoints[selectedPoint][0] = xPos;
			controlPoints[selectedPoint][1] = yPos;

			if (currentState == INTERACTIVE) {
				int completedCurves = (numCP - 1) / degree;

				if (completedCurves > 0) { //if there are internal junction points
					int nextPoint = selectedPoint + 1;
					int prevPoint = selectedPoint - 1;

					int continuityAtPoint = junctionContinuity[selectedPoint / degree];

					//if selectedPoint is an internal junction point
					if (selectedPoint%degree==0 && selectedPoint != 0 
						&& selectedPoint != numCP - 1 
						&& (continuityAtPoint == C1 || continuityAtPoint == G1)) {
						//moves i-1 and i+1. Valid for C1 and G1 
						controlPoints[nextPoint][0] += Xvariation;
						controlPoints[nextPoint][1] += Yvariation;
						controlPoints[prevPoint][0] += Xvariation;
						controlPoints[prevPoint][1] += Yvariation;
					}

					//if the next point is a junction point
					if (nextPoint%degree == 0) {
						continuityAtPoint = junctionContinuity[nextPoint / degree];
						switch (continuityAtPoint) {
							case C1:
									//apply the variations to the point after the junction point
									controlPoints[nextPoint + 1][0] -= Xvariation;
									controlPoints[nextPoint + 1][1] -= Yvariation;
								break;

							case G1:
									float newX, newY;

									getRotationPoints(nextPoint, &newX, &newY);

									//apply the variations to the point after the junction point
									controlPoints[nextPoint + 1][0] = newX;
									controlPoints[nextPoint + 1][1] = newY;
								break;
							default:
								break;
						}
					}
					//if the previous point is a junction point
					if (prevPoint%degree == 0) {
						continuityAtPoint = junctionContinuity[prevPoint / degree];
						switch (continuityAtPoint) {
						case C1:
							//apply the variations to the point before the junction point
							controlPoints[prevPoint-1][0] -= Xvariation;
							controlPoints[prevPoint-1][1] -= Yvariation;
							break;

						case G1:
							float newX, newY;
							getRotationPoints(prevPoint, &newX, &newY);

							//apply the variations to the point before the junction point
							controlPoints[prevPoint-1][0] = newX;
							controlPoints[prevPoint-1][1] = newY;
							break;
						default:
							break;
						}
					}
					
				}
			}
			glutPostRedisplay();
		}
}

void mouse(int button, int state, int x, int y) {

	float xPos = ((float)x) / ((float)(WindowWidth - 1));
	float yPos = ((float)y) / ((float)(WindowHeight - 1));
	yPos = 1.0f - yPos; // Flip value since y position is from top row.

	//Add new point
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		addNewPoint(xPos, yPos);
		glutPostRedisplay();
	}

	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (selectedPoint != -1)
			gotPoint = true;
		glutPostRedisplay();
	}

	if (state == GLUT_UP) {
		gotPoint = false;
		selectedPoint = -1;
		gotRay = false;
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key)
	{
		case 'f':
			removeFirstPoint ();
			glutPostRedisplay ();
			break;
		case 'l':
			removeLastPoint ();
			glutPostRedisplay ();
			break;
		case '1':
			currentState = DE_CASTELJAU;
			glutPostRedisplay();
			break;
		case '2':
			currentState = SUBDIVISION;
			glutPostRedisplay();
			break;
		case '3':
			currentState = INTERACTIVE;
			glutPostRedisplay();
			break;
		case 27:  // Escape key
			exit (0);
			break;
	}
}

// == Draw Routines == //
//		    =======BEZIER CURVE - DE CASTELJAU'S ALGORITHM=======          //
//		curveDegree	== degree of the curve that you are drawing with this function
//	firstPointIndex	== the index of the controlPoints array in which you want to begin to draw
void drawBezierCurve(int firstPointIndex,int curveDegree) {
	float **temp = (float **)malloc((curveDegree + 1) * sizeof(float*));
	int k;
	for (k = 0; k < curveDegree + 1; k++) {
		temp[k] = (float *)malloc(sizeof(float) * 3);
	}

	glBegin(GL_LINE_STRIP);
	for (float t = 0; t < 1; t += TPRECISION) { //parameter precision: 100 (e.g. t+=0.001 -> precision 1000)
		//temp array initialization
		for (int i = 0; i <= curveDegree; i++) {
			temp[i][0] = controlPoints[firstPointIndex + i][0];
			temp[i][1] = controlPoints[firstPointIndex + i][1];
			temp[i][2] = controlPoints[firstPointIndex + i][2];
		}

		//De Casteljau
		for (int i = 1; i <= curveDegree; i++) {
			for (int j = 0; j <= curveDegree - i; j++) {
				temp[j][0] = Lerp(temp[j][0], temp[j + 1][0], t);
				temp[j][1] = Lerp(temp[j][1], temp[j + 1][1], t);
				temp[j][2] = Lerp(temp[j][2], temp[j + 1][2], t);
			}
		}

		//point print
		glVertex3f(temp[0][0], temp[0][1], temp[0][2]);
	}
	glEnd();
}

//		    =======BEZIER CURVE - ADAPTIVE SUBIDIVISION ALGORITHM=======          //
bool flatTest(float points[MAX_CP][3]) {
	float vLine[3], vPointFromBegin[3];
	float *begin = points[0];
	float *end = points[numCP - 1];

	vLine[0] = end[0] - begin[0];
	vLine[1] = end[1] - begin[1];
	
	int i = 0;
	for (i = 1; i < numCP - 1; i++) {
		vPointFromBegin[0] = points[i][0] - begin[0];
		vPointFromBegin[1] = points[i][1] - begin[1];

		float distancePointFromBegin = sqrt(vPointFromBegin[0] * vPointFromBegin[0] + vPointFromBegin[1] * vPointFromBegin[1]);
		float dotProduct = vPointFromBegin[0] * vLine[0] + vPointFromBegin[1] * vLine[1];
		float distanceBeginEnd = sqrt(vLine[0] * vLine[0] + vLine[1] * vLine[1]);

		float proj = distancePointFromBegin - (dotProduct)*(dotProduct) / distanceBeginEnd;
		//printf("\nDistance found=%f\n", proj);
		if (proj > TOL)
			return false;
	}
	return true;
}

void drawSubdividedCurve(float points[MAX_CP][3]) {
	bool testResult = flatTest(points);

	if (testResult) { //draw
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < numCP;i++)
			glVertex2f(points[i][0], points[i][1]);
		glEnd();
	} else { //subdivide

		float C1[MAX_CP][3];
		float C2[MAX_CP][3];
		float temp[MAX_CP][3];

		//temp array initialization
		for (int i = 0; i < numCP; i++) {
			temp[i][0] = points[i][0];
			temp[i][1] = points[i][1];
			temp[i][2] = points[i][2];
		}
		C1[0][0] = temp[0][0];
		C1[0][1] = temp[0][1];
		//De Casteljau
		for (int i = 1; i < numCP; i++) {
			for (int j = 0; j < numCP - i; j++) {
				temp[j][0] = Lerp(temp[j][0], temp[j + 1][0], 0.5);
				temp[j][1] = Lerp(temp[j][1], temp[j + 1][1], 0.5);
				temp[j][2] = Lerp(temp[j][2], temp[j + 1][2], 0.5);
			}
			C1[i][0] = temp[0][0];
			C1[i][1] = temp[0][1];
		}
		for (int i = 0; i < numCP; i++) {
			C2[i][0] = temp[i][0];
			C2[i][1] = temp[i][1];
		}

		drawSubdividedCurve(C1);
		drawSubdividedCurve(C2);
	}
}

void display(void) {
	int i;
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw the interpolated points
	for (i = 0; i < numCP; i++) {
		bool foundJunctionPoint = (i%degree == 0);

		if (i == selectedPoint) { // highlights only the point near cursor
			glPointSize(9);
			glColor3f(1.0f, 0.2f, 0.2f); //red

		}
		else if (foundJunctionPoint && currentState == INTERACTIVE) {
			glPointSize(8);
			glColor3f(0.0f, 0.8f, 0.8f); //green
		}
		else {
			glPointSize(5);
			glColor3f(0.0f, 0.6f, 0.6f); //default
		}

		//print point
		glBegin(GL_POINTS);
		glVertex2f(controlPoints[i][0], controlPoints[i][1]);
		glEnd();
	}

	if (numCP > 1)
	{

		// Draw the line segments
		glColor3f(0.6f, 0.6f, 0.6f);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x00FF);
	
			glBegin(GL_LINE_STRIP);
			for (i = 0; i < numCP; i++)
				glVertex2f(controlPoints[i][0], controlPoints[i][1]);
			glEnd();
		glDisable(GL_LINE_STIPPLE);

	
		//------- BEZIER CURVES -------//
		glColor3f(0.1f, 0.6f, 1.0f); //blue

		switch (currentState) {
			/*
			glEnable(GL_MAP1_VERTEX_3); //best on init
			glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, numCP, &controlPoints[0][0]);
			*/
			//---1 step with MapGrid
			/*
			if (numCP>1) {
				glMapGrid1f(100.0,0.0,1.0);
				glEvalMesh1(GL_LINE,0,100);
			}
			*/
			//---2 step with evalCoord
			/*
			case EVALCOORD:
				glBegin(GL_LINE_STRIP);
				if (numCP>1) {
					for (int t=0; t<100; t++) {
						glEvalCoord1f((GLfloat) t/100.0);
					}
				}
				glEnd();
			break;
			*/

			//---3 step with rough De Casteljau Algorithm
		case DE_CASTELJAU:
			drawBezierCurve(0, numCP - 1); //numCP - 1 = curve degree
			break;

			//---4 step with subdivision
		case SUBDIVISION:
			drawSubdividedCurve(controlPoints);
			break;

			//---5 step with C0 C1 G1
		case INTERACTIVE:
			int completedCurves = (numCP - 1) / degree;
			int check = degree*(completedCurves > 0 ? completedCurves : 1);
			int surplusPoints = (numCP - 1) % check + 1;
			int newCurveBegin = (numCP - surplusPoints);
			int tempDegree = surplusPoints - 1;

			//when a curve is in construction (it has less than <degree> control points)
			if (surplusPoints != 0) {
				drawBezierCurve(newCurveBegin, tempDegree);
			}

			//routine that draws all the completed curves
			for (int c = 0; c < numCP; c++) {
				if (c%degree == 0 && completedCurves > 0 && c != 0) {
					drawBezierCurve(c - degree, degree);
				}
			}
			break;
		}
	}
	
	glFlush();
}

void initRendering() {
	glClearColor (1.0f, 1.0f, 1.0f, 1.0f);

	// Make big points and wide lines.  (This may be commented out if desired.)
	glPointSize (5);
	glLineWidth (1);

	// The following commands should induce OpenGL to create round points and 
	//  antialias points and lines.  (This is implementation dependent unfortunately, and
	//  may slow down rendering considerably.)
	//  You may comment these out if you wish.
	glEnable (GL_POINT_SMOOTH);
	glEnable (GL_LINE_SMOOTH);

	glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);	// Make round points, not square points
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);	// Antialias the lines
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
} 

void reshape(int w, int h) {
	WindowHeight = (h > 1) ? h : 2;
	WindowWidth = (w > 1) ? w : 2;
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluOrtho2D (0.0f, 1.0f, 0.0f, 1.0f);	// Always view [0,1]x[0,1].
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
} 

int main(int argc, char **argv) {
	glutInit (&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize (700, 700);
	glutInitWindowPosition (600, 50);
	glutCreateWindow (argv[0]);

	initRendering ();

	glutDisplayFunc (display);
	glutReshapeFunc (reshape);
	glutKeyboardFunc (keyboard);
	glutMouseFunc (mouse);
	glutMotionFunc(movePoint); //with click (drag)
	glutPassiveMotionFunc(highlightPoint); //mouse over

	int main_menu = glutCreateMenu(changeContinuity);
	glutAddMenuEntry("C0", C0);
	glutAddMenuEntry("C1", C1);
	glutAddMenuEntry("G1", G1);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);

	glutMainLoop ();
	return 0;						   // This line is never reached
}
