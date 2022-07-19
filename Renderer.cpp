#include "Renderer.h"
#include <stdio.h>
#include <stdlib.h>


void draw_center(void) //좌표축을 그리는 함수
{
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f); /* R */
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.2f, 0.0f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'x');

	glBegin(GL_LINES);
	glColor3f(0.0f, 1.0f, 0.0f); /* G */
	glVertex3f(0.0f, 0.2f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.2f, 0.0f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'y');

	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f); /* B */
	glVertex3f(0.0f, 0.0f, -0.2f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glEnd();
	glRasterPos3f(0.0f, 0.0f, -0.2f);
	glutBitmapCharacter(GLUT_BITMAP_9_BY_15, 'z');
}

void idle() {
	static GLuint previousClock = glutGet(GLUT_ELAPSED_TIME);
	static GLuint currentClock = glutGet(GLUT_ELAPSED_TIME);
	static GLfloat deltaT;

	currentClock = glutGet(GLUT_ELAPSED_TIME);
	deltaT = currentClock - previousClock;
	if (deltaT < 1000.0 / 20.0) { return; }
	else { previousClock = currentClock; }

	//char buff[256];
	//sprintf_s(buff, "Frame Rate = %f", 1000.0 / deltaT);
	//frameRate = buff;

	glutPostRedisplay();
}

void close()
{
	glDeleteTextures(1, &dispBindIndex);
	glutLeaveMainLoop();
	CloseHandle(hMutex);
}

void add_quats(float q1[4], float q2[4], float dest[4])
{
	static int count = 0;
	float t1[4], t2[4], t3[4];
	float tf[4];

	vcopy(q1, t1);//t1에 q1을 복사
	vscale(t1, q2[3]);//t1을 q2[3]배 늘임

	vcopy(q2, t2);
	vscale(t2, q1[3]);

	vcross(q2, q1, t3);
	vadd(t1, t2, tf);
	vadd(t3, tf, tf);
	tf[3] = q1[3] * q2[3] - vdot(q1, q2);

	dest[0] = tf[0];
	dest[1] = tf[1];
	dest[2] = tf[2];
	dest[3] = tf[3];

	if (++count > RENORMCOUNT) {
		count = 0;
		normalize_quat(dest);
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(58, (double)width / height, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	GLfloat spin_quat[4];
	float gain;
	gain = 2.0; /* trackball gain */

	if (drag_state == GLUT_DOWN)
	{
		if (button_state == GLUT_LEFT_BUTTON)
		{
			trackball(spin_quat,
				(gain * rot_x - 500) / 500,
				(500 - gain * rot_y) / 500,
				(gain * x - 500) / 500,
				(500 - gain * y) / 500);
			add_quats(spin_quat, quat, quat);
		}
		else if (button_state == GLUT_RIGHT_BUTTON)
		{
			t[0] -= (((float)trans_x - x) / 500);
			t[1] += (((float)trans_y - y) / 500);
		}
		else if (button_state == GLUT_MIDDLE_BUTTON)
			t[2] -= (((float)trans_z - y) / 500 * 4);
		else if (button_state == 3 || button_state == 4) // scroll
		{

		}
		//glutPostRedisplay();
	}

	rot_x = x;
	rot_y = y;

	trans_x = x;
	trans_y = y;
	trans_z = y;
}

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			rot_x = x;
			rot_y = y;

			//t[0] = t[0] + 1;


		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			trans_x = x;
			trans_y = y;
		}
		else if (button == GLUT_MIDDLE_BUTTON)
		{
			//trcon = trcon + 1;
			trans_z = y;
		}
		else if (button == 3 || button == 4)
		{
			const float sign = (static_cast<float>(button) - 3.5f) * 2.0f;
			t[2] -= sign * 500 * 0.00015f;
		}
	}

	drag_state = state;
	button_state = button;
}

void vzero(float* v) //0벡터로 초기화하는 함수
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

