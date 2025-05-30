#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;
bool textureReplace = true;		


GLdouble* crossProduct(const double* A, const double* B, const double* D) {
	GLdouble* result = new GLdouble[3];


	GLdouble AB_x = B[0] - A[0];
	GLdouble AB_y = B[1] - A[1];
	GLdouble AB_z = B[2] - A[2];


	GLdouble CD_x = D[0] - A[0];
	GLdouble CD_y = D[1] - A[1];
	GLdouble CD_z = D[2] - A[2];


	result[0] = AB_y * CD_z - AB_z * CD_y;
	result[1] = AB_z * CD_x - AB_x * CD_z;
	result[2] = AB_x * CD_y - AB_y * CD_x;

	return result;
}


class CustomCamera : public Camera
{
public:

	double camDist;

	double fi1, fi2;



	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}



	void SetUpCamera()
	{

		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{

		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   

class CustomLight : public Light
{
public:
	CustomLight()
	{

		pos = Vector3(1, 1, 3);
	}


	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);

			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		glLightfv(GL_LIGHT0, GL_POSITION, position);
		
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  



int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;


	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 'E')											
	{
		textureReplace = !textureReplace;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}


GLuint texId;
GLuint texId2;									
void initRender(OpenGL* ogl){


	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	glEnable(GL_TEXTURE_2D);


	RGBTRIPLE* texarray;


	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);					
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	glGenTextures(1, &texId);

	glBindTexture(GL_TEXTURE_2D, texId);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);


	free(texCharArray);
	free(texarray);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);														



	RGBTRIPLE* texarray2;


	char* texCharArray2;
	int texW2, texH2;
	OpenGL::LoadBMP("texture1.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);


	glGenTextures(1, &texId2);

	glBindTexture(GL_TEXTURE_2D, texId2);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);


	free(texCharArray2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	glEnable(GL_NORMALIZE);

	glEnable(GL_LINE_SMOOTH);



	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}


