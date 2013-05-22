#include <GL/glut.h>

int last_x, last_y, spin_x, spin_y;

void draw()
{
	glTranslatef(0, 0, 0.1);
	glutWireCube(0.3);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef((float)spin_x, 0, 1, 0);
	glRotatef((float)spin_y, 1, 0, 0);
	draw();
	glPopMatrix();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, 1, 3.5);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);
	glClearColor(0.2, 0.2, 0.2, 0);
}

void motion(int x, int y)
{
	spin_x += x - last_x;
	spin_y += y - last_y;
	last_x = x;
	last_y = y;
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	if(state & GLUT_UP){

	} else {
		last_x = x;
		last_y = y;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutInit(&argc, argv);
	glutCreateWindow("OpenGL");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutMainLoop();

	return 0;
}