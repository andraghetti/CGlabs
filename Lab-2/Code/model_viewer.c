/*  CG LAB2
Gestione interattiva di una scena 3D mediante controllo
da mouse e da tastiera. I modelli geometrici in scena
sono primitive GLU e mesh poligonali in formato *.m

* INPUT: file .m contenente la mesh a triangoli:
*        Vertex  NUM  x y z
*                .......
*        Normal  NUM  x y z
*                .......
*        Face    NUM f1 f2 f3
*                .......
* OUPUT: visualizzazione della mesh in una finestra OpenGL
*
*
* NB: i vertici di ogni faccia sono in verso orario
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "v3d.h"

#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#include <unistd.h>
#else
#include <GL/freeglut.h>
#include <windows.h>
#endif

#define MAX_V 10000 /* max number of vertices allowed in the mesh model */
#define M_PI 3.141592653589793
#define MESH 4 //mesh count

int 	listname;

int  	wireframe;		/* controls the visualization of primitives via glPolygonMode */
int     orpro;			/* controls the type of projection via gluPerspective and glOrtho */
int     cull;			/* toggles backface culling via glEnable( GL_CULL_FACE ) and glDisable( GL_CULL_FACE ); */
int     mater;			/* controls the material associated with the model via glMaterial */
int     shading;		/* controls the shading model via glShadeModel */
int		DEBUG;			/* print some points for debug */

int initPos = 1;

/* Camera variables */
GLfloat fovy;			/* angolo del punto di vista */
GLfloat camC[3];		/* centro del sistema */
GLfloat camE[3];		/* punto di vista */
GLfloat camU[3];		/* vettore su della camera */

						/* Color & light variables */
GLfloat lightPos[4];	/* light position */
GLfloat ambient[4][4];
GLfloat diffuse[4][4];
GLfloat specular[4][4];
GLfloat shininess[4][4];

/* Meshes variables */
const char*		meshes[MESH-1] = { "pig.m","cactus.m","teapot.m"};
GLfloat			translation[3];		/* traslazione sui 3 assi*/
GLfloat			rotation[3];		/* rotazione sui 3 assi*/
GLfloat			OCS[MESH][16];		//Matrice delle trasformazioni OCS
GLfloat			WCS[MESH][16];		//Matrice delle trasformazioni WCS
GLfloat			VCS[MESH][16];		//Matrice delle trasformazioni VCS
GLfloat			initialPosition[MESH][16] = { { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 },
{ -0.500000, 0.000000, 0.866025, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, -0.866025, 0.000000, -0.500000, 0.000000, 0.483974, 0.000000, -0.933012, 1.000000 },
{ 0.866025, -0.000000, 0.500000, 0.000000, 0.500000, -0.000000, -0.866025, 0.000000, 0.000000, 1.000000, -0.000000, 0.000000, -0.946410, -0.000000, 1.600001, 1.000000 },
{ 0.087156, -0.086824, 0.992404, 0.000000, 0.996195, 0.007596, -0.086824, 0.000000, 0.000000, 0.996195, 0.087156, 0.000000, 0.684873, -0.082266, 0.940310, 1.000000 } };

enum Objects {
	TRACKBALL,
	PIG,
	CACTUS,
	TEAPOT
};

enum Objects selectedMesh = PIG;	/*current selected mesh between meshes array and trackball*/
				
int		WindowWidth = 700;				/* Window appearance */
int		WindowHeight = 700;
GLfloat aspect = 1.0;				/* rapporto larghezza-altezza della viewport */
float	t = 0;						/* for explore scene animation*/

/* Trackball variables */
float tbAngle = 0.0;
float tbAxis[3];
float tbRadius = 1.0;
int tbDragging = 0;
float currentPosition[3];
float lastPosition[3];

enum Modes
{
	MODE_INVALID,
	MODE_CHANGE_EYE_POS,
	MODE_CHANGE_REFERENCE_POS,
	MODE_CHANGE_UP_POS,
	MODE_CHANGE_LIGHT_POS,
	MODE_CHANGE_ZOOM,
	MODE_TRANSFORM,

	MODE_CHANGE_CULLING,
	MODE_CHANGE_WIREFRAME,
	MODE_CHANGE_SHADING,
	MODE_CHANGE_PROJECTION,
	MODE_CHANGE_MATERIAL,