void Render(OpenGL* ogl)
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	if (textureReplace)														
		glBindTexture(GL_TEXTURE_2D, texId);								
	else
		glBindTexture(GL_TEXTURE_2D, texId2);


	//альфаналожение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	glDisable(GL_BLEND);



	GLfloat amb[] = { 0.9, 0.8, 0.8, 1. };
	GLfloat dif[] = { 0.4, 0.25, 0.7, 1. };
	GLfloat spec[] = { 0.9, 0.6, 0.4, 1. };
	GLfloat sh = 0.7f * 256;



	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);

	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);

	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	glShadeModel(GL_SMOOTH);
	



	double A[] = { 9, 6, 0 };
	double B[] = { 17, 4, 0 };
	double C[] = { 11, 9, 0 };
	double D[] = { 14, 15, 0 };
	double E[] = { 9, 10, 0 };
	double F[] = { 4, 15, 0 };
	double G[] = { 0, 9, 0 };
	double H[] = { 5, 1, 0 };

	double A1[] = { 9, 6, 5 };
	double B1[] = { 17, 4, 5 };
	double C1[] = { 11, 9, 5 };
	double D1[] = { 14, 15, 5 };
	double E1[] = { 9, 10, 5 };
	double F1[] = { 4, 15, 5 };
	double G1[] = { 0, 9, 5 };
	double H1[] = { 5, 1, 5 };




	glColor3d(0.3, 0.3, 0.3);



	glBegin(GL_TRIANGLES);
	GLdouble* normal = crossProduct(C, B, A);
	glNormal3d(normal[0], normal[1], normal[2]);					
	glColor3f(0.7f, 0.7f, 1.f);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(1,0);
	glVertex3dv(B);
	glTexCoord2d(1, 1);
	glVertex3dv(C);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(E, D, C);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glTexCoord2d(0, 0);
	glVertex3dv(C);
	glTexCoord2d(1, 1);
	glVertex3dv(D);
	glTexCoord2d(0, 1);
	glVertex3dv(E);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(G, F, E);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G);
	glTexCoord2d(1, 1);
	glVertex3dv(E);
	glTexCoord2d(1, 0);
	glVertex3dv(F);
	glTexCoord2d(0, 0);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A, H, G);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G);
	glTexCoord2d(0, 1);
	glVertex3dv(A);
	glTexCoord2d(1, 0);
	glVertex3dv(H);
	glTexCoord2d(0, 0);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A, E, C);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 1);
	glVertex3dv(E);
	glTexCoord2d(1, 1);
	glVertex3dv(C);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A, E, G);
	glNormal3d(normal[0], normal[1], -normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G);
	glTexCoord2d(0, 0);
	glVertex3dv(E);
	glTexCoord2d(1, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 0);
	glEnd();

	//

	glBegin(GL_TRIANGLES);
	normal = crossProduct(C1, A1, B1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glTexCoord2d(0, 0);
	glVertex3dv(A1);
	glTexCoord2d(1, 0);
	glVertex3dv(B1);
	glTexCoord2d(1, 1);
	glVertex3dv(C1);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(E1, C1, D1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glTexCoord2d(0, 0);
	glVertex3dv(C1);
	glTexCoord2d(1, 1);
	glVertex3dv(D1);
	glTexCoord2d(0, 1);
	glVertex3dv(E1);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(G1, E1, F1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G1);
	glTexCoord2d(1, 1);
	glVertex3dv(E1);
	glTexCoord2d(1, 0);
	glVertex3dv(F1);
	glTexCoord2d(0, 0);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A1, G1, H1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G1);
	glTexCoord2d(0, 1);
	glVertex3dv(A1);
	glTexCoord2d(1, 0);
	glVertex3dv(H1);
	glTexCoord2d(0, 0);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A1, C1, E1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glTexCoord2d(0, 0);
	glVertex3dv(A1);
	glTexCoord2d(0, 1);
	glVertex3dv(E1);
	glTexCoord2d(1, 1);
	glVertex3dv(C1);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A1, G1, E1);
	glNormal3d(normal[0], normal[1], -normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G1);
	glTexCoord2d(0, 0);
	glVertex3dv(E1);
	glTexCoord2d(1, 0);
	glVertex3dv(A1);
	glTexCoord2d(0, 0);
	glEnd();

	//

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	normal = crossProduct(B, B1, A1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 1);
	glVertex3dv(A1);
	glTexCoord2d(1, 1);
	glVertex3dv(B1);
	glTexCoord2d(1, 0);
	glVertex3dv(B);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	normal = crossProduct(C, C1, B1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);			
	glVertex3dv(B);
	glTexCoord2d(0, 1);
	glVertex3dv(B1);
	glTexCoord2d(1, 1);
	glVertex3dv(C1);
	glTexCoord2d(1, 0);
	glVertex3dv(C);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	normal = crossProduct(D, D1, C1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3dv(C);
	glTexCoord2d(1, 0);
	glVertex3dv(C1);
	glTexCoord2d(1, 1);
	glVertex3dv(D1);
	glTexCoord2d(0, 1);
	glVertex3dv(D);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	normal = crossProduct(E, E1, D1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3dv(E);
	glTexCoord2d(1, 0);
	glVertex3dv(E1);
	glTexCoord2d(1, 1);
	glVertex3dv(D1);
	glTexCoord2d(0, 1);
	glVertex3dv(D);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	normal = crossProduct(F, F1, E1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3dv(E);
	glTexCoord2d(1, 0);
	glVertex3dv(E1);
	glTexCoord2d(1, 1);
	glVertex3dv(F1);
	glTexCoord2d(0, 1);
	glVertex3dv(F);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	normal = crossProduct(F, F1, G1);
	glNormal3d(-normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3dv(F);
	glTexCoord2d(0, 1);
	glVertex3dv(F1);
	glTexCoord2d(1, 1);
	glVertex3dv(G1);
	glTexCoord2d(1, 0);
	glVertex3dv(G);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	normal = crossProduct(H, H1, G1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3dv(G);
	glTexCoord2d(1, 0);
	glVertex3dv(G1);
	glTexCoord2d(1, 1);
	glVertex3dv(H1);
	glTexCoord2d(0, 1);
	glVertex3dv(H);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	normal = crossProduct(A, A1, H1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glTexCoord2d(0, 0);
	glVertex3dv(H);
	glTexCoord2d(1, 0);
	glVertex3dv(H1);
	glTexCoord2d(1, 1);
	glVertex3dv(A1);
	glTexCoord2d(0, 1);
	glVertex3dv(A);
	glEnd();

	double x0 = 0;
	double y0 = 9;

	double tx0 = 5;
	double ty0 = 1;




	double Det[] = { 0,0,0 };
	double Det1[] = { 0,0,5 };
	double N[] = { 0,0,0 };
	double Nl;

	for (double i = -0.3; i <= 0.69; i += 0.001)						
	{
		double x = 2.5 - sqrt(89) / 2 * cos(i * 3.141593);
		double y = 5 - sqrt(89) / 2 * sin(i * 3.141593);

		double tx = tx0 + 1.0 / 1000.0;
		double ty = ty0 + 1.0 / 1000.0;

		glBegin(GL_POLYGON);
		glNormal3d(0, 0, -1);
		glColor3d(0.5f, 0.5f, 0.5f);
		glVertex3dv(A);
		glVertex3d(x0, y0, 0);
		glVertex3d(x, y, 0);
		glEnd();

		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glColor3d(0.5f, 0.5f, 0.5f);
		glVertex3dv(A1);
		glVertex3d(x0, y0, 5);
		glVertex3d(x, y, 5);
		glEnd();

		glBegin(GL_QUADS);
		glColor3d(0.3f, 0.3f, 0.3f);
		Det[0] = { x - (2.5 - sqrt(89) / 2 * cos((i - 0.0001) * 3.141593)) };
		Det[1] = { y - (5 - sqrt(89) / 2 * sin((i - 0.0001) * 3.141593)) };
		N[0] = { Det[1] * Det1[2] - Det1[1] * Det[2] };
		N[1] = { -Det[0] * Det1[2] + Det1[0] * Det[2] };
		N[2] = { Det[0] * Det1[1] - Det1[0] * Det[1] };
		Nl = sqrt(N[0] * N[0] + N[1] * N[1] + N[2] * N[2]);
		N[0] = { N[0] / Nl };
		N[1] = { N[1] / Nl };
		N[2] = { N[2] / Nl };
		glNormal3d(N[0], N[1], N[2]);

		glTexCoord2d(tx0, 0);
		glVertex3d(x0, y0, 0);
		glTexCoord2d(tx, 0);
		glVertex3d(x, y, 0);
		glTexCoord2d(ty, 0.5);
		glVertex3d(x, y, 5);
		glTexCoord2d(ty0, 0.5);
		glVertex3d(x0, y0, 5);
		glEnd();

		x0 = x;
		y0 = y;
		tx0 = tx;
		ty0 = ty;
	}



	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	x0 = 0;
	y0 = 9;

	for (double i = -0.3; i <= 0.69; i += 0.001)
	{
		double x = 2.5 - sqrt(89) / 2 * cos(i * 3.141593);
		double y = 5 - sqrt(89) / 2 * sin(i * 3.141593);

		glBegin(GL_POLYGON);
		glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
		glNormal3d(0, 0, 1);
		glVertex3dv(A);
		glVertex3d(x0, y0, 5);
		glVertex3d(x, y, 5);
		glEnd();

		x0 = x;
		y0 = y;
	}


	glBegin(GL_TRIANGLES);
	normal = crossProduct(C1, A1, B1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(E1, C1, D1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glVertex3dv(E1);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(G1, E1, F1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G1);
	glVertex3dv(E1);
	glVertex3dv(F1);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A1, G1, H1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G1);
	glVertex3dv(A1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A1, C1, E1);
	glNormal3d(normal[0], normal[1], normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(A1);
	glVertex3dv(E1);
	glVertex3dv(C1);
	glEnd();

	glBegin(GL_TRIANGLES);
	normal = crossProduct(A1, G1, E1);
	glNormal3d(normal[0], normal[1], -normal[2]);
	glColor3f(0.7f, 0.7f, 1.f);
	glVertex3dv(G1);
	glVertex3dv(E1);
	glVertex3dv(A1);
	glEnd();


	glDisable(GL_BLEND);





	


	glMatrixMode(GL_PROJECTION);	
	
	glPushMatrix();    				    
	glLoadIdentity();	  
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 

	glMatrixMode(GL_MODELVIEW);		
	glPushMatrix();			  
	glLoadIdentity();		 

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   
	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 200 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "E - Переключение текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	ss << "UV-развёртка" << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}