#include <stdio.h>
#include <stdlib.h>
#include "glut.h"
#include <math.h>
#include <string.h>

#pragma comment( lib, "glut32.lib")

unsigned char* bufferImage;
unsigned char magic[2];
unsigned int fileSize;
unsigned int offset;
int width;
int height;
unsigned short depth;
float brightness = 1;//0~inf
float contrast = 0;//-1~1
float scale = 1;
int currentWidth = 0;
int currentHeight = 0;
GLdouble lookAtParam[9] = { 0,0,1,0,0,0,0,1,0 };

void RGBmodifier(unsigned char* BGR) {
	for (int i = 0; i < 3; i++) {
		if (BGR[i] * brightness > 255)
			BGR[i] = 255;
		else BGR[i] *= brightness;
		int BGRnew = BGR[i] + (BGR[i] - 128) * contrast;
		if (BGRnew > 255)
			BGR[i] = 255;
		else if (BGRnew < 0)
			BGR[i] = 0;
		else BGR[i] = BGRnew;
	}
}

void drawBMP(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(lookAtParam[0], lookAtParam[1], lookAtParam[2], lookAtParam[3], lookAtParam[4], lookAtParam[5], lookAtParam[6], lookAtParam[7], lookAtParam[8]);
	glScalef(scale, scale, 0);
	glBegin(GL_POINTS); {
		unsigned char BGR[4];
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				memcpy_s(BGR, 4, bufferImage + (i * width + j) * 4, 4);
				RGBmodifier(BGR);
				glColor4f(1.0 * BGR[2] / 255, 1.0 * BGR[1] / 255, 1.0 * BGR[0] / 255, 1.0 * BGR[3] / 255);
				glVertex2f(((2 * j - (float)width) / width) * ((float)width / currentWidth), ((2 * i - (float)height) / height) * ((float)height / currentHeight));
			}
		}
	}
	glEnd();
	glFlush();
}

void drawBMP(int w, int h) {
	if (0 == h)
		h = 1;
	glViewport(0, 0, w, h);
	currentHeight = h;
	currentWidth = w;
}

void function(unsigned char key, int, int) {
	//brightness
	if (key == 'b')
		brightness += 0.1;
	else if (key == 'n' && brightness > 0)
		brightness -= 0.1;
	else if (key == 'm')
		brightness = 1;
	//contrast
	else if (key == 'c' && contrast < 1)
		contrast += 0.1;
	else if (key == 'x' && contrast > -1)
		contrast -= 0.1;
	else if (key == 'z')
		contrast = 0;
	//flip
	else if (key == 'f') {
		lookAtParam[2] = -lookAtParam[2];
		lookAtParam[5] = -lookAtParam[5];
	}
	//rotate
	else if (key == 'r' || key == 't') {
		static const int y[8] = { 1,1,0,-1,-1,-1,0,1 };
		static const int x[8] = { 0,1,1,1,0,-1,-1,-1 };
		static int seq = 0;
		seq = (seq + (key == 'r' ? 1 : -1)) % 8;
		if (seq == -1)
			seq = 7;
		printf("%d", seq);
		lookAtParam[6] = x[seq];
		lookAtParam[7] = y[seq];
	}
	//move
	else if (key == 'w') {
		lookAtParam[1] -= 0.1;
		lookAtParam[4] -= 0.1;
	}
	else if (key == 's') {
		lookAtParam[1] += 0.1;
		lookAtParam[4] += 0.1;
	}
	else if (key == 'a') {
		lookAtParam[0] += 0.1;
		lookAtParam[3] += 0.1;
	}
	else if (key == 'd') {
		lookAtParam[0] -= 0.1;
		lookAtParam[3] -= 0.1;
	}
	else if (key == 'q') {
		lookAtParam[0] = 0;
		lookAtParam[3] = 0;
		lookAtParam[1] = 0;
		lookAtParam[4] = 0;
	}
	//zoom
	else if (key == '[' && scale > 0)
		scale -= 0.1;
	else if (key == ']')
		scale += 0.1;
	else if (key == 'p')
		scale = 1;
	drawBMP();

}

void loadImage(FILE *f) {
	bufferImage = (unsigned char*)malloc(width * height * 4);
	fseek(f, offset, SEEK_SET);
	int _32bit = 0;
	int pad = 0;
	if (depth == 32)
		_32bit = 1;
	else _32bit = 0;
	if (!_32bit) {
		pad = 4 - (width * 3) % 4;
		pad == 4 ? (pad = 0) : 0;
	}
	unsigned char BGR[4];
	BGR[3] = 0;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			fread(BGR, _32bit ? 4 : 3, 1, f);
			memcpy_s(bufferImage + (i * width + j) * 4, 4, BGR, 4);
		}
		fseek(f, pad, SEEK_CUR);
	}
	fclose(f);
}

int glmain(int width, int height, int argc, char **argv) {
	//glut初始化
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
	//初始化窗口
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(width, height);
	glutCreateWindow("BMP Viwer");
	
	
	//设置回调函数
	//glClearColor(0.996, 0.992, 0.901, 1);
	glutDisplayFunc(drawBMP);
	glutReshapeFunc(drawBMP);
	glutKeyboardFunc(function);
	//glutMouseFunc(mousefunc);
	//glutIdleFunc(idleAnimation);
	//主循环
	glutMainLoop();
	return 0;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Welcome to my bitmap renderer written by mkyybx.\n\nUsage: Bitmap Renerer filename.\nB key and N key are used to adjust the birghtness while M key can reset.\n"
			"C key and X key are used to adjust the contrast while Z key can reset.\nF key is used to flip the image.\nWASD keys are used to move the image.\n"
			"R key and T key are used to rotate the image.\n[ key and ] key are used to zoom the image while p key can reset.");
		return 3;
	}
	FILE *f;
	fopen_s(&f,argv[1], "rb");
	fread(magic, 2, 1, f);
	if (magic[0] != 'B' || magic[1] != 'M')
		return 1;
	fread(&fileSize, 4, 1, f);
	fseek(f, 0xA, SEEK_SET);
	fread(&offset, 4, 1, f);
	fseek(f, 0x12, SEEK_SET);
	fread(&width, 4, 1, f);
	fseek(f, 0x16, SEEK_SET);
	fread(&height, 4, 1, f);
	fseek(f, 0x1C, SEEK_SET);
	fread(&depth, 2, 1, f);
	if (depth != 32 && depth != 24)
		return 2;
	loadImage(f);
	glmain(width, height, argc, argv);
	return 0;
}
