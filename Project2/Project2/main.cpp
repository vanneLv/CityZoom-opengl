#define _CRT_SECURE_NO_DEPRECATE //Ϊ�˸߰汾VS��ʹ��fopen��������

#include <stdlib.h>
#include<stdio.h>
#include<windows.h>
#include<math.h>
#include "ObjLoader.h"

#define PI 3.14159265359
#define BITMAP_ID 0x4D42  
#define BMP_Header_Length 54//ͼ���������ڴ���е�ƫ����  
#define BUFSIZE 100

static char head[54] = {
	0x42,0x4d,0x66,0x75,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,
	0x00,0x28,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x64,0x00,0x00,0x00,
	0x01,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x30,0x75
};//0-9,a-f,0-9,a-f,0-3

static GLint windowWidthH = 800, windowHeightH = 800;
//����ID
GLuint texName[50];

double eye[] = { 0, 1.5, -8 };
double center[] = { 0, 1.5, 0 };
//�ӽǷ���
double frotate1 = 0.0;
double frotate2 = 0.0;
double direction = 0;//վ��λ�ü�ǰ������

ObjLoader chair;
ObjLoader building1;
ObjLoader building2;
ObjLoader building3;
ObjLoader lamp;
ObjLoader teachhall;
ObjLoader base;
ObjLoader leaves;
ObjLoader wheel;


int power_of_two(int n)
{
	if (n <= 0)
		return 0;
	return (n & (n - 1)) == 0;
}
//����bmp��ʽ������������ID

//��������,����߳�
void Draw_SolidCube(GLuint *tex)
{
	//��ʼ�߳�Ϊ1
	//�ԣ�0.0.0��Ϊ���ĵġ��߳�Ϊ1��������
	GLfloat v[8][3] = { { 0.5,0.5,0.5 },{ 0.5,-0.5,0.5 },{ -0.5,-0.5,0.5 },{ -0.5,0.5,0.5 },
	{ 0.5, 0.5,-0.5 },{ 0.5,-0.5,-0.5 },{ -0.5,-0.5,-0.5 },{ -0.5,0.5,-0.5 } };
	//the 6 six face
	//˳�������桢���桢���桢���桢���桢����
	int face[6][4] = { { 0,1,2,3 },{ 4,5,1,0 },{ 7,6,5,4 },{ 3,2,6,7 },{ 0,3,7,4 },{ 1,5,6,2 } };
	

	//draw 6 faces
	for (int i = 0; i < 6; i++)
	{
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glBegin(GL_POLYGON);
		glTexCoord2i(1, 1); glVertex3fv(v[face[i][0]]);
		glTexCoord2i(1, 0); glVertex3fv(v[face[i][1]]);
		glTexCoord2i(0, 0); glVertex3fv(v[face[i][2]]);
		glTexCoord2i(0, 1); glVertex3fv(v[face[i][3]]);
		glEnd();
	}
}
GLuint load_tex(const char* file_name)
{

	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID = 0, texture_ID = 0;

	// ���ļ������ʧ�ܣ�����  
	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
	{
		printf("Error! Open file Failed!");
		return 0;
	}

	// ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�  
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���  
	GLint line_bytes = width * 3;
	while (line_bytes % 4 != 0)
		++line_bytes;
	total_bytes = line_bytes * height;

	// �����������ֽ��������ڴ�  
	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		printf("Error! Malloc pixels space Failed!");
		return 0;
	}

	// ��ȡ��������  
	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		printf("Error! Read pixels Failed!");
		return 0;
	}

	// �Ծ;ɰ汾�ļ��ݣ����ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������  
	// ��ͼ���߳�����OpenGL�涨�����ֵ��Ҳ����  
	GLint max;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
	if (!power_of_two(width) || !power_of_two(height) || width > max || height > max)
	{
		const GLint new_width = 256;
		const GLint new_height = 256; // �涨���ź��µĴ�СΪ�߳���������  
		GLint new_line_bytes, new_total_bytes;
		GLubyte* new_pixels = 0;

		// ����ÿ����Ҫ���ֽ��������ֽ���  
		new_line_bytes = new_width * 3;
		while (new_line_bytes % 4 != 0)
			++new_line_bytes;
		new_total_bytes = new_line_bytes * new_height;

		// �����ڴ�  
		new_pixels = (GLubyte*)malloc(new_total_bytes);
		if (new_pixels == 0)
		{
			printf("Error! Malloc newpixels space Failed!");
			free(pixels);
			fclose(pFile);
			return 0;
		}

		// ������������  
		gluScaleImage(GL_RGB,
			width, height, GL_UNSIGNED_BYTE, pixels,
			new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

		// �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height  
		free(pixels);
		pixels = new_pixels;
		width = new_width;
		height = new_height;
	}

	GLuint tex_id;
	glGenTextures(1, &tex_id);
	// ���µ������������������������  
	//������
	glBindTexture(GL_TEXTURE_2D, tex_id);
	//����������˷�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //������С��minifying��ʱ���ڽ�����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //�����Ŵ�magnifying��ʱ���Թ���
																	  //���������Ʒ�ʽ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//��S��(��x��ȼ�)��ʹ���ظ�����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//��T��(��y��ȼ�)��ʹ���ظ�����
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	//ʹ��ͼ����������
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);    // ����������

	return tex_id;
}


