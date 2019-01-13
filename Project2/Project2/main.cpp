#define _CRT_SECURE_NO_DEPRECATE //为了高版本VS中使用fopen（）函数

#include <stdlib.h>
#include<stdio.h>
#include<windows.h>
#include<math.h>
#include "ObjLoader.h"

#define PI 3.14159265359
#define BITMAP_ID 0x4D42  
#define BMP_Header_Length 54//图像数据在内存块中的偏移量  
#define BUFSIZE 100

static char head[54] = {
	0x42,0x4d,0x66,0x75,0x00,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,
	0x00,0x28,0x00,0x00,0x00,0x64,0x00,0x00,0x00,0x64,0x00,0x00,0x00,
	0x01,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x30,0x75
};//0-9,a-f,0-9,a-f,0-3

static GLint windowWidthH = 800, windowHeightH = 800;
//纹理ID
GLuint texName[50];

double eye[] = { 0, 1.5, -8 };
double center[] = { 0, 1.5, 0 };
//视角方向
double frotate1 = 0.0;
double frotate2 = 0.0;
double direction = 0;//站立位置及前进方向；

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
//载入bmp格式纹理并生成纹理ID

//画立方体,输入边长
void Draw_SolidCube(GLuint *tex)
{
	//初始边长为1
	//以（0.0.0）为中心的、边长为1的立方体
	GLfloat v[8][3] = { { 0.5,0.5,0.5 },{ 0.5,-0.5,0.5 },{ -0.5,-0.5,0.5 },{ -0.5,0.5,0.5 },
	{ 0.5, 0.5,-0.5 },{ 0.5,-0.5,-0.5 },{ -0.5,-0.5,-0.5 },{ -0.5,0.5,-0.5 } };
	//the 6 six face
	//顺序是正面、右面、后面、左面、上面、下面
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

	// 打开文件，如果失败，返回  
	FILE* pFile = fopen(file_name, "rb");
	if (pFile == 0)
	{
		printf("Error! Open file Failed!");
		return 0;
	}

	// 读取文件中图象的宽度和高度  
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	// 计算每行像素所占字节数，并根据此数据计算总像素字节数  
	GLint line_bytes = width * 3;
	while (line_bytes % 4 != 0)
		++line_bytes;
	total_bytes = line_bytes * height;

	// 根据总像素字节数分配内存  
	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		printf("Error! Malloc pixels space Failed!");
		return 0;
	}

	// 读取像素数据  
	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		printf("Error! Read pixels Failed!");
		return 0;
	}

	// 对就旧版本的兼容，如果图象的宽度和高度不是的整数次方，则需要进行缩放  
	// 若图像宽高超过了OpenGL规定的最大值，也缩放  
	GLint max;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
	if (!power_of_two(width) || !power_of_two(height) || width > max || height > max)
	{
		const GLint new_width = 256;
		const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形  
		GLint new_line_bytes, new_total_bytes;
		GLubyte* new_pixels = 0;

		// 计算每行需要的字节数和总字节数  
		new_line_bytes = new_width * 3;
		while (new_line_bytes % 4 != 0)
			++new_line_bytes;
		new_total_bytes = new_line_bytes * new_height;

		// 分配内存  
		new_pixels = (GLubyte*)malloc(new_total_bytes);
		if (new_pixels == 0)
		{
			printf("Error! Malloc newpixels space Failed!");
			free(pixels);
			fclose(pFile);
			return 0;
		}

		// 进行像素缩放  
		gluScaleImage(GL_RGB,
			width, height, GL_UNSIGNED_BYTE, pixels,
			new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

		// 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height  
		free(pixels);
		pixels = new_pixels;
		width = new_width;
		height = new_height;
	}

	GLuint tex_id;
	glGenTextures(1, &tex_id);
	// 绑定新的纹理，载入纹理并设置纹理参数  
	//绑定纹理
	glBindTexture(GL_TEXTURE_2D, tex_id);
	//设置纹理过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //纹理被缩小（minifying）时最邻近过滤
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //纹理被放大（magnifying）时线性过滤
																	  //设置纹理环绕方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//在S轴(与x轴等价)上使用重复环绕
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//在T轴(与y轴等价)上使用重复环绕
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	//使用图像生成纹理
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);    // 解绑纹理对象

	return tex_id;
}


