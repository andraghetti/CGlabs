#include "../common/common.h"

GLuint         program;
GLint          timeParam;
GLint          vertices2;
GLint          color2;


//triangle	--------------------------------------------------	 
const GLfloat vertices_one[4][2] = { { 0.0, 0.0 },{ 0.5,1.0 },{ 0.5, 1.0 },{ 1.0, 0.0 } };
const GLfloat color_one[4][3] = { { 1.0,0.0,0.0 },{ 0.0,1.0,0.0 },{ 0.0,1.0,0.0 },{ 0.0,0.0,1.0 } };
//square	--------------------------------------------------
const GLfloat vertices_two[4][2] = { { 0.0, 0.0 },{ 0.0,1.0 },{ 1.0, 1.0 },{ 1.0, 0.0 } };
const GLfloat color_two[4][3] = { { 0.0,0.0,1.0 },{ 1.0,0.0,0.0 } ,{ 1.0,1.0,0.0 },{ 0.0,1.0,0.0 }, };

static void init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glColor3f(0.0,0.0,0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0,1.0,0.0,1.0);

    glEnable(GL_DEPTH_TEST);

    glewInit();
    program = initShader("v.glsl", "f.glsl");

    // Setup uniform and attribute prameters
    timeParam = glGetUniformLocation(program, "time");
	vertices2 = glGetAttribLocation(program, "vertices2");
	color2 = glGetAttribLocation(program, "color2");
}

static void draw(void)
{
    /* send elapsed time to shaders */
    glUniform1f(timeParam, glutGet(GLUT_ELAPSED_TIME));

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	glBegin(GL_QUADS);
		glColor3f(color_one[0][0], color_one[0][1], color_one[0][2]);
		glVertexAttrib2fv(vertices2, &vertices_two[0][0]);
        glVertexAttrib3fv(color2, &color_two[0][0]);
        glVertex2fv(vertices_one[0]);
		glColor3f(color_one[1][0], color_one[1][1], color_one[1][2]);
		glVertexAttrib3fv(color2, &color_two[1][0]);
        glVertexAttrib2fv(vertices2, &vertices_two[1][0]);
        glVertex2fv(vertices_one[1]);
		glColor3f(color_one[2][0], color_one[2][1], color_one[2][2]);
		glVertexAttrib3fv(color2, &color_two[2][0]);
        glVertexAttrib2fv(vertices2, &vertices_two[2][0]);
        glVertex2fv(vertices_one[2]);
		glColor3f(color_one[3][0], color_one[3][1], color_one[3][2]);
		glVertexAttrib3fv(color2, &color_two[3][0]);
		glVertexAttrib2fv(vertices2, &vertices_two[3][0]);
		glVertex2fv(vertices_one[3]);
    glEnd();

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Simple GLSL example");
    glutDisplayFunc(draw);
    glutKeyboardFunc(commonKeyboard);

    init();

    glutMainLoop();
    return 0;
}