void Draw_Wall()
{
	glBindTexture(GL_TEXTURE_2D, texName[1]);//wall 
	glBegin(GL_QUADS);

	//���
	glTexCoord2f(0.0, 0.0);glVertex3f(-40.0, 0.0, 40.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-40.0, 0.0, -40.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-40.0, 40.0, -40.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-40.0, 40.0, 40.0);
	glEnd();

	//�ұ�
	glBindTexture(GL_TEXTURE_2D, texName[2]);//wall 
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(40.0, 0.0, 40.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(40.0, 0.0, -40.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(40.0, 40.0, -40.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(40.0, 40.0, 40.0);
	glEnd();

	//ǰ��
	glBindTexture(GL_TEXTURE_2D, texName[1]);//wall 
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-40.0, 40.0, 40.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(40.0, 40.0, 40.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(40.0, 0.0, 40.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-40.0, 0.0, 40.0);
	glEnd();

	//����
	glBindTexture(GL_TEXTURE_2D, texName[2]);//wall 
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(-40.0, 0.0, -40.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(40.0, 0.0, -40.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(40.0, 40.0, -40.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-40.0, 40.0, -40.0);
	glEnd();
}
void Draw_Floor()
{
	glBindTexture(GL_TEXTURE_2D, texName[3]);//floor
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-40.0, 0.0, 40.0);
	glTexCoord2f(0.0, 10.0); glVertex3f(40.0, 0.0, 40.0);
	glTexCoord2f(10.0, 10.0); glVertex3f(40.0, 0.0, -40.0);
	glTexCoord2f(10.0, 0.0); glVertex3f(-40.0, 0.0, -40.0);
	glEnd();
}
void Draw_Sky()
{
	glBindTexture(GL_TEXTURE_2D, texName[0]);//sky
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-40.0, 40.0, 40.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(40.0, 40.0, 40.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(40.0, 40.0, -40.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-40.0, 40.0, -40.0);
	glEnd();
}
void Draw_Building()
{
	GLuint tex[6] = { texName[4],texName[4] ,texName[4] ,texName[4] ,texName[5],texName[5] };
	glPushMatrix();
	glTranslated(7.5, 6, 2.5);
	glScalef(5, 20, 5);
	Draw_SolidCube(tex);
	glPopMatrix();

	glPushMatrix();
	glTranslated(7.5, 10, 7.5);
	glScalef(5, 20, 5);
	Draw_SolidCube(tex);
	glPopMatrix();
}
void Draw_Road()
{
	glPushMatrix();
	glTranslatef(0, 0, 35);
	//glRotatef(90, 0, 1, 0);
	glScalef(8, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texName[6]);//floor
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-5.0, 0.1, 5.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(5.0, 0.1, 5.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(5.0, 0.1, -5.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-5.0, 0.1, -5.0);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, 15);
	glRotatef(90, 0, 1, 0);
	glScalef(3, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texName[6]);//floor
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-5.0, 0.1, 5.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(5.0, 0.1, 5.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(5.0, 0.1, -5.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-5.0, 0.1, -5.0);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, -5);
	glScalef(8, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texName[6]);//floor
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-5.0, 0.1, 5.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(5.0, 0.1, 5.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(5.0, 0.1, -5.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-5.0, 0.1, -5.0);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.1, -25);
	//glRotatef(90, 0, 1, 0);
	glScalef(1, 1, 3);
	glBindTexture(GL_TEXTURE_2D, texName[9]);//floor
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-5.0, 0.1, 5.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(5.0, 0.1, 5.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(5.0, 0.1, -5.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-5.0, 0.1, -5.0);
	glEnd();
	glPopMatrix();
}
void Draw_Grass()
{
	glPushMatrix();
	glTranslatef(22.5, 0.1, -25);
	glScalef(3.5, 1, 3);
	glBindTexture(GL_TEXTURE_2D, texName[8]);//�ݵ�
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-5.0, 0.1, 5.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(5.0, 0.1, 5.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(5.0, 0.1, -5.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-5.0, 0.1, -5.0);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-22.5, 0.1, -25);
	glScalef(3.5, 1, 3);
	glBindTexture(GL_TEXTURE_2D, texName[8]);//�ݵ�
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex3f(-5.0, 0.1, 5.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(5.0, 0.1, 5.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(5.0, 0.1, -5.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-5.0, 0.1, -5.0);
	glEnd();
	glPopMatrix();
}

void Draw_Building1()
{
	glPushMatrix();
	glTranslatef(-8, 1, 4);//��xyz����ƽ��
	glScalef(1, 1, 1);
	//glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building1.Draw();
	glPopMatrix();
}
void Draw_Building2()
{
	glPushMatrix();
	glTranslatef(10, 3, 3);//��xyz����ƽ��
	glScalef(1, 1, 1);
	//glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building2.Draw();
	glPopMatrix();
}
void Draw_Building3()
{
	glPushMatrix();
	glTranslatef(9, 4, 18);//��xyz����ƽ��
	glScalef(1, 1, 1);
	//glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building3.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(25, 4, 9);//��xyz����ƽ��
	glScalef(1, 1, 1);
	glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building3.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-25, 4, 9);//��xyz����ƽ��
	glScalef(1, 1, 1);
	glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building3.Draw();
	glPopMatrix();
}
void Draw_Chair()
{
	glPushMatrix();
	glTranslatef(7, 0.35, -13);//��xyz����ƽ��
	glScalef(0.1, 0.1, 0.1);
	glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[10]);
	chair.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-7, 0.35, -13);//��xyz����ƽ��
	glScalef(0.1, 0.1, 0.1);
	glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[10]);
	chair.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(7, 0.35, -25);//��xyz����ƽ��
	glScalef(0.1, 0.1, 0.1);
	glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[10]);
	chair.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-7, 0.35, -25);//��xyz����ƽ��
	glScalef(0.1, 0.1, 0.1);
	glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[10]);
	chair.Draw();
	glPopMatrix();
}
void Draw_Lamp()
{
	glPushMatrix();
	glTranslatef(5.5, 0.1, 0.5);//��xyz����ƽ��
	glScalef(0.02, 0.02, 0.02);
	//glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[11]);
	lamp.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-5.5, 0.1, 0.5);//��xyz����ƽ��
	glScalef(0.02, 0.02, 0.02);
	glRotated(180, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[11]);
	lamp.Draw();
	glPopMatrix();
}

void Draw_Windmill()
{
	frotate1 += 10;
	if (frotate1 == 360)frotate1 = 0;

	glPushMatrix();
	glTranslatef(14, 0.25, -20);//��xyz����ƽ��
	glScalef(0.1, 0.05, 0.1);
	glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[12]);
	base.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(14, 8, -17.5);//��xyz����ƽ��
	glScalef(0.2, 0.2, 0.2);
	glRotated(frotate1, 0, 0, 1);
	glRotated(90, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, texName[12]);
	leaves.Draw();
	glPopMatrix();
}
void Draw_FerrisWheel()
{
	frotate2 += 3;
	if (frotate2 == 360)frotate2 = 0;

	glPushMatrix();
	glTranslatef(-14, 0.25, -20);//��xyz����ƽ��
	glScalef(0.1, 0.05, 0.1);
	glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[12]);
	base.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-14, 8, -17.5);//��xyz����ƽ��
	glScalef(5, 5, 5);
	glRotated(frotate2, 0, 0, 1);
	//glRotated(90, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, texName[12]);
	wheel.Draw();
	glPopMatrix();
}

//��Բ��
void Draw_Circle()
{
	glBegin(GL_TRIANGLE_FAN);//����������������δ�  
	glVertex3f(0.0f, 0.0f, 0.0f);
	int i = 0;
	for (i = 0; i <= 390; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(sin(p), cos(p), 0.0f);
	}
	glEnd();
}
//��Բ��
void Draw_Cylinder()
{
	glBegin(GL_QUAD_STRIP);//��������ı��δ�  
	int i = 0;
	for (i = 0; i <= 390; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(sin(p), cos(p), 1.0f);
		glVertex3f(sin(p), cos(p), 0.0f);
	}
	glEnd();
	Draw_Circle();
	glTranslatef(0, 0, 1);
	Draw_Circle();
}
//��Բ׶
void Draw_Cone()
{
	glBegin(GL_QUAD_STRIP);//��������ı��δ�  
	int i = 0;
	for (i = 0; i <= 390; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(0, 0, 1.0f);
		glVertex3f(sin(p), cos(p), 0.0f);
	}
	glEnd();
	Draw_Circle();
}
void Draw_Ball()
{
	glPushMatrix();
	glTranslatef(0, 35, 35);
	glutWireSphere(3, 500, 500);
	glPopMatrix();

}
void SetLights()
{
	GLfloat light_ambient[] = { 1,165.0/255.0,0.0/255.0};
	//	GLfloat light_ambient[] = { 1,1,1 };
	GLfloat light_diffuse[] = { 1,165.0 / 255.0,0.0 / 255.0 };
	GLfloat light_position[] = { 38,1,38,1 };

	
	//�ƹ�
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_diffuse);
	//glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHT1);
}

