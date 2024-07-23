#include "olcConsoleGameEngine.h"
using namespace std;



struct vec3d
{
	float x, y, z;
};


struct triangle
{
	vec3d p[3];
};


struct mesh
{
	vector<triangle> tris;
};

struct mat4x4
{
	float m[4][4] = { 0 };
};


class GameEngine3D : public olcConsoleGameEngine
{

public:
	GameEngine3D()
	{
		m_sAppName = L"3D Demo";
	}

private:
	mesh meshCube;
	mat4x4 matProj;
	float fTheta = 0.0f;

	void MultiplyMatrixVector(vec3d& i, vec3d& o, mat4x4& m) // i = input, o = output, m = matrix
	{

		o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
		o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
		o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
		float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

		if (w != 0.0f)
		{
			o.x /= w; o.y /= w; o.z /= w;
		}
	}

public:

	bool OnUserCreate() override
	{
		meshCube.tris = {

			// SOUTH
			{ 0.0f, 0.0f, 0.0f,    0.0f, 2.0f, 0.0f,    2.0f, 2.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 2.0f, 0.0f,    2.0f, 0.0f, 0.0f },

			// EAST                                                      
			{ 2.0f, 0.0f, 0.0f,    2.0f, 2.0f, 0.0f,    2.0f, 2.0f, 2.0f },
			{ 2.0f, 0.0f, 0.0f,    2.0f, 2.0f, 2.0f,    2.0f, 0.0f, 2.0f },

			// NORTH                                                     
			{ 2.0f, 0.0f, 2.0f,    2.0f, 2.0f, 2.0f,    0.0f, 2.0f, 2.0f },
			{ 2.0f, 0.0f, 2.0f,    0.0f, 2.0f, 2.0f,    0.0f, 0.0f, 2.0f },

			// WEST                                                      
			{ 0.0f, 0.0f, 2.0f,    0.0f, 2.0f, 2.0f,    0.0f, 2.0f, 0.0f },
			{ 0.0f, 0.0f, 2.0f,    0.0f, 2.0f, 0.0f,    0.0f, 0.0f, 0.0f },

			// TOP                                                       
			{ 0.0f, 2.0f, 0.0f,    0.0f, 2.0f, 1.0f,    2.0f, 2.0f, 2.0f },
			{ 0.0f, 2.0f, 0.0f,    2.0f, 2.0f, 1.0f,    2.0f, 2.0f, 0.0f },

			// BOTTOM                                                    
			{ 2.0f, 0.0f, 2.0f,    0.0f, 0.0f, 2.0f,    0.0f, 0.0f, 0.0f },
			{ 2.0f, 0.0f, 2.0f,    0.0f, 0.0f, 0.0f,    2.0f, 0.0f, 0.0f },

		};

		//Projection Matrix
		float fNear = 0.1f;
		float fFar = 2000.0f;
		float fFov = 100.0f;
		float aspectRatio = (float)ScreenHeight() / (float)ScreenWidth();
		float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

		matProj.m[0][0] = aspectRatio * fFovRad;
		matProj.m[1][1] = fFovRad;
		matProj.m[2][2] = fFar / (fFar - fNear);
		matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matProj.m[2][3] = 1.0f;
		matProj.m[3][3] = 0.0f;

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		mat4x4 matRotZ, matRotX;

		fTheta += 1.0f * fElapsedTime;

		//Rotation Z
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sin(fTheta);
		matRotZ.m[1][1] = cos(fTheta);
		matRotZ.m[2][2] = 1.0f;
		matRotZ.m[3][3] = 1.0f;

		//Rotation X
		matRotX.m[0][0] = 1.0f;
		matRotX.m[1][1] = cosf(fTheta);
		matRotX.m[1][2] = sinf(fTheta);
		matRotX.m[2][1] = -sinf(fTheta);
		matRotX.m[2][2] = cosf(fTheta);
		matRotX.m[3][3] = 1.0f;


		//Draw Triangles
		for (auto tri : meshCube.tris)
		{
			triangle triProjected, triTranslated, triRotatedZ, triRotatedX;

			//Rotation Z 
			MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
			MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
			MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

			//Rotation X
			MultiplyMatrixVector(triRotatedZ.p[0], triRotatedX.p[0], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[1], triRotatedX.p[1], matRotX);
			MultiplyMatrixVector(triRotatedZ.p[2], triRotatedX.p[2], matRotX);


			triTranslated = triRotatedX;
			triTranslated.p[0].z = triTranslated.p[0].z + 3.0f;
			triTranslated.p[1].z = triTranslated.p[1].z + 3.0f;
			triTranslated.p[2].z = triTranslated.p[2].z + 3.0f;


			// Normal Identification

			vec3d normal, line1, line2;

			line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
			line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
			line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

			line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
			line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
			line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

			normal.x = line1.z * line2.y - line2.z * line1.y;
			normal.y = line1.x * line2.z - line2.x * line1.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;

			//Normalising the Normal

			float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			normal.x /= l;
			normal.y /= l;
			normal.y /= l;

			if (normal.z < 0)
			{
				// Projecting Triangles
				MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
				MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
				MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

				//Scale into view
				triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
				triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
				triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;

				triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[1].x *= 0.5f * (float)ScreenWidth(); 
				triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

				//Draw Triangle
				DrawTriangle((int)triProjected.p[0].x, (int)triProjected.p[0].y, (int)triProjected.p[1].x,
					(int)triProjected.p[1].y, (int)triProjected.p[2].x, (int)triProjected.p[2].y, PIXEL_SOLID, FG_WHITE);
			}
		}

		return true;
	}
};



int main()
{
	GameEngine3D demo;

	if (demo.ConstructConsole(120, 80, (float)-0.1, (float)-0.1)) 
	{
		demo.Start();
	}

	return 0;
}