	MODE_TRANSLATE_WCS,
	MODE_ROTATE_WCS,
	MODE_TRANSLATE_OCS,
	MODE_ROTATE_OCS,
	MODE_TRANSLATE_VCS,
	MODE_ROTATE_VCS,

	MODE_EXPLORE_SCENE,

	MODE_DEBUG,
	MODE_PRINT_SYSTEM_STATUS,
	MODE_RESET,
	MODE_QUIT
};
enum Modes mode = MODE_CHANGE_REFERENCE_POS; /* global variable that stores the current mode */

/* =========== PROTOTYPES ============== */
void applyTransform();
void reset();

/* =========== FUNCTIONS ============== */
void computePointOnTrackball(int x, int y, float pointOnTrackball[3])
{
	float zTemp;
	//map to [-1;1]
	pointOnTrackball[0] = -(WindowWidth - 2.0f * x) / WindowWidth;
	pointOnTrackball[1] = (WindowHeight - 2.0f * y) / WindowHeight;

	zTemp = 1.0f - (pointOnTrackball[0] * pointOnTrackball[0]) - (pointOnTrackball[1] * pointOnTrackball[1]);
	pointOnTrackball[2] = (zTemp > 0.0f) ? sqrt(zTemp) : 0.0;

	//printf( "pointOnTrackball = (%.2f, %.2f, %.2f)\n", pointOnTrackball[0], pointOnTrackball[1], pointOnTrackball[2] );
}

void print_sys_status()
{
	system("clear");
	printf("\nSystem status\n\n");
	printf("---------------------------------------------------\n");
	printf("Eye point coordinates :\n");
	printf("x = %.2f, y = %.2f, z = %.2f\n", camE[0], camE[1], camE[2]);
	printf("---------------------------------------------------\n");
	printf("Reference point coordinates :\n");
	printf("x = %.2f, y = %.2f, z = %.2f\n", camC[0], camC[1], camC[2]);
	printf("---------------------------------------------------\n");
	printf("Up vector coordinates :\n");
	printf("x = %.2f, y = %.2f, z = %.2f\n", camU[0], camU[1], camU[2]);
	printf("---------------------------------------------------\n");
	printf("Light position coordinates :\n");
	printf("x = %.2f, y = %.2f, z = %.2f\n", lightPos[0], lightPos[1], lightPos[2]);
	printf("---------------------------------------------------\n");
	printf("Field of view angle (in degree) = %.2f\n", fovy);
	printf("---------------------------------------------------\n");
	if (wireframe)
		printf("Wireframe = YES\n");
	else
		printf("Wireframe = NO\n");
	printf("---------------------------------------------------\n");
}

/* --- Hardware Handlers --- */
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		computePointOnTrackball(x, y, lastPosition);
		tbDragging = 1;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		tbDragging = 0;
	}
	glutPostRedisplay();
}

void motion(int x, int y)
{
	float dx, dy, dz;
	computePointOnTrackball(x, y, currentPosition);
	if (tbDragging)
	{
		dx = (currentPosition[0] - lastPosition[0]);
		dy = (currentPosition[1] - lastPosition[1]);
		dz = (currentPosition[2] - lastPosition[2]);
		if (dx || dy || dz)
		{
			tbAngle = sqrt(dx*dx + dy*dy + dz*dz) * (180.0 / M_PI);
			tbAxis[0] = lastPosition[1] * currentPosition[2] - lastPosition[2] * currentPosition[1];
			tbAxis[1] = lastPosition[2] * currentPosition[0] - lastPosition[0] * currentPosition[2];
			tbAxis[2] = lastPosition[0] * currentPosition[1] - lastPosition[1] * currentPosition[0];
			v3dSet(lastPosition, currentPosition);

			glLoadIdentity();
			glRotatef(tbAngle, tbAxis[0], tbAxis[1], tbAxis[2]);
			glMultMatrixf(WCS[TRACKBALL]);
			glGetFloatv(GL_MODELVIEW_MATRIX, WCS[TRACKBALL]);
		}
		if (DEBUG)
			printf("tbAngle = %.2f tbAxis = (%.2f, %.2f, %.2f)\n", tbAngle, tbAxis[0], tbAxis[1], tbAxis[2]);
	}
	glutPostRedisplay();
}

