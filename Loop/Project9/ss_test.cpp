#include "Subdivision.h"
#include <GL/glut.h>

#include <math.h>
#include <time.h>

SS_Surface object;
Point cameraPos(0, 0, -350);

float gScale = 800;

int last_x, last_y, spin_x, spin_y;

clock_t start, finish;

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(cameraPos.x, cameraPos.y, cameraPos.z);
	glRotatef((float)spin_x, 0.0, 1.0, 0.0);
	glRotatef((float)spin_y, 1.0, 0.0, 0.0);
	object.Draw((SS_Surface::DRAWFLAGS) (SS_Surface::WIREFRAME | SS_Surface::SOLID));
	glPopMatrix();
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
	if(state & GLUT_UP)
	{
		;
	}
	else
	{
		last_x = x;
		last_y = y;
	}
	glutPostRedisplay();
}

void motion(int x, int y)
{
	spin_x += x - last_x;
	spin_y += y - last_y;
	last_x = x;
	last_y = y;
	glutPostRedisplay();
}


//void InitShape(int shape = 1)
//{
//	Vertex v1, v2, v3, v4, v5, v6, v7, v8;
//
//	v1.pos = Point(-30, 30, 30);
//	v2.pos = Point(-30, 30, -30);
//	v3.pos = Point( 30, 30, -30);
//	v4.pos = Point( 30, 30, 30);
//	v5.pos = Point(-30, -30, 30);
//	v6.pos = Point(-30, -30, -30);
//	v7.pos = Point( 30, -30, -30);
//	v8.pos = Point( 30, -30, 30);
//
//	object.Reset();
//
//	switch( shape )
//	{
//		case 1:
//		{
//			/*
//			 * Rectangle
//			 */
//			object.AddFace( &v1, &v3, &v2 );
//			object.AddFace( &v1, &v4, &v3 );
//			object.AddFace( &v5, &v6, &v8 );
//			object.AddFace( &v6, &v7, &v8 );
//			object.AddFace( &v1, &v2, &v6 );
//			object.AddFace( &v1, &v6, &v5 );
//			object.AddFace( &v2, &v3, &v7 );
//			object.AddFace( &v2, &v7, &v6 );
//			object.AddFace( &v3, &v8, &v7 );
//			object.AddFace( &v3, &v4, &v8 );
//			object.AddFace( &v1, &v5, &v4 );
//			object.AddFace( &v4, &v5, &v8 );
//			break;
//		}
//
//		case 2:
//		{
//			/*
//			 * Triangular Pyramid
//			 */
//			v1.pos = Point(0, 30, 0);
//			v2.pos = Point(0, -30, 0);
//			object.AddFace( &v1, &v6, &v5 );
//			object.AddFace( &v1, &v7, &v6 );
//			object.AddFace( &v1, &v8, &v7 );
//			object.AddFace( &v1, &v5, &v8 );
//			object.AddFace( &v6, &v2, &v5 );
//			object.AddFace( &v6, &v7, &v2 );
//			object.AddFace( &v7, &v8, &v2 );
//			object.AddFace( &v8, &v5, &v2 );
//			break;	
//		}
//
//		case 3:
//		{
//			/*
//			 * Sinusoidal Mesh
//			 */
//			#define MESH_SIZE 10.0
//			Vertex mesh[(int)MESH_SIZE][(int)MESH_SIZE];
//
//			for( int i = 0; i < MESH_SIZE; i++ )
//				for( int j = 0; j < MESH_SIZE; j++ )
//					mesh[i][j].pos = Point(
//							 -30 + (i/MESH_SIZE * 60)
//							, -20 + (j/MESH_SIZE * 40)
//							, 20 * sin( 3.1415926 / 180.0 * i * 2 * 360.0 / MESH_SIZE ) );
//			for( int i = 0; i < MESH_SIZE - 1; i++ )
//				for( int j = 0; j < MESH_SIZE - 1; j++ )
//				{
//					object.AddFace( &mesh[i][j], &mesh[i+1][j], &mesh[i+1][j+1] );
//					object.AddFace( &mesh[i][j], &mesh[i+1][j+1], &mesh[i][j+1] );
//				}
//			break;
//		}
//	}
//}


void renderScene(void)
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glLoadIdentity();
	glTranslatef(cameraPos.x, cameraPos.y, cameraPos.z);

	object.Draw( (SS_Surface::DRAWFLAGS)
					( SS_Surface::WIREFRAME | SS_Surface::SOLID) );

	glutSwapBuffers();
}

void changeSize(int w, int h)
{
	if ( h == 0 )
		h = 1;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glViewport(0, 0, w, h);
	gluPerspective(45, (float)w/(float)h, 1, 10000);

	glMatrixMode( GL_MODELVIEW );
}

void keyboard(unsigned char key, int x, int y)
{
	switch( key )
	{
		case 32: // Space bar
			// Subdivide the object when the space bar is hit
			start = clock();
			object.Subdivide();
			finish = clock();
			printf("time = %lf = %ld\n", (double)(finish - start) / CLOCKS_PER_SEC, finish-start);
			break;

		case 27: // Escape
			exit(0);
			break;

		case GLUT_KEY_UP:
			cameraPos.z += 10;
			break;

		case GLUT_KEY_DOWN:
			cameraPos.z -= 10;
			break;

		case GLUT_KEY_LEFT:
			cameraPos.x -= 10;
			break;

		case GLUT_KEY_RIGHT:
			cameraPos.x += 10;
			break;

		//case 49: // Number 1
		//	InitShape(1);
		//	break;

		//case 50: // Number 2
		//	InitShape(2);
		//	break;

		//case 51: // Number 3
		//	InitShape(3);
		//	break;
		case 'w':
			object.WriteObj("out.obj");
			break;
	}

	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	// Init OpenGL
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB );
	glutInitWindowSize(640, 480);
	glutCreateWindow("Surface Subidivision Test");;

	//glutDisplayFunc(&renderScene);
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(changeSize);
	glutKeyboardFunc(keyboard);

	// Set-up the basic shape
	//InitShape(1);
	if(argc > 1)
		gScale = atof(argv[1]);
	object.ReadObj("face.obj", gScale);

	glutMainLoop();

	return 0;
}
