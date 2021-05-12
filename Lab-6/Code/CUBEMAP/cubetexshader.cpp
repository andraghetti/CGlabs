#include "../common/common.h"
#include "RgbImage.h"
GLuint program;
GLuint tex;

const char* cubeMap[6] = {
	"textures/3posx.bmp",
	"textures/3negx.bmp",
	"textures/3posy.bmp",
	"textures/3negy.bmp",
	"textures/3posz.bmp",
	"textures/3negz.bmp"
};

RgbImage loadTextureFromFile(const char *filename) {
	RgbImage theTexMap;
	if (!RgbImageInitFile(&theTexMap, filename)) {
		exit(1);
	}

	return theTexMap;
}
void init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
	glEnable(GL_TEXTURE_CUBE_MAP);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	/*
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE,GL_REFLECTION_MAP);
	*/
	//load file
	//devono essere tutti uguali sennò diventa nero
	GLint internalFormat = GL_RGB; //3
	GLsizei height = 256, width = 256;
	RgbImage right = loadTextureFromFile(cubeMap[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, internalFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData(&right)); //right

	RgbImage left = loadTextureFromFile(cubeMap[1]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, internalFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData(&left)); //left

	RgbImage top = loadTextureFromFile(cubeMap[2]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, internalFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData(&top)); //top

	RgbImage bottom = loadTextureFromFile(cubeMap[3]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, internalFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData(&bottom)); //bottom

	RgbImage back = loadTextureFromFile(cubeMap[4]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, internalFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData(&back)); //back

	RgbImage front = loadTextureFromFile(cubeMap[5]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, internalFormat, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData(&front)); //front

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-2.0, 2.0, -2.0, 2.0, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);


	glewInit();
	program = initShader("v.glsl", "f.glsl");

	GLuint texMapLocation;
	texMapLocation = glGetUniformLocation(program, "texMap");
	glUniform1i(texMapLocation, 0);
}

void draw(void)
{
	double t = (double)glutGet(GLUT_ELAPSED_TIME);
	double k = 0.05 * 360.0 / 1000.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glPushMatrix();
	//glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -5.0f);
	glRotatef(k*t, 1.0, 0.0, 0.0);
	glRotatef(k*t, 0.0, 1.0, 0.0);
	glutSolidTeapot(1.0);
	glPopMatrix();

	glutSwapBuffers();
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutCreateWindow("colorcube environment");

	glutDisplayFunc(draw);
	glutKeyboardFunc(commonKeyboard);

	init();

	glutMainLoop();
	return 0;
}
