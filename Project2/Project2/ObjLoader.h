#ifndef OBJLOADER_H
#define OBJLOADER_H

#define _CRT_SECURE_NO_DEPRECATE //为了高版本VS中使用fopen（）函数
#include "GL/glut.h"
#include<vector>
#include <fstream>
#include <string>
#include <iostream>
using namespace std;


class ObjLoader {
private:
	vector<vector<GLfloat>>vertexs;	//vertexs
	vector<vector<GLint>>faces;		//faces

public:
	ObjLoader() {};
	ObjLoader(string filename) 
	{
		string line;
		fstream file; 
		file.open(filename, ios::in);
		if (!file.is_open()) {
			cout << "Error open file" << endl;//read file error
			return;
		}

		cout << "Loading file " << filename << "..." << endl;

		while (!file.eof())
		{			
			getline(file, line);//get a line			
			vector <string> parts;//record the new information

			//spilt the line
			int i;
			for (i = 0; i < line.length(); i++)
			{
				// get the first char not " "
				while (line[i] == ' ') {
					i++;
				}
				if (line[i] == '\0') break;

				//get the whole string
				int j = i;
				while (line[j] != '\0' && line[j] != ' ') {
					j++;
				}

				//record in parts
				string part = line.substr(i, j - i);
				parts.push_back(part);
				i = j;
			}

			if (line != "")
			{
				if (parts[0] == "v") // vertexs
				{
					vector<GLfloat> v;
					for (int i = 1; i < 4; i++)
					{
						GLfloat value = atof(parts[i].c_str());// get the x, y, z
						v.push_back(value);//push
					}

					vertexs.push_back(v);
				}
				else if (parts[0] == "f")// faces
				{
					vector<GLint> v_index;
					string strV = "";
					for (int i = 1; i < 4; i++)
					{
						strV = "";
						for (int j = 0; j < parts[i].length(); j++)
						{
							if (parts[i][j] != '//' && parts[i][j] != '\0') //spilt
							{
								strV = strV + parts[i][j];//get string
							}
							else
							{
								break;
							}
						}
						GLint index = atoi(strV.c_str());//convert to integer
						v_index.push_back(index - 1);//push
					}
					faces.push_back(v_index);
				}
			}
		}
		file.close();
		cout << "Load file successfully!" << endl;

		vector<vector<GLint>>::iterator face;
	}

	void Draw()
	{
	vector<vector<GLint>>::iterator face;
	glFrontFace(GL_CW);
	glBegin(GL_TRIANGLES);
	for (face = faces.begin(); face < faces.end(); face++)
	{
		GLint v1 = (*face)[0], v2 = (*face)[1], v3 = (*face)[2];
		//v1
		GLfloat v1x = vertexs[v1][0], v1y = vertexs[v1][1], v1z = vertexs[v1][2];
		//v2
		GLfloat v2x = vertexs[v2][0], v2y = vertexs[v2][1], v2z = vertexs[v2][2];
		//v3
		GLfloat v3x = vertexs[v3][0], v3y = vertexs[v3][1], v3z = vertexs[v3][2];

		//calclut noraml
		GLfloat vec[3];
		vec[0] = (v2y - v1y) * (v3z - v1z) - (v3y - v1y) * (v2z - v1z);
		vec[1] = (v2z - v1z) * (v3x - v1x) - (v3z - v1z) * (v2x - v1x);
		vec[2] = (v2x - v1x) * (v3y - v1y) - (v3x - v1x) * (v2y - v1y);
		GLfloat D = sqrt(pow(vec[0], 2) + pow(vec[1], 2) + pow(vec[2], 2));
		//map to 1
		GLfloat VN[3];
		VN[0] = vec[0] / D;
		VN[1] = vec[1] / D;
		VN[2] = vec[2] / D;

		// draw normal and triangle
		glNormal3f(VN[0], VN[1], VN[2]);
		glTexCoord2f(0.0, 0.0);
		glVertex3f(v1x, v1y, v1z);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(v2x, v2y, v2z);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(v3x, v3y, v3z);
	}
	glEnd();

	}

};




#endif // !OBJLOADER_H