void mouseWheel(int button, int dir, int x, int y)
{
	if (dir > 0)
	{
		if (fovy > 1)
			fovy -= 1;
	}
	else
	{
		if (fovy < 180)
			fovy += 1;
	}

	glutPostRedisplay();
	return;
}

void keyboard(unsigned char key, int x, int y)
{
	float* pos = NULL;
	GLfloat *element = NULL;
	float step;

	switch (mode) {
	case MODE_CHANGE_EYE_POS:
		pos = camE;
		step = 0.1;
		break;
	case MODE_CHANGE_REFERENCE_POS:
		pos = camC;
		step = 0.1;
		break;
	case MODE_CHANGE_UP_POS:
		pos = camU;
		step = 0.1;
		break;
	case MODE_CHANGE_LIGHT_POS:
		pos = lightPos;
		float distanceOrigin = sqrt(lightPos[0] * lightPos[0] + lightPos[1] * lightPos[1] + lightPos[2] * lightPos[2]);
		step = distanceOrigin / 40;
		break;
	case MODE_CHANGE_ZOOM:
		element = &fovy;
		step = 4.0;
		break;
	case MODE_TRANSFORM:
	case MODE_TRANSLATE_OCS:
	case MODE_TRANSLATE_WCS:
	case MODE_TRANSLATE_VCS:
		pos = translation;
		step = 0.1;
		break;
	case MODE_ROTATE_OCS:
	case MODE_ROTATE_WCS:
	case MODE_ROTATE_VCS:
		pos = rotation;
		step = 5;
		break;
	}

	if (rotation != NULL || translation != NULL)
		switch (key) {
		case 'o': //translate OCS
			mode = MODE_TRANSLATE_OCS;
			printf("\nSelected mode: MODE_TRANSLATE_OCS\n");
			break;
		case 'O': //rotation OCS
			mode = MODE_ROTATE_OCS;
			printf("\nSelected mode: MODE_ROTATE_OCS\n");
			break;
		case 'w': //translate WCS
			mode = MODE_TRANSLATE_WCS;
			printf("\nSelected mode: MODE_TRANSLATE_WCS\n");
			break;
		case 'W': //rotation WCS
			mode = MODE_ROTATE_WCS;
			printf("\nSelected mode: MODE_ROTATE_WCS\n");
			break;
		case 'v': //translate VCS
			mode = MODE_TRANSLATE_VCS;
			break;
		case 'V': //rotation VCS
			mode = MODE_ROTATE_VCS;
			break;
		case '1':
			selectedMesh = PIG;
			printf("\nSelected mesh: PIG\n");
			break;
		case '2':
			selectedMesh = CACTUS;
			printf("\nSelected mesh: CACTUS\n");
			break;
		case '3':
			selectedMesh = TEAPOT;
			printf("\nSelected mesh: TEAPOT\n");
			break;
		}

	if (pos != NULL) {
		switch (key) {
		case 'x': pos[0] -= step; break;
		case 'X': pos[0] += step; break;
		case 'y': pos[1] -= step; break;
		case 'Y': pos[1] += step; break;
		case 'z': pos[2] -= step; break;
		case 'Z': pos[2] += step; break;
		}
		if ((rotation != NULL || translation != NULL) && pos != lightPos) {
			applyTransform();
			v3dSetZero(rotation);
			v3dSetZero(translation);
		}
	}
	if (element != NULL)
		switch (key) {
		case 'f': *element -= step; break;
		case 'F': *element += step; break;
		}

	glutPostRedisplay();


	if (key == 27) //esc
		exit(1);
}

/* --- Draw Routines --- */
void moveCamera() {
	int d = 9.0;
	float controlPoints[6][3] = { { 0,d,d } ,{ d,d,d } ,{ d,d,-d },{ -d,d,-d },{ -d,d,d },{ 0,d,d } };
	float temp[6][3];

	//temp array initialization
	for (int i = 0; i <= 5; i++) {
		temp[i][0] = controlPoints[i][0];
		temp[i][1] = controlPoints[i][1];
		temp[i][2] = controlPoints[i][2];
	}

	//De Casteljau
	for (int i = 1; i <= 5; i++) {
		for (int j = 0; j <= 5 - i; j++) {
			temp[j][0] = (1 - t) * temp[j][0] + t * temp[j + 1][0];
			temp[j][1] = (1 - t) * temp[j][1] + t * temp[j + 1][1];
			temp[j][2] = (1 - t) * temp[j][2] + t * temp[j + 1][2];
		}
	}

	//set point
	v3dSet(camE, temp[0]);

}