void vset(float* v, float x, float y, float z) //벡터 초기화 함수
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vsub(const float* src1, const float* src2, float* dst) //두 벡터의 차 dst에 저장하는 함수
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void vcopy(const float* v1, float* v2) //v1벡터를 v2벡터에 복사하는 함수
{
	register int i;
	for (i = 0; i < 3; i++)
		v2[i] = v1[i];
}

void vcross(const float* v1, const float* v2, float* cross) //v1과 v2의 외적을 cross벡터에 저장하는 함수
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

float vlength(const float* v) //벡터의 원점과의 거리
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float* v, float div) //벡터의 div배
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void vnormal(float* v) //크기가 1로 normalize하는 함수 
{
	vscale(v, 1.0f / vlength(v));
}

float vdot(const float* v1, const float* v2) //v1과 v2 내적한 결과
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void vadd(const float* src1, const float* src2, float* dst) //v1과 v2의 합
{
	dst[0] = src1[0] + src2[0];
	dst[1] = src1[1] + src2[1];
	dst[2] = src1[2] + src2[2];
}

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
	float a[3]; /* Axis of rotation */
	float phi;  /* how much to rotate about axis */
	float p1[3], p2[3], d[3];
	float t;

	if (p1x == p2x && p1y == p2y) {
		/* Zero rotation */
		vzero(q);
		q[3] = 1.0;
		return;
	}

	/*
	 * First, figure out z-coordinates for projection of P1 and P2 to
	 * deformed sphere
	 */
	vset(p1, p1x, p1y, tb_project_to_sphere(TRACKBALLSIZE, p1x, p1y));
	vset(p2, p2x, p2y, tb_project_to_sphere(TRACKBALLSIZE, p2x, p2y));

	/*
	 *  Now, we want the cross product of P1 and P2
	 */
	vcross(p2, p1, a);

	/*
	 *  Figure out how much to rotate around that axis.
	 */
	vsub(p1, p2, d);
	t = vlength(d) / (2.0f * TRACKBALLSIZE);

	/*
	 * Avoid problems with out-of-control values...
	 */
	if (t > 1.0) t = 1.0;
	if (t < -1.0) t = -1.0;
	phi = 2.0f * asin(t);

	axis_to_quat(a, phi, q);
}

void axis_to_quat(float a[3], float phi, float q[4])
{
	vnormal(a);
	vcopy(a, q);
	vscale(q, sin(phi / 2.0f));
	q[3] = cos(phi / 2.0f);
}

float tb_project_to_sphere(float r, float x, float y)
{
	float d, t, z;

	d = sqrt(x * x + y * y);
	if (d < r * 0.70710678118654752440f) {    /* Inside sphere */
		z = sqrt(r * r - d * d);
	}
	else {           /* On hyperbola */
		t = r / 1.41421356237309504880f;
		z = t * t / d;
	}
	return z;
}

void normalize_quat(float q[4])
{
	int i;
	float mag;

	mag = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
	for (i = 0; i < 4; i++) q[i] /= mag;
}

