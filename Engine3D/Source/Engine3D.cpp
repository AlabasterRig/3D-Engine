﻿#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;

struct vec3d
{
	float x, y, z, w;
};


struct triangle
{
	vec3d p[3];

	wchar_t sym;
	short col;
};


struct mesh
{
	vector<triangle> tris;

	bool LoadFromObjectFile(string sFilename)
	{
		ifstream f(sFilename);
		if (!f.is_open())
			return false;

		// Local cache of verts
		vector<vec3d> verts;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			strstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}

		return true;
	}

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
	vec3d vCamera = { 0.0f, 0.0f, 0.0f };

	vec3d MultiplyMatrixVector(mat4x4& m, vec3d& i) // i = input, m = matrix
	{
		vec3d v;
		v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
		
		return v;
	}

	mat4x4 MatriMakeIdentity()
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 MatrixMakeRotationX(float fTheta)
	{
		mat4x4 matRotX;
		matRotX.m[0][0] = 1.0f;
		matRotX.m[1][1] = cosf(fTheta);
		matRotX.m[1][2] = sinf(fTheta);
		matRotX.m[2][1] = -sinf(fTheta);
		matRotX.m[2][2] = cosf(fTheta);
		matRotX.m[3][3] = 1.0f;
		return matRotX;
	}

	mat4x4 MatrixMakeRotationY(float fTheta)
	{
		mat4x4 matRotY;
		matRotY.m[0][0] = cosf(fTheta);
		matRotY.m[0][2] = sinf(fTheta);
		matRotY.m[2][0] = -sinf(fTheta);
		matRotY.m[1][1] = 1.0f;
		matRotY.m[2][2] = cosf(fTheta);
		matRotY.m[3][3] = 1.0f;
		return matRotY;
	}

	mat4x4 MatrixMakeRotationZ(float fTheta)
	{
		mat4x4 matRotZ;
		matRotZ.m[0][0] = cosf(fTheta);
		matRotZ.m[0][1] = sinf(fTheta);
		matRotZ.m[1][0] = -sinf(fTheta);
		matRotZ.m[1][1] = cosf(fTheta);
		matRotZ.m[2][2] = 1.0f;
		matRotZ.m[3][3] = 1.0f;
		return matRotZ;
	}

	mat4x4 MatrixMakeTranslation(float x, float y, float z)
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}

	mat4x4 MatrixMakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
	{
		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix;
		matrix.m[0][0] = fAspectRatio * fFovRad;
		matrix.m[1][1] = fFovRad;
		matrix.m[2][2] = fFar / (fFar - fNear);
		matrix.m[3][3] = (-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}

	mat4x4 MatrixMultiplyMatrix(mat4x4& m1, mat4x4& m2)
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++)
		{
			for (int r = 0; r < 4; r++)
			{
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
			}
		}
		return matrix;
	}

	vec3d VectorAdd(vec3d& v1, vec3d& v2)
	{
		return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	}

	vec3d VectorSub(vec3d& v1, vec3d& v2)
	{
		return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	}

	vec3d VectorMul(vec3d& v1, float k)
	{
		return { v1.x * k, v1.y * k, v1.z * k };
	}

	vec3d VectorDiv(vec3d& v1, float k)
	{
		return { v1.x / k, v1.y / k, v1.z / k };
	}

	float VectorDotP(vec3d& v1, vec3d& v2)
	{
		return v1.x * v2.x, v1.y * v2.y, v1.z * v1.z;
	}

	float VectorLength(vec3d& v)
	{
		return sqrtf(VectorDotP(v, v));
	}

	vec3d VectorNormalise(vec3d& v1)
	{
		float l = VectorLength(v1);
		return { v1.x / l, v1.y / l, v1.z / l };
	}

	vec3d VectorCrossProduct(vec3d& v1, vec3d& v2)
	{
		vec3d v;
		v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
		return v;
	}

	CHAR_INFO GetColour(float x)
	{
		short bg_col, fg_col;
		wchar_t sym;
		int pixel_bw = (int)(13.0f * x);

		switch (pixel_bw)
		{

		case 0: bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
			break;

		case 1: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_QUARTER; break;
		case 2: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_HALF; break;
		case 3: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 4: bg_col = BG_BLACK; fg_col = FG_DARK_GREY; sym = PIXEL_SOLID; break;

		case 5: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_QUARTER; break;
		case 6: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_HALF; break;
		case 7: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_THREEQUARTERS; break;
		case 8: bg_col = BG_DARK_GREY; fg_col = FG_GREY; sym = PIXEL_SOLID; break;

		case 9: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_QUARTER; break;
		case 10: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_HALF; break;
		case 11: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_THREEQUARTERS; break;
		case 12: bg_col = BG_GREY; fg_col = FG_WHITE; sym = PIXEL_SOLID; break;

		default:
			bg_col = BG_BLACK; fg_col = FG_BLACK; sym = PIXEL_SOLID;
		}

		CHAR_INFO c;
		c.Attributes = bg_col | fg_col;
		c.Char.UnicodeChar = sym;
		return c;
	}