void drawAxis(float scale, int drawLetters)
{
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glScalef(scale, scale, scale);
	glBegin(GL_LINES);

	glColor4d(1.0, 0.0, 0.0, 1.0);
	if (drawLetters)
	{
		glVertex3f(.8f, 0.05f, 0.0);  glVertex3f(1.0, 0.25f, 0.0); /* Letter X */
		glVertex3f(0.8f, .25f, 0.0);  glVertex3f(1.0, 0.05f, 0.0);
	}
	glVertex3f(0.0, 0.0, 0.0);  glVertex3f(1.0, 0.0, 0.0); /* X axis      */


	glColor4d(0.0, 1.0, 0.0, 1.0);
	if (drawLetters)
	{
		glVertex3f(0.10f, 0.8f, 0.0);   glVertex3f(0.10f, 0.90f, 0.0); /* Letter Y */
		glVertex3f(0.10f, 0.90f, 0.0);  glVertex3f(0.05, 1.0, 0.0);
		glVertex3f(0.10f, 0.90f, 0.0);  glVertex3f(0.15, 1.0, 0.0);
	}
	glVertex3f(0.0, 0.0, 0.0);  glVertex3f(0.0, 1.0, 0.0); /* Y axis      */


	glColor4d(0.0, 0.0, 1.0, 1.0);
	if (drawLetters)
	{
		glVertex3f(0.05f, 0, 0.8f);  glVertex3f(0.20f, 0, 0.8f); /* Letter Z*/
		glVertex3f(0.20f, 0, 0.8f);  glVertex3f(0.05, 0, 1.0);
		glVertex3f(0.05f, 0, 1.0);   glVertex3f(0.20, 0, 1.0);
	}
	glVertex3f(0.0, 0.0, 0.0);  glVertex3f(0.0, 0.0, 1.0); /* Z axis    */

	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);
}

void applyTransform() {
	glLoadIdentity();
	glTranslated(translation[0], translation[1], translation[2]);
	glRotatef(rotation[0], 1, 0, 0);
	glRotatef(rotation[1], 0, 1, 0);
	glRotatef(rotation[2], 0, 0, 1);

	switch (mode) {
	case MODE_TRANSLATE_OCS:
	case MODE_ROTATE_OCS:
		glMultMatrixf(OCS[selectedMesh]);
		glGetFloatv(GL_MODELVIEW_MATRIX, OCS[selectedMesh]);
		break;
	case MODE_TRANSLATE_WCS:
	case MODE_ROTATE_WCS:
		glMultMatrixf(WCS[selectedMesh]);
		glGetFloatv(GL_MODELVIEW_MATRIX, WCS[selectedMesh]);
		break;
	case MODE_TRANSLATE_VCS:
	case MODE_ROTATE_VCS:
		glMultMatrixf(VCS[selectedMesh]);
		glGetFloatv(GL_MODELVIEW_MATRIX, VCS[selectedMesh]);
		break;
	}
}