void init(void)
{
	//�����������
	texName[0] = load_tex("top.bmp");//�����ͼ
	texName[1] = load_tex("dawn.bmp");//�ƻ���ͼ
	texName[2] = load_tex("dark.bmp");//�ƻ谵��ͼ
	texName[3] = load_tex("buttom.bmp");//�����ͼ
	texName[4] = load_tex("window2.bmp");//������ǽ��ͼ
	texName[5] = load_tex("surface2.dib");//����������ͼ
	texName[6] = load_tex("road.dib");//���ٹ�·��ͼ
	texName[7] = load_tex("malu.dib");//��·��ͼ
	texName[8] = load_tex("grass.bmp");//�ݵ���ͼ
	texName[9] = load_tex("xiaolu.dib");//С·��ͼ
	texName[10] = load_tex("chair.dib");//����
	texName[11] = load_tex("lamp.dib");//·��
	texName[12] = load_tex("wood.bmp");
	//����obj��ʽ�ļ�
	chair = ObjLoader("chair.obj");
	building1 = ObjLoader("building1.obj");
	building2 = ObjLoader("building2.obj");
	building3 = ObjLoader("teachhall.obj");
	lamp = ObjLoader("lamp.obj");
	teachhall = ObjLoader("teachhall.obj");
	base = ObjLoader("fengchebase.obj");
	leaves = ObjLoader("fengcheye.obj");
	wheel=  ObjLoader("revolve.obj");



}
void display(void)
{
	//Clear
	glClearDepth(1.0f);
	//glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(80.0f, 1.0f, 1.0f, 20.0f);
	glMatrixMode(GL_MODELVIEW);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//set the camare position
	gluLookAt(eye[0], eye[1], eye[2], center[0], center[1], center[2], 0, 1, 0);
	glShadeModel(GL_FLAT);

	//�ƹ�
	SetLights();
	//���
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	//����
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);


	glEnable(GL_TEXTURE_2D);//��������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//����Ͳ��ʻ�Ϸ�ʽ

	glPushMatrix();
	Draw_Wall();
	Draw_Floor();
	Draw_Sky();
	Draw_Grass();
	//Draw_Building();
	Draw_Road();
	Draw_Building1();
	Draw_Building2();
	Draw_Chair();
	Draw_Lamp();
	Draw_Building3();
	Draw_Windmill();
	Draw_FerrisWheel();
	Draw_Ball();
	glPopMatrix();
	glutSwapBuffers();//����˫����
}
void reshape(int width, int height)
{
	//reset the viewport
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)width / (GLfloat)height, 0.01, 120.0);
	glMatrixMode(GL_MODELVIEW);
}
void idle()
{
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {

	case 'a':
		direction = direction + 3; break;
	case 'd':
		direction = direction - 3; break;

	case 'w':

		eye[0] = eye[0] + 0.5 * sin(direction * PI / 180);
		eye[2] = eye[2] + 0.5 * cos(direction * PI / 180);
		break;

	case 's':
		eye[0] = eye[0] - 0.5 * sin(direction * PI / 180);
		eye[2] = eye[2] - 0.5 * cos(direction * PI / 180);
		break;	

	case 'z':
		center[1] += 1;
		break;
	case 'c':
		center[1] -= 1;
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	center[0] = eye[0] + 8 * sin(direction * PI / 180); //���ĺ��۾���ֱ�߾���Ϊ8
	center[2] = eye[2] + 8 * cos(direction * PI / 180);
	
	eye[1] = center[1];
	glutPostRedisplay();
}

void grab(GLint width, GLint height)//����/���溯��
{
	GLint pixelLength;
	GLubyte* pixelDate;
	FILE* file;

	file = fopen("C:\\Users\\ASUS\\Pictures\\capture", "wb");
	fwrite(head, 54, 1, file);
	fseek(file, 0x0012, SEEK_SET);
	fwrite(&width, sizeof(width), 1, file);
	fwrite(&height, sizeof(height), 1, file);

	//Ϊ���ط����ڴ�
	pixelLength = width * 3;
	if (pixelLength % 4 != 0)
		pixelLength += 4 - pixelLength % 4;
	pixelLength *= height;
	pixelDate = (GLubyte*)malloc(pixelLength);
	if (pixelDate == 0)
		printf("Error! Malloc pixels space Failed!\n");//�����ڴ�ʧ��

	//��ȡ�������ز�����bmpͼ
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glReadPixels(0, 0, width, height, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixelDate);
	fseek(file, 0, SEEK_END);
	fwrite(pixelDate, pixelLength, 1, file);
	fclose(file);
	free(pixelDate);
}
void Menu(int value)
{
	if (value == 1)
		grab(windowWidthH, windowHeightH);
	else if (value == 2)
		exit(0);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(windowWidthH, windowHeightH);
	glutInitWindowPosition(80, 80);
	glutCreateWindow("City Zoom");

	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	//glutMouseFunc(mouse);
	glutCreateMenu(Menu);
	glutAddMenuEntry("Screenshot", 1);//������һ���Ҽ�������ť
	glutAddMenuEntry("Exit", 2);//������һ���Ҽ��˳���ť
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutIdleFunc(idle);//�����ó������ʱ����idle��ֱ���д����¼�����
	glutMainLoop();

	return 0;
}