public:

	bool OnUserCreate() override
	{
		//meshCube.tris = {

		//	// SOUTH
		//	{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
		//	{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

		//	// EAST                                                      
		//	{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
		//	{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

		//	// NORTH                                                     
		//	{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

		//	// WEST                                                      
		//	{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
		//	{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

		//	// TOP                                                       
		//	{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
		//	{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

		//	// BOTTOM                                                    
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
		//	{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },

		//}; 

		meshCube.LoadFromObjectFile("VideoShip.obj");

		//Projection Matrix
		MatrixMakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

		return true;
	}


	bool OnUserUpdate(float fElapsedTime) override
	{

		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		//Rotation Matrix Var
		mat4x4 matRotZ, matRotX;
		fTheta += 1.0f * fElapsedTime;
		//Rotation Z
		matRotZ = MatrixMakeRotationZ(fTheta * 0.5f);
		//Rotation X
		matRotX = MatrixMakeRotationX(fTheta);

		mat4x4 matTrans;
		matTrans = MatrixMakeTranslation(0.0f, 0.0f, 16.0f);

		//World Matrix for Translation
		mat4x4 matWorld;
		matWorld = MatriMakeIdentity();
		matWorld = MatrixMultiplyMatrix(matRotZ, matRotX);
		matWorld = MatrixMultiplyMatrix(matWorld, matTrans);

		vector<triangle> vecTrianglesToRaster;

		//Draw Triangles
		for (auto tri : meshCube.tris)
		{
			triangle triProjected, triTransformed;

			triTransformed.p[0] = MultiplyMatrixVector(matWorld, tri.p[0]);
			triTransformed.p[1] = MultiplyMatrixVector(matWorld, tri.p[1]);
			triTransformed.p[2] = MultiplyMatrixVector(matWorld, tri.p[2]);


			// Normal Identification
			vec3d normal, line1, line2;

			line1 = VectorSub(triTransformed.p[1], triTransformed.p[0]);
			line2 = VectorSub(triTransformed.p[2], triTransformed.p[0]);

			//Cross Product normal to triangle surface
			normal = VectorCrossProduct(line1, line2);

			//Normalising the Normal
			normal = VectorNormalise(normal);


			float l = sqrtf((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));
			normal.x /= l;
			normal.y /= l;
			normal.z /= l;


			if (normal.x * (triTranslated.p[0].x - vCamera.x) +
				normal.y * (triTranslated.p[0].y - vCamera.y) +
				normal.z * (triTranslated.p[0].z - vCamera.z) < 0.0f)
			{

				//Illuminate Triangle
				vec3d LightPosition = { 0.0f, 0.0f, -1.0f };

				float i = sqrtf(LightPosition.x * LightPosition.x + LightPosition.y * LightPosition.y + LightPosition.z * LightPosition.z);
				LightPosition.x /= i;
				LightPosition.y /= i;
				LightPosition.z /= i;

				float DotProduct = normal.x * LightPosition.x + normal.y * LightPosition.y + normal.z * LightPosition.z;

				CHAR_INFO c = GetColour(DotProduct);
				triTranslated.sym = c.Char.UnicodeChar;
				triTranslated.col = c.Attributes;

				// Projecting Triangles
				MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
				MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
				MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);
				triProjected.col = triTranslated.col;
				triProjected.sym = triTranslated.sym;

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

				vecTrianglesToRaster.push_back(triProjected);
			}
		}

		//Sort Triangles from back to front
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
				float midPoint1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float midPoint2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return midPoint1 > midPoint2;
			});

		for (auto& triProjected : vecTrianglesToRaster)
		{
			//Draw Triangle
			FillTriangle((int)triProjected.p[0].x, (int)triProjected.p[0].y, (int)triProjected.p[1].x,
				(int)triProjected.p[1].y, (int)triProjected.p[2].x, (int)triProjected.p[2].y, triProjected.sym, triProjected.col);

			//WireFrame
			 /* DrawTriangle((int)triProjected.p[0].x, (int)triProjected.p[0].y, (int)triProjected.p[1].x,
				(int)triProjected.p[1].y, (int)triProjected.p[2].x, (int)triProjected.p[2].y, PIXEL_SOLID, FG_WHITE); */
		}

		return true;
	}
};


int main()
{
	GameEngine3D demo;

	if (demo.ConstructConsole(120, 90, 4, 4))
	{
		demo.Start();
	}

	return 0;
}