void display()
{
	//apply material & light
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient[mater]);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse[mater]);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular[mater]);
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess[mater]);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	if (cull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (shading)
		glShadeModel(GL_SMOOTH);
	else
		glShadeModel(GL_FLAT);

	

	glMatrixMode(GL_PROJECTION); //================================= GL_PROJECTION
	glLoadIdentity();
	float fovr = fovy / 20;
	if (orpro)
		glOrtho(-fovr, fovr, -fovr, fovr, -fovr, fovy);
	else
		gluPerspective(fovy, aspect, 1, 100);

	

	glMatrixMode(GL_MODELVIEW); //================================= GL_MODELVIEW
	
	int mesh;
	glLoadIdentity();


	/*for (mesh = 1; mesh < MESH; mesh++) {
		glPushMatrix();
		glMultMatrixf(VCS[mesh]);
		glPopMatrix();
	}*/

	gluLookAt(camE[0], camE[1], camE[2], camC[0], camC[1], camC[2], camU[0], camU[1], camU[2]);

	glMultMatrixf(WCS[TRACKBALL]);				//applico la rotazione della trackball (quindi anche luce)

	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);//nella posizione della trackball applico la luce
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	if (DEBUG) {
		//Assi WCS
		drawAxis(3, 1);
		// point Light
		glPointSize(15);
		glColor3f(0.0f, 0.9f, 0.3f); //default
		glBegin(GL_POINT);
		glVertex3f(lightPos[0], lightPos[1], lightPos[2]);
		glEnd();
	}

	
	for (mesh = 1; mesh < MESH; mesh++) {
		glPushMatrix();
			glMultMatrixf(WCS[mesh]);				//applico le trasformazioni rispetto al WCS
			glMultMatrixf(initialPosition[mesh]);	//applico la posizione iniziale rispetto al WCS
			if (DEBUG) drawAxis(1, 0);				//disegno gli assi nella posizione iniziale dell'oggetto
			glMultMatrixf(OCS[mesh]);				//applico le trasformazioni rispetto all'OCS
			glCallList(mesh);						//disegno l'oggetto nella posizione derivante da WCS, posizione iniziale, OCS
		glPopMatrix();
	}
	


	glutSwapBuffers();
}

void menu(int sel) {
	switch (sel) {
	case MODE_CHANGE_EYE_POS:
	case MODE_CHANGE_REFERENCE_POS:
	case MODE_CHANGE_UP_POS:
	case MODE_CHANGE_ZOOM:
	case MODE_CHANGE_LIGHT_POS:
	case MODE_TRANSFORM:
	case MODE_EXPLORE_SCENE:
		mode = sel;
		break;

	case MODE_CHANGE_CULLING:
		cull = !cull;
		break;
	case MODE_CHANGE_WIREFRAME:
		wireframe = !wireframe;
		break;
	case MODE_CHANGE_PROJECTION:
		orpro = !orpro;
		break;
	case MODE_CHANGE_SHADING:
		shading = !shading;
		break;
	case MODE_CHANGE_MATERIAL:
		mater = (mater + 1) % 4;
		break;
	case MODE_DEBUG:
		DEBUG = !DEBUG;
		break;
	case MODE_RESET:
		reset();
		break;
	case MODE_QUIT:
		exit(0);
		break;
	case MODE_PRINT_SYSTEM_STATUS:
		print_sys_status();
		break;
	}

	glutPostRedisplay();
}