void build_rotmatrix(float m[4][4], float q[4])
{
	m[0][0] = 1.0f - 2.0f * (q[1] * q[1] + q[2] * q[2]);
	m[0][1] = 2.0f * (q[0] * q[1] - q[2] * q[3]);
	m[0][2] = 2.0f * (q[2] * q[0] + q[1] * q[3]);
	m[0][3] = 0.0f;

	m[1][0] = 2.0f * (q[0] * q[1] + q[2] * q[3]);
	m[1][1] = 1.0f - 2.0f * (q[2] * q[2] + q[0] * q[0]);
	m[1][2] = 2.0f * (q[1] * q[2] - q[0] * q[3]);
	m[1][3] = 0.0f;

	m[2][0] = 2.0f * (q[2] * q[0] - q[1] * q[3]);
	m[2][1] = 2.0f * (q[1] * q[2] + q[0] * q[3]);
	m[2][2] = 1.0f - 2.0f * (q[1] * q[1] + q[0] * q[0]);
	m[2][3] = 0.0f;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

void InitializeWindow(int argc, char* argv[])
{
	// initialize glut settings
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(1000 / 2, 1000 / 2);

	glutInitWindowPosition(0, 0);

	dispWindowIndex = glutCreateWindow("3D Model");

	trackball(quat, 90.0, 0.0, 0.0, 0.0);

	glutIdleFunc(idle); //큐에 이벤트가 없을 때 실행되는 함수
	glutDisplayFunc(display);
	glutReshapeFunc(reshape); //윈도우 크기가 변하면 Reshape
	glutSpecialFunc(special); //특별한 입력을 받음
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutCloseFunc(close);
	//GLuint image = load   ("./my_texture.bmp");

	//glBindTexture(1,)

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// bind textures
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	reshape(1000, 1000);

	glGenTextures(8, tex_ids);
	//glBindTexture(GL_TEXTURE_2D, dispBindIndex);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	gluPerspective(60, 1, 0.1, 200);
	glTranslatef(t[0], t[1]-1.0f, t[2] - 1.0f);
	glScalef(1, 1, 1);
	GLfloat m[4][4], m1[4][4];
	build_rotmatrix(m, quat);
	gluLookAt(0, 0.2, 2.0, 0, 0, 0, 0, 1.0, 0); //시점

	GLfloat r, g, b;
	glMultMatrixf(&m[0][0]);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	GLfloat diffuse0[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient0[4] = { 0.1, 0.1, 0.1, 1.0 };
	GLfloat specular0[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light0_pos[4] = { 2.0, 2.0, -1.0, 1.0 };
	////GLfloat light0_pos[4] = { 0.3, 0.3, 0.5, 1.0 };
	//GLfloat spot_dir[3] = { -2.0f, 0.0f, -1.0f };

	GLfloat diffuse1[4] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat ambient1[4] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat specular1[4] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat light1_pos[4] = { -2.0, 2.0, -1.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);

	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
	glLightfv(GL_LIGHT1, GL_AMBIENT, ambient1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular1);

	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.005);

	glShadeModel(GL_SMOOTH);

	//floor 바닥
	glBindTexture(GL_TEXTURE_2D, tex_ids[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 276, 276, 0, GL_RGB, GL_UNSIGNED_BYTE, mytexels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glGenerateMipmap(GL_TEXTURE_2D);


	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	for (float fl = -1.0; fl < 1.0; fl = fl + 0.2)
	{
		for (float fr = -1.0; fr < 1.0; fr = fr + 0.2)
		{
			glColor3f(1, 1, 1);
			glTexCoord2d(0.0, 0.0);
			glVertex3f(-0.2 + fl, 0.0, -0.2 + fr);
			glColor3f(1, 1, 1);
			glTexCoord2d(1.0, 0.0);
			glVertex3f(-0.2 + fl, 0.0, 0.2 + fr);
			glColor3f(1, 1, 1);
			glTexCoord2d(1.0, 1.0);
			glVertex3f(0.2 + fl, 0.0, 0.2 + fr);
			glColor3f(1, 1, 1);
			glTexCoord2d(0.0, 1.0);
			glVertex3f(0.2 + fl, 0.0, -0.2 + fr);
		}

	}
	glEnd();


	//wall
	glBindTexture(GL_TEXTURE_2D, tex_ids[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 512, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, wall);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	//뒤쪽 벽
	glTexCoord2d(0.0, 0.0);
	glVertex3f(-1.2, 0.0, 1.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(-1.2, 2.4, 1.2);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(1.2, 2.4, 1.2);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(1.2, 0.0, 1.2);
	
	//오른쪽 벽
	glTexCoord2d(1.0, 1.0);
	glVertex3f(-1.2, 2.4, -1.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(-1.2, 2.4, 1.2);
	glTexCoord2d(0.0, 0.0);
	glVertex3f(-1.2, 0.0, 1.2);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(-1.2, 0.0, -1.2);
	
	//왼쪽벽
	glTexCoord2d(0.0, 1.0);
	glVertex3f(1.2, 0.0, -1.2);
	glTexCoord2d(0.0, 0.0);
	glVertex3f(1.2, 0.0, 1.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(1.2, 2.4, 1.2);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(1.2, 2.4, -1.2);

	glEnd();
	
	//침대
	glBindTexture(GL_TEXTURE_2D, tex_ids[2]);

	glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(0.5f, 0.5f, 0.31f);

	//glTranslatef(0.0f, 0.0f, 1.0f);
	//glColor3f(1.0f, 1.0f, 1.0f);
	//glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	//glTranslatef(1.0f, 1.0f, 0.0f); // <*>

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, bed);
	gluBuild2DMipmaps(GL_TEXTURE_2D,3,1024, 1024, GL_RGB, GL_UNSIGNED_BYTE, bed);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	for (register int i = 0; i < BedVertex.size(); i = i + 4) {
		glColor3f(1, 1, 1);
		glTexCoord2f(BedTexture[i].X, BedTexture[i].Y);
		glVertex3f(BedVertex[i].X, BedVertex[i].Y, BedVertex[i].Z);
		glNormal3f(BedNormal[i].X, BedNormal[i].Y, BedNormal[i].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(BedTexture[i + 1].X, BedTexture[i + 1].Y);
		glVertex3f(BedVertex[i + 1].X, BedVertex[i + 1].Y, BedVertex[i + 1].Z);
		glNormal3f(BedNormal[i + 1].X, BedNormal[i + 1].Y, BedNormal[i + 1].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(BedTexture[i + 2].X, BedTexture[i + 2].Y);
		glVertex3f(BedVertex[i + 2].X, BedVertex[i + 2].Y, BedVertex[i + 2].Z);
		glNormal3f(BedNormal[i + 2].X, BedNormal[i + 2].Y, BedNormal[i + 2].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(BedTexture[i + 3].X, BedTexture[i + 3].Y);
		glVertex3f(BedVertex[i + 3].X, BedVertex[i + 3].Y, BedVertex[i + 3].Z);
		glNormal3f(BedNormal[i + 3].X, BedNormal[i + 3].Y, BedNormal[i + 3].Z);
	}
	glDisable(GL_TEXTURE_2D);
	glEnd();
	
	//소파
	glBindTexture(GL_TEXTURE_2D, tex_ids[3]);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	glTranslatef(-1.4f,-0.33f, -0.05f);
	//glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	//glTranslatef(0.5f, 0.5f, 0.31f);
	//glTranslatef(0.0f, 0.0f, 1.0f);
	//glColor3f(1.0f, 1.0f, 1.0f);
	//glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	//glTranslatef(1.0f, 1.0f, 0.0f); // <*>
	
	
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, sofa);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 1024, 1024, GL_RGB, GL_UNSIGNED_BYTE, sofa);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);

	for (register int i = 0; i < SofaVertex.size(); i = i + 4) {
		glColor3f(1, 1, 1);
		glTexCoord2f(SofaTexture[i].X, SofaTexture[i].Y);
		glVertex3f(SofaVertex[i].X, SofaVertex[i].Y, SofaVertex[i].Z);
		glNormal3f(SofaNormal[i].X, SofaNormal[i].Y, SofaNormal[i].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(SofaTexture[i + 1].X, SofaTexture[i + 1].Y);
		glVertex3f(SofaVertex[i + 1].X, SofaVertex[i + 1].Y, SofaVertex[i + 1].Z);
		glNormal3f(SofaNormal[i + 1].X, SofaNormal[i + 1].Y, SofaNormal[i + 1].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(SofaTexture[i + 2].X, SofaTexture[i + 2].Y);
		glVertex3f(SofaVertex[i + 2].X, SofaVertex[i + 2].Y, SofaVertex[i + 2].Z);
		glNormal3f(SofaNormal[i + 2].X, SofaNormal[i + 2].Y, SofaNormal[i + 2].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(SofaTexture[i + 3].X, SofaTexture[i + 3].Y);
		glVertex3f(SofaVertex[i + 3].X, SofaVertex[i + 3].Y, SofaVertex[i + 3].Z);
		glNormal3f(SofaNormal[i + 3].X, SofaNormal[i + 3].Y, SofaNormal[i + 3].Z);
	}
	glDisable(GL_TEXTURE_2D);
	glEnd();

	
	//냉장고
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_ids[4]);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(0.0f, 1.43f, 0.0f);
	glTranslatef(-0.05f, 0.0f, 0.0f);
	//glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
	//glTranslatef(0.5f, 0.5f, 0.31f);
	//glTranslatef(0.0f, 0.0f, 1.0f);
	//glColor3f(1.0f, 1.0f, 1.0f);
	//glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	//glTranslatef(1.0f, 1.0f, 0.0f); // <*>

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, refrig);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 1024, 1024, GL_RGB, GL_UNSIGNED_BYTE, refrig);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin(GL_QUADS);

	for (register int i = 0; i < RefVertex.size(); i = i + 4) {
		glColor3f(1, 1, 1);
		glTexCoord2f(RefTexture[i].X, RefTexture[i].Y);
		glVertex3f(RefVertex[i].X, RefVertex[i].Y, RefVertex[i].Z);
		glNormal3f(RefNormal[i].X, RefNormal[i].Y, RefNormal[i].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(RefTexture[i + 1].X, RefTexture[i + 1].Y);
		glVertex3f(RefVertex[i + 1].X, RefVertex[i + 1].Y, RefVertex[i + 1].Z);
		glNormal3f(RefNormal[i + 1].X, RefNormal[i + 1].Y, RefNormal[i + 1].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(RefTexture[i + 2].X, RefTexture[i + 2].Y);
		glVertex3f(RefVertex[i + 2].X, RefVertex[i + 2].Y, RefVertex[i + 2].Z);
		glNormal3f(RefNormal[i + 2].X, RefNormal[i + 2].Y, RefNormal[i + 2].Z);

		glColor3f(1, 1, 1);
		glTexCoord2f(RefTexture[i + 3].X, RefTexture[i + 3].Y);
		glVertex3f(RefVertex[i + 3].X, RefVertex[i + 3].Y, RefVertex[i + 3].Z);
		glNormal3f(RefNormal[i + 3].X, RefNormal[i + 3].Y, RefNormal[i + 3].Z);
	}
	glDisable(GL_TEXTURE_2D);
	glEnd();
	

	//탁자
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_ids[5]);
	glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	glTranslatef(-0.1f, 0.07f, -1.0f);
	//glTranslatef(-0.05f, 0.0f, 0.0f);
	//glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
	//glTranslatef(0.5f, 0.5f, 0.31f);
	//glTranslatef(0.0f, 0.0f, 1.0f);
	//glColor3f(1.0f, 1.0f, 1.0f);
	//glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
	//glTranslatef(1.0f, 1.0f, 0.0f); // <*>

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, table);
	//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, 512, 512, GL_RGB, GL_UNSIGNED_BYTE, table);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin(GL_QUADS);

	for (register int i = 0; i < TableVertex.size(); i = i + 4) {
		//glColor3f(1, 1, 1);
		glTexCoord2f(TableTexture[i].X, TableTexture[i].Y);
		glVertex3f(TableVertex[i].X, TableVertex[i].Y, TableVertex[i].Z);
		glNormal3f(TableNormal[i].X, TableNormal[i].Y, TableNormal[i].Z);

		//glColor3f(1, 1, 1);
		glTexCoord2f(TableTexture[i + 1].X, TableTexture[i + 1].Y);
		glVertex3f(TableVertex[i + 1].X, TableVertex[i + 1].Y, TableVertex[i + 1].Z);
		glNormal3f(TableNormal[i + 1].X, TableNormal[i + 1].Y, TableNormal[i + 1].Z);

		//glColor3f(1, 1, 1);
		glTexCoord2f(TableTexture[i + 2].X, TableTexture[i + 2].Y);
		glVertex3f(TableVertex[i + 2].X, TableVertex[i + 2].Y, TableVertex[i + 2].Z);
		glNormal3f(TableNormal[i + 2].X, TableNormal[i + 2].Y, TableNormal[i + 2].Z);

		//glColor3f(1, 1, 1);
		glTexCoord2f(TableTexture[i + 3].X, TableTexture[i + 3].Y);
		glVertex3f(TableVertex[i + 3].X, TableVertex[i + 3].Y, TableVertex[i + 3].Z);
		glNormal3f(TableNormal[i + 3].X, TableNormal[i + 3].Y, TableNormal[i + 3].Z);
	}
	glDisable(GL_TEXTURE_2D);
	glEnd();
	/*
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0);
	glVertex3f(-0.2, -0.2, -0.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(-0.2, 0.2, -0.2);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(0.2, 0.2, -0.2);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(0.2, -0.2, -0.2);

	glTexCoord2d(0.0, 0.0);
	glVertex3f(-0.2, -0.2, 0.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(-0.2, 0.2, 0.2);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(0.2, 0.2, 0.2);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(0.2, -0.2, 0.2);

	glTexCoord2d(0.0, 0.0);
	glVertex3f(-0.2, -0.2, -0.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(-0.2, -0.2, 0.2);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(-0.2, 0.2, 0.2);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(-0.2, 0.2, -0.2);

	glTexCoord2d(0.0, 0.0);
	glVertex3f(0.2, -0.2, -0.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(0.2, -0.2, 0.2);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(0.2, 0.2, 0.2);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(0.2, 0.2, -0.2);

	glTexCoord2d(0.0, 0.0);
	glVertex3f(-0.2, -0.2, -0.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(-0.2, -0.2, 0.2);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(0.2, -0.2, 0.2);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(0.2, -0.2, -0.2);

	glTexCoord2d(0.0, 0.0);
	glVertex3f(-0.2, 0.2, -0.2);
	glTexCoord2d(1.0, 0.0);
	glVertex3f(-0.2, 0.2, 0.2);
	glTexCoord2d(1.0, 1.0);
	glVertex3f(0.2, 0.2, 0.2);
	glTexCoord2d(0.0, 1.0);
	glVertex3f(0.2, 0.2, -0.2);
	glEnd();`
	*/
	glutSwapBuffers();
}


int main(int argc, char* argv[])
{
	//vertex = new Vertex[100000];
	//mymesh = new Meshmodel[100000];

	int i, j, k = 0;
	FILE* f = fopen("./asset/carpet.bmp", "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++)
		{
			mytexels[j][i][0] = data[k * 3 + 2];
			mytexels[j][i][1] = data[k * 3 + 1];
			mytexels[j][i][2] = data[k * 3];
			k++;
		}
	}
	delete[] data;


	k = 0;
	FILE* f2 = fopen("./asset/mymap.bmp", "rb");
	unsigned char info2[54];
	fread(info2, sizeof(unsigned char), 54, f2); // read the 54-byte header
	int width2 = *(int*)&info2[18];
	int height2 = *(int*)&info2[22];
	int size2 = 3 * width2 * height2;
	unsigned char* data2 = new unsigned char[size2]; // allocate 3 bytes per pixel
	fread(data2, sizeof(unsigned char), size2, f2); // read the rest of the data at once
	fclose(f2);
	for (i = 0; i < width2; i++)
		for (j = 0; j < height2; j++)
		{
			wall[j][i][0] = data2[k * 3 + 2];
			wall[j][i][1] = data2[k * 3 + 1];
			wall[j][i][2] = data2[k * 3];
			k++;
		}
	delete[] data2;

	k = 0;
	FILE* f3 = fopen("./asset/bed1.bmp", "rb");
	unsigned char info3[54];
	fread(info3, sizeof(unsigned char), 54, f3); // read the 54-byte header
	int width3 = *(int*)&info3[18];
	int height3 = *(int*)&info3[22];
	int size3 = 3 * width3 * height3;
	unsigned char* data3 = new unsigned char[size3]; // allocate 3 bytes per pixel
	fread(data3, sizeof(unsigned char), size3, f3); // read the rest of the data at once
	fclose(f3);
	for (i = 0; i < width3; i++)
		for (j = 0; j < height3; j++)
		{
			bed[j][i][0] = data3[k * 3 + 2];
			bed[j][i][1] = data3[k * 3 + 1];
			bed[j][i][2] = data3[k * 3];
			k++;
		}
	delete[] data3;
	
	k = 0;
	FILE* f4 = fopen("./asset/blacksofa.bmp", "rb");
	unsigned char info4[54];
	fread(info4, sizeof(unsigned char), 54, f4); // read the 54-byte header
	int width4 = *(int*)&info4[18];
	int height4 = *(int*)&info4[22];
	int size4 = 3 * width4 * height4;
	unsigned char* data4 = new unsigned char[size4]; // allocate 3 bytes per pixel
	fread(data4, sizeof(unsigned char), size4, f4); // read the rest of the data at once
	fclose(f4);
	for (i = 0; i < width4; i++)
		for (j = 0; j < height4; j++)
		{
			sofa[j][i][0] = data4[k * 3 + 2];
			sofa[j][i][1] = data4[k * 3 + 1];
			sofa[j][i][2] = data4[k * 3];
			k++;
		}
	delete[] data4;

	k = 0;
	FILE* f5 = fopen("./asset/ref.bmp", "rb");
	unsigned char info5[54];
	fread(info5, sizeof(unsigned char), 54, f5); // read the 54-byte header
	int width5 = *(int*)&info5[18];
	int height5 = *(int*)&info5[22];
	int size5 = 3 * width5 * height5;
	unsigned char* data5 = new unsigned char[size5]; // allocate 3 bytes per pixel
	fread(data5, sizeof(unsigned char), size5, f5); // read the rest of the data at once
	fclose(f5);
	for (i = 0; i < width5; i++)
		for (j = 0; j < height5; j++)
		{
			refrig[j][i][0] = data5[k * 3 + 2];
			refrig[j][i][1] = data5[k * 3 + 1];
			refrig[j][i][2] = data5[k * 3];
			k++;
		}
	delete[] data5;

	k = 0;
	FILE* f6 = fopen("./asset/wood.bmp", "rb");
	unsigned char info6[54];
	fread(info6, sizeof(unsigned char), 54, f6); // read the 54-byte header
	int width6 = *(int*)&info6[18];
	int height6 = *(int*)&info6[22];
	int size6 = 3 * width6 * height6;
	unsigned char* data6 = new unsigned char[size6]; // allocate 3 bytes per pixel
	fread(data6, sizeof(unsigned char), size6, f6); // read the rest of the data at once
	fclose(f6);
	for (i = 0; i < width6; i++)
		for (j = 0; j < height6; j++)
		{
			table[j][i][0] = data6[k * 3 + 2];
			table[j][i][1] = data6[k * 3 + 1];
			table[j][i][2] = data6[k * 3];
			k++;
		}
	delete[] data6;
	
	LoadObj("./asset/bed.obj", BedVertex,BedTexture,BedNormal, bedscale);

	LoadObj("./asset/Sofa.obj",SofaVertex, SofaTexture, SofaNormal, sofascale);

	LoadObj("./asset/refrigerator.obj",RefVertex,RefTexture, RefNormal, refscale);

	LoadObj("./asset/table.obj", TableVertex, TableTexture, TableNormal, tablescale);

	InitializeWindow(argc, argv);

	display();
	
	glutMainLoop();


	//delete[] vertex;
	//delete[] vertex_color;
	return 0;
} 