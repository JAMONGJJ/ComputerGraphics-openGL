#include "Renderer.h"
#include <time.h>
#include <random>

void draw_center(void)
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

	vcopy(q1, t1);
	vscale(t1, q2[3]);

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
			trans_z = y;
		}
		else if (button == 3 || button == 4)
		{
			const float sign = (static_cast<float>(button)-3.5f) * 2.0f;
			t[2] -= sign * 500 * 0.00015f;
		}
	}

	drag_state = state;
	button_state = button;
}

void vzero(float* v)
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

void vset(float* v, float x, float y, float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vsub(const float *src1, const float *src2, float *dst)
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void vcopy(const float *v1, float *v2)
{
	register int i;
	for (i = 0; i < 3; i++)
		v2[i] = v1[i];
}

void vcross(const float *v1, const float *v2, float *cross)
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

float vlength(const float *v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v, float div)
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void vnormal(float *v)
{
	vscale(v, 1.0f / vlength(v));
}

float vdot(const float *v1, const float *v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void vadd(const float *src1, const float *src2, float *dst)
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
	t = vlength(d) / (2.0f*TRACKBALLSIZE);

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

	d = sqrt(x*x + y*y);
	if (d < r * 0.70710678118654752440f) {    /* Inside sphere */
		z = sqrt(r*r - d*d);
	}
	else {           /* On hyperbola */
		t = r / 1.41421356237309504880f;
		z = t*t / d;
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

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	glutCloseFunc(close);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	// bind textures
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	reshape(1000, 1000);

	/*glGenTextures(1, &dispBindIndex);
	glBindTexture(GL_TEXTURE_2D, dispBindIndex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(20, 1, 0.1, 20);
	glTranslatef(t[0], t[1], t[2] - 1.5f);		// z값에 -1.0f는 내가 보는 장소를 뒤로 빼는 것 값이 커지면 더 뒤로 감 -> 물체가 작아진 효과
	
	GLfloat m[4][4];
	build_rotmatrix(m, quat);
	glMultMatrixf(&m[0][0]);
	glMatrixMode(GL_PROJECTION);

	// Light Source 정의
	// 쉐이딩 적용
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat diffuse[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat ambient[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specular[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light0_pos[4] = {2.0, 2.0, 2.0, 1.0};
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	// 거리에 따른 밝기값의 변화를 추가로 더 줌 -> 좀 더 realistic한 렌더링
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.2);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.1);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.05);
	


	// 텍스처 매핑
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 255, 255, 0, GL_RGB, GL_UNSIGNED_BYTE, cart);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// 텍스처를 입힐 모델 생성
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);
	int vertex_1, vertex_2, vertex_3;
	int normal_1, normal_2, normal_3;
	float newNormal_X, newNormal_Y, newNormal_Z;
	for (int i = 0; i < 32922; i++) {
		vertex_1 = carf[i][0];		// 삼각형을 이루는 점들 중 첫번째 점의 인덱스
		vertex_2 = carf[i][3];		// 삼각형을 이루는 점들 중 두번째 점의 인덱스
		vertex_3 = carf[i][6];		// 삼각형을 이루는 점들 중 세번째 점의 인덱스

		// 위에 인덱스를 가져온 3개의 점들을 가지고 삼각형 생성
		// uv map의 좌표는 임의로 순서대로 (0, 0), (1, 0), (0, 1)로 설정
		glTexCoord2f(0, 0);
		glVertex3f(vertex[vertex_1 - 1].X - 0.1, vertex[vertex_1 - 1].Y, vertex[vertex_1 - 1].Z);			  
		glTexCoord2f(1, 0);				  
		glVertex3f(vertex[vertex_2 - 1].X - 0.1, vertex[vertex_2 - 1].Y, vertex[vertex_2 - 1].Z);
		glTexCoord2f(0, 1);				  
		glVertex3f(vertex[vertex_3 - 1].X - 0.1, vertex[vertex_3 - 1].Y, vertex[vertex_3 - 1].Z);
	}
	glDisable(GL_TEXTURE_2D);
	glEnd();



	//텍스처를 입히지 않는 모델 생성
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < 32922; i++) {
		vertex_1 = carf[i][0];	normal_1 = carf[i][2];	// 첫번째 점의 좌표와 노말벡터
		vertex_2 = carf[i][3];	normal_2 = carf[i][5];	// 두번째 점의 좌표와 노말벡터
		vertex_3 = carf[i][6];	normal_3 = carf[i][8];	// 세번째 점의 좌표와 노말벡터
		
		// 세 점의 노말벡터의 x, y, z 방향의 평균을 각각 계산, 새로운 노말벡터 (newNormal_X, newNormal_Y, newNormal_Z) 생성
		newNormal_X = (carvn[normal_1 - 1].X + carvn[normal_2 - 1].X + carvn[normal_3 - 1].X) / 3;
		newNormal_Y = (carvn[normal_1 - 1].Y + carvn[normal_2 - 1].Y + carvn[normal_3 - 1].Y) / 3;
		newNormal_Z = (carvn[normal_1 - 1].Z + carvn[normal_2 - 1].Z + carvn[normal_3 - 1].Z) / 3;
		
		glNormal3f(newNormal_X, newNormal_Y, newNormal_Z);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(vertex[vertex_1 - 1].X + 0.1, vertex[vertex_1 - 1].Y, vertex[vertex_1 - 1].Z);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(vertex[vertex_2 - 1].X + 0.1, vertex[vertex_2 - 1].Y, vertex[vertex_2 - 1].Z);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(vertex[vertex_3 - 1].X + 0.1, vertex[vertex_3 - 1].Y, vertex[vertex_3 - 1].Z);
	}

	glEnd();
	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	vertex = new Vertex[16453];
	carvn = new Vertex[16613];

	FILE* fp;
	fp = fopen("car.obj", "r");
	int count = 0;
	char ch;
	float x, y, z;
	for (register int j = 0; j < 16453; j++) {
		count = fscanf(fp, "%c %f %f %f", &ch, &x, &y, &z);
		if (count == 4 && ch == 'v') {
			vertex[j].X = x / scale;
			vertex[j].Y = y / scale;
			vertex[j].Z = z / scale;
		}
		else
			j -= 1;
	}
	fclose(fp);

	// carvn.txt 파일 읽어오기
	fp = fopen("carvn.txt", "r");
	char* str;
	for (register int j = 0; j < 16613; j++) {
		count = fscanf(fp, "%s %f %f %f", &str, &x, &y, &z);
		if (count == 4) {
			carvn[j].X = x;
			carvn[j].Y = y;
			carvn[j].Z = z;
		}
	}
	fclose(fp);

	// cart.bmp 파일 읽어오기
	FILE* f = fopen("cart.bmp", "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
											   // extract image height and width from header 
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size];
	fread(data, sizeof(unsigned char), size, f);
	fclose(f);
	int k = 0;
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++) {
			cart[j][i][0] = data[k * 3 + 2];
			cart[j][i][1] = data[k * 3 + 1];
			cart[j][i][2] = data[k * 3];
			k++;
		}

	// carf.txt 파일 읽어오기
	string map1 = "", map2 = "", map3 = "";
	ifstream fin;
	fin.open("carf.txt");
	for (int i = 0; i < 32922; i++) {
		fin >> ch >> map1 >> map2 >> map3;
		for (int j = 0; j < 12; j++) {
			if (j < 3) {
				carf[i][j] = atoi(map1.substr(0, map1.find("/")).c_str());
				map1.erase(0, map1.find("/") + 1);
			}
			else if (j < 6) {
				carf[i][j] = atoi(map2.substr(0, map2.find("/")).c_str());
				map2.erase(0, map2.find("/") + 1);
			}
			else {
				carf[i][j] = atoi(map3.substr(0, map3.find("/")).c_str());
				map3.erase(0, map3.find("/") + 1);
			}
		}
	}
	fin.close();

	InitializeWindow(argc, argv);

	display();

	glutMainLoop();
	delete[] vertex;
	delete[] carvn;
	return 0;
}