/* --- Init Functions --- */
void loadMesh(int meshIndex)
{
	int 	faces[MAX_V][3];    /* faces */
	float 	vertices[MAX_V][3]; /* vertices */
	float 	vnormals[MAX_V][3]; /* vertex normals */ //una normale per ogni vertice
	float   fnormals[MAX_V][3]; /* face normals */ //una normale per ogni faccia

	int i, j, nrighe;
	int ii, ids[3];
	FILE * idf;
	char s[10];
	float a, b, c;
	int nface, nvert, noused;
	int* face;
	float* vert;

	for (i = 0; i<MAX_V; i++)
	{
		for (j = 0; j<3; j++)
		{
			fnormals[i][j] = 0.0f;
			vnormals[i][j] = 0.0f;
		}
	}

	//apertura del file *.m
	printf("Apertura del file...\n");
	char path[255] = "../data/";
	strcat(path, meshes[meshIndex]);

	if ((idf = fopen(path, "r")) == NULL)
	{
		strcat(path, ": file non trovato\n");
		perror(path);
		exit(1);
	}

	i = 0;
	nface = 0;
	nvert = 0;

	while (!feof(idf)) //finchè non finisce il file, leggiamo tutto e caricamo tutti i vertici e le facce.
	{
		fscanf(idf, "%s %d %f %f %f", s, &noused, &a, &b, &c);
		switch (s[0])
		{
		case 'V':
			vert = vertices[nvert];
			nvert++;

			vert[0] = a;
			vert[1] = b;
			vert[2] = c;
			break;
		case 'N':
			printf(" Vertex normals not used! \n");
			break;
		case 'F':
			face = faces[nface];

			face[0] = (int)a - 1;
			face[1] = (int)b - 1;
			face[2] = (int)c - 1;

			//FACE NORMALS
			float v0[3], v1[3], v2[3], va[3], vb[3], norm[3];

			//set v0,v1,v2
			v3dSet(v0, vertices[face[0]]);
			v3dSet(v1, vertices[face[1]]);
			v3dSet(v2, vertices[face[2]]);

			//set va,vb
			v3dSub(v1, v0, va);
			v3dSub(v2, v0, vb);

			//set the face normal
			v3dCross(va, vb, norm);
			v3dNormalize(norm);

			v3dSet(fnormals[nface], norm);

			nface++; //next face

			break;
		case 'E':
			printf(" Edge not used! \n");
			break;
		default:
			printf(" ERROR: Parsing line \n");
			break;
		}
	}

	//VERTEX NORMALS
	for (i = 0; i < nvert; i++)
	{
		float vnorm[3];
		float cont = 0;

		v3dSetZero(vnorm);
		for (j = 0; j < nface; j++)
		{
			if (faces[j][0] == i || faces[j][1] == i || faces[j][2] == i) {
				vnorm[0] += fnormals[j][0];
				vnorm[1] += fnormals[j][1];
				vnorm[2] += fnormals[j][2];
				cont++;
			}
		}
		v3dMul(vnorm, 1 / cont, vnorm);
		v3dNormalize(vnorm);
		v3dSet(vnormals[i], vnorm);
	}

	nrighe = nvert + nface;
	nface = nface - 1;
	printf("Chiusura del file...Vertices %d Faces %d -> %d\n", nvert, nface, nrighe);
	fclose(idf);

	// crea la display list mesh
	printf("----Display list n#%d\n", listname);
	glNewList(listname, GL_COMPILE);
	for (i = 0; i < nface; i++) {
		ids[0] = faces[i][0];
		ids[1] = faces[i][1];
		ids[2] = faces[i][2];

		//disegna triangoli coi vertici specificati
		glBegin(GL_TRIANGLES);
		glColor3f(1, 0, 0);
		for (ii = 0; ii < 3; ii++)
		{
			//gestione illuminazione
			glNormal3f(vnormals[ids[ii]][0], vnormals[ids[ii]][1], vnormals[ids[ii]][2]);
			glVertex3f(vertices[ids[ii]][0], vertices[ids[ii]][1], vertices[ids[ii]][2]);
		}
		glEnd();
	}
	glEndList();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);	// Make round points, not square points
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);	// Antialias the lines

	/*
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/
}

void initColors() {
	GLfloat brass_ambient[] = { 0.33, 0.22, 0.03, 1.0 }, brass_diffuse[] = { 0.78, 0.57, 0.11, 1.0 }, brass_specular[] = { 0.99, 0.91, 0.81, 1.0 }, brass_shininess[] = { 27.8 };
	GLfloat red_plastic_ambient[] = { 0.0, 0.0, 0.0 }, red_plastic_diffuse[] = { 0.5, 0.0, 0.0 }, red_plastic_specular[] = { 0.7, 0.6, 0.6 }, red_plastic_shininess[] = { 32.0 };
	GLfloat emerald_ambient[] = { 0.0215, 0.1745, 0.0215 }, emerald_diffuse[] = { 0.07568, 0.61424, 0.07568 }, emerald_specular[] = { 0.633, 0.727811, 0.633 }, emerald_shininess[] = { 76.8 };
	GLfloat slate_ambient[] = { 0.02, 0.02, 0.02 }, slate_diffuse[] = { 0.02, 0.01, 0.01 }, slate_specular[] = { 0.4, 0.4, 0.4 }, slate_shininess[] = { .78125 };

	v3dSet(ambient[0], brass_ambient);
	v3dSet(ambient[1], red_plastic_ambient);
	v3dSet(ambient[2], emerald_ambient);
	v3dSet(ambient[3], slate_ambient);

	v3dSet(diffuse[0], brass_diffuse);
	v3dSet(diffuse[1], red_plastic_diffuse);
	v3dSet(diffuse[2], emerald_diffuse);
	v3dSet(diffuse[3], slate_diffuse);

	v3dSet(specular[0], brass_specular);
	v3dSet(specular[1], red_plastic_specular);
	v3dSet(specular[2], emerald_specular);
	v3dSet(specular[3], slate_specular);

	shininess[0][0] = brass_shininess[0];
	shininess[1][0] = red_plastic_shininess[0];
	shininess[2][0] = emerald_shininess[0];
	shininess[3][0] = slate_shininess[0];

	int i; //only for brass
	for (i = 0; i < 4; i++)
		ambient[0][i] = 1.0;
}

void myCreateMenu() {
	glutCreateMenu(menu);
	glutAddMenuEntry("MENU", -1); //-1 significa che non si vuole gestire questa riga
	glutAddMenuEntry("Change eye point (use x,y,z,X,Y,Z)", MODE_CHANGE_EYE_POS);
	glutAddMenuEntry("Change reference point (use x,y,z,X,Y,Z)", MODE_CHANGE_REFERENCE_POS);
	glutAddMenuEntry("Change up vector (use x,y,z,X,Y,Z)", MODE_CHANGE_UP_POS);
	glutAddMenuEntry("Change light position (use x,y,z,X,Y,Z)", MODE_CHANGE_LIGHT_POS);
	glutAddMenuEntry("Zoom (use f,F)", MODE_CHANGE_ZOOM);
	glutAddMenuEntry("Transform Objects (use x,y,z && o,w,v)", MODE_TRANSFORM);
	glutAddMenuEntry("Explore Scene", MODE_EXPLORE_SCENE);

	glutAddMenuEntry("", -1);

	glutAddMenuEntry("Culling", MODE_CHANGE_CULLING);
	glutAddMenuEntry("Wireframe", MODE_CHANGE_WIREFRAME);
	glutAddMenuEntry("Ortographic or Prospective", MODE_CHANGE_PROJECTION);
	glutAddMenuEntry("Shading", MODE_CHANGE_SHADING);
	glutAddMenuEntry("Material", MODE_CHANGE_MATERIAL);

	glutAddMenuEntry("", -1);

	glutAddMenuEntry("Debug", MODE_DEBUG);
	glutAddMenuEntry("Print system status", MODE_PRINT_SYSTEM_STATUS);
	glutAddMenuEntry("Reset", MODE_RESET);
	glutAddMenuEntry("Quit", MODE_QUIT);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void reset()
{
	camE[0] = 8.8;
	camE[1] = 4.9;
	camE[2] = 9.0;

	camC[0] = 0.0;
	camC[1] = 0.0;
	camC[2] = 0.0;

	camU[0] = 0.0;
	camU[1] = 1.0;
	camU[2] = 0.0;

	lightPos[0] = 3.0;
	lightPos[1] = 3.0;
	lightPos[2] = 3.0;
	lightPos[3] = 1.0;

	fovy = 20; //field of view
	wireframe = 0;
	cull = 0;
	mater = 1;
	orpro = 0;
	shading = 1;
	mater = 0;
	selectedMesh = 1;

	mode = MODE_CHANGE_EYE_POS;

	t = 0;


	int j;
	for (j = 0; j < MESH; j++) {
		int i;
		for (i = 0; i < 16; i++) {
			OCS[j][i] = 0.0;
			WCS[j][i] = 0.0;
			VCS[j][i] = 0.0;
		}
		OCS[j][0] = 1;
		OCS[j][5] = 1;
		OCS[j][10] = 1;
		OCS[j][15] = 1;
		WCS[j][0] = 1;
		WCS[j][5] = 1;
		WCS[j][10] = 1;
		WCS[j][15] = 1;
		VCS[j][0] = 1;
		VCS[j][5] = 1;
		VCS[j][10] = 1;
		VCS[j][15] = 1;
	}

	glutPositionWindow(10, 10);
	glutReshapeWindow(700, 700);
	glutPostRedisplay();
}

/* --- Loops --- */
void idle() {
	if (mode == MODE_EXPLORE_SCENE) {
		Sleep(10);
		
		if (t > 1) {
			t = 0;
			printf("finito\n");
		}
		else {
			t += 0.001;
			moveCamera();
		}
		glutPostRedisplay();
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WindowWidth, WindowHeight);
	glutInitWindowPosition(0, 0); // posizione finestra all'avvio
	glutCreateWindow("Model Viewer");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutMouseWheelFunc(mouseWheel);

	myCreateMenu();

	initColors();

	int i;
	listname = glGenLists(3);
	for (i = 0; i < MESH - 1; i++) {
		//pig, cactus, cow
		loadMesh(i);
		listname++;
	}

	reset();

	glutMainLoop();

	return -1;

}