void Draw_Wall()
{
	glBindTexture(GL_TEXTURE_2D, texName[1]);//wall 
	glBegin(GL_QUADS);

	//左边
	glTexCoord2f(0.0, 0.0);glVertex3f(-40.0, 0.0, 40.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-40.0, 0.0, -40.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-40.0, 40.0, -40.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-40.0, 40.0, 40.0);
	glEnd();

	//右边
	glBindTexture(GL_TEXTURE_2D, texName[2]);//wall 
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0); glVertex3f(40.0, 0.0, 40.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(40.0, 0.0, -40.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(40.0, 40.0, -40.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(40.0, 40.0, 40.0);
	glEnd();

	//前面
	glBindTexture(GL_TEXTURE_2D, texName[1]);//wall 
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-40.0, 40.0, 40.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(40.0, 40.0, 40.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(40.0, 0.0, 40.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-40.0, 0.0, 40.0);
	glEnd();

	//后面
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
	glBindTexture(GL_TEXTURE_2D, texName[8]);//草地
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
	glBindTexture(GL_TEXTURE_2D, texName[8]);//草地
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
	glTranslatef(-8, 1, 4);//按xyz方向平移
	glScalef(1, 1, 1);
	//glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building1.Draw();
	glPopMatrix();
}
void Draw_Building2()
{
	glPushMatrix();
	glTranslatef(10, 3, 3);//按xyz方向平移
	glScalef(1, 1, 1);
	//glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building2.Draw();
	glPopMatrix();
}
void Draw_Building3()
{
	glPushMatrix();
	glTranslatef(9, 4, 18);//按xyz方向平移
	glScalef(1, 1, 1);
	//glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building3.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(25, 4, 9);//按xyz方向平移
	glScalef(1, 1, 1);
	glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building3.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-25, 4, 9);//按xyz方向平移
	glScalef(1, 1, 1);
	glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[5]);
	building3.Draw();
	glPopMatrix();
}
void Draw_Chair()
{
	glPushMatrix();
	glTranslatef(7, 0.35, -13);//按xyz方向平移
	glScalef(0.1, 0.1, 0.1);
	glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[10]);
	chair.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-7, 0.35, -13);//按xyz方向平移
	glScalef(0.1, 0.1, 0.1);
	glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[10]);
	chair.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(7, 0.35, -25);//按xyz方向平移
	glScalef(0.1, 0.1, 0.1);
	glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[10]);
	chair.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-7, 0.35, -25);//按xyz方向平移
	glScalef(0.1, 0.1, 0.1);
	glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[10]);
	chair.Draw();
	glPopMatrix();
}
void Draw_Lamp()
{
	glPushMatrix();
	glTranslatef(5.5, 0.1, 0.5);//按xyz方向平移
	glScalef(0.02, 0.02, 0.02);
	//glRotated(-90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[11]);
	lamp.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-5.5, 0.1, 0.5);//按xyz方向平移
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
	glTranslatef(14, 0.25, -20);//按xyz方向平移
	glScalef(0.1, 0.05, 0.1);
	glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[12]);
	base.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(14, 8, -17.5);//按xyz方向平移
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
	glTranslatef(-14, 0.25, -20);//按xyz方向平移
	glScalef(0.1, 0.05, 0.1);
	glRotated(90, 0, 1, 0);
	glBindTexture(GL_TEXTURE_2D, texName[12]);
	base.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-14, 8, -17.5);//按xyz方向平移
	glScalef(5, 5, 5);
	glRotated(frotate2, 0, 0, 1);
	//glRotated(90, 1, 0, 0);
	glBindTexture(GL_TEXTURE_2D, texName[12]);
	wheel.Draw();
	glPopMatrix();
}

//画圆形
void Draw_Circle()
{
	glBegin(GL_TRIANGLE_FAN);//扇形连续填充三角形串  
	glVertex3f(0.0f, 0.0f, 0.0f);
	int i = 0;
	for (i = 0; i <= 390; i += 15)
	{
		float p = i * 3.14 / 180;
		glVertex3f(sin(p), cos(p), 0.0f);
	}
	glEnd();
}
//画圆柱
void Draw_Cylinder()
{
	glBegin(GL_QUAD_STRIP);//连续填充四边形串  
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
//画圆锥
void Draw_Cone()
{
	glBegin(GL_QUAD_STRIP);//连续填充四边形串  
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

	
	//灯光
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
	//绑定所需的纹理
	texName[0] = load_tex("top.bmp");//天空贴图
	texName[1] = load_tex("dawn.bmp");//黄昏贴图
	texName[2] = load_tex("dark.bmp");//黄昏暗贴图
	texName[3] = load_tex("buttom.bmp");//大地贴图
	texName[4] = load_tex("window2.bmp");//大厦外墙贴图
	texName[5] = load_tex("surface2.dib");//建筑表面贴图
	texName[6] = load_tex("road.dib");//高速公路贴图
	texName[7] = load_tex("malu.dib");//马路贴图
	texName[8] = load_tex("grass.bmp");//草地贴图
	texName[9] = load_tex("xiaolu.dib");//小路贴图
	texName[10] = load_tex("chair.dib");//椅子
	texName[11] = load_tex("lamp.dib");//路灯
	texName[12] = load_tex("wood.bmp");
	//加载obj格式文件
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

	//灯光
	SetLights();
	//深度
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	//材质
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);


	glEnable(GL_TEXTURE_2D);//启用纹理
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);//纹理和材质混合方式

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
	glutSwapBuffers();//交换双缓存
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

	center[0] = eye[0] + 8 * sin(direction * PI / 180); //中心和眼睛的直线距离为8
	center[2] = eye[2] + 8 * cos(direction * PI / 180);
	
	eye[1] = center[1];
	glutPostRedisplay();
}

void grab(GLint width, GLint height)//截屏/保存函数
{
	GLint pixelLength;
	GLubyte* pixelDate;
	FILE* file;

	file = fopen("C:\\Users\\ASUS\\Pictures\\capture", "wb");
	fwrite(head, 54, 1, file);
	fseek(file, 0x0012, SEEK_SET);
	fwrite(&width, sizeof(width), 1, file);
	fwrite(&height, sizeof(height), 1, file);

	//为像素分配内存
	pixelLength = width * 3;
	if (pixelLength % 4 != 0)
		pixelLength += 4 - pixelLength % 4;
	pixelLength *= height;
	pixelDate = (GLubyte*)malloc(pixelLength);
	if (pixelDate == 0)
		printf("Error! Malloc pixels space Failed!\n");//分配内存失败

	//读取窗口像素并保存bmp图
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
	glutAddMenuEntry("Screenshot", 1);//创建了一个右键截屏按钮
	glutAddMenuEntry("Exit", 2);//创建了一个右键退出按钮
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	glutIdleFunc(idle);//这里让程序空闲时调用idle，直到有窗口事件发生
	glutMainLoop();

	return 0;
}
