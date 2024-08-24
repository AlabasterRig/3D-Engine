#include "olcConsoleGameEngine.h"
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;

struct vec3d
{
	float x = 0, y = 0, z = 0, w = 1;
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
	vec3d vLookDir;
	float fYaw;

	vec3d MultiplyMatrixVector(mat4x4& m, vec3d& i) // i = input, m = matrix
	{
		vec3d v;
		v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];

		return v;
	}

	mat4x4 MatrixMakeIdentity()
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
		matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = 1.0f;
		matrix.m[3][3] = 0.0f;

		return matrix;
	}

	mat4x4 MatrixMultiplyMatrix(mat4x4& m1, mat4x4& m2)
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
		return matrix;
	}

	mat4x4 MatrixPointAt(vec3d& pos, vec3d& target, vec3d& up)  // Points to the object (Formula To Code)
	{
		//Create Forward Vector for Forward Direction
		vec3d newForward = VectorSub(target, pos);
		newForward = VectorNormalise(newForward);

		// Calculate new Up Vector Direction
		vec3d temp = VectorMul(newForward, VectorDotP(up, newForward));
		vec3d newUp = VectorSub(up, temp);
		newUp = VectorNormalise(newUp);

		//Right Direction
		vec3d newRight = VectorCrossProduct(newUp, newForward);

		mat4x4 matrix;

		matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;

		return matrix;
	}

	mat4x4 MatrixQuickInverse(mat4x4 &m)
	{
		mat4x4 matrix;

		matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;

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
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
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

	//Algorithm to compute how lines intersect with planes (Taken formula from internet and converted to code directly)
	vec3d VectorIntersectPlane(vec3d& plane_p, vec3d& plane_n, vec3d& lineStart, vec3d& lineEnd)
	{
		plane_n = VectorNormalise(plane_n);
		float plane_d = -VectorDotP(plane_n, plane_p);
		float ad = VectorDotP(lineStart, plane_n);
		float bd = VectorDotP(lineEnd, plane_n);
		float t = (-plane_d - ad) / (bd - ad);
		vec3d lineStartToEnd = VectorSub(lineEnd, lineStart);
		vec3d lineToIntersect = VectorMul(lineStartToEnd, t);

		return VectorAdd(lineStart, lineToIntersect);
	}

	int TriangleClipAgainstPlane(vec3d plane_p, vec3d plane_n, triangle& inTri, triangle& outTri1, triangle& outTri2)
	{
		plane_n = VectorNormalise(plane_n);

		auto dist = [&](vec3d& p)
			{
				vec3d n = VectorNormalise(p);
				return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - VectorDotP(plane_n, plane_p));
			};

		//Temporary Storage for - if distance is positive it will store in 'insidePoints' vector and vice versa
		vec3d* insidePoints[3];
		vec3d* outsidePoints[3];
		int insidePointsCount = 0;
		int outsidePointsCount = 0;

		//Get signed Distance of each point in the plane

		float d0 = dist(inTri.p[0]);
		float d1 = dist(inTri.p[1]);
		float d2 = dist(inTri.p[2]);

		//Point 1
		if (d0 >= 0)
		{
			insidePoints[insidePointsCount++] = &inTri.p[0];
		}
		else {
			outsidePoints[outsidePointsCount++] = &inTri.p[0];
		}

		//Point 2
		if (d1 >= 0)
		{
			insidePoints[insidePointsCount++] = &inTri.p[1];
		}
		else {
			outsidePoints[outsidePointsCount++] = &inTri.p[1];
		}

		//Point 3
		if (d2 >= 0)
		{
			insidePoints[insidePointsCount++] = &inTri.p[2];
		}
		else {
			outsidePoints[outsidePointsCount++] = &inTri.p[2];
		}

		//Classify and break triangle into smaller triangles. There are Four Outcomes
		
		if (insidePointsCount == 0)
		{
			return 0; // All points of triangle lie outside plane so clip whole triangle
		}

		if (insidePointsCount == 3)
		{
			outTri1 = inTri; // All points lie inside plane, so do nothing

			return 1;
		}

		if (insidePointsCount == 1 && outsidePointsCount == 2)
		{
			//Appearance info of triangle
			outTri1.col = inTri.col;
			outTri1.sym = inTri.sym;

			// 1 valid inside point so use it
			outTri1.p[0] = *insidePoints[0];

			//But other 2 point lines intersect with the plane so use the intersect function
			outTri1.p[1] = VectorIntersectPlane(plane_p, plane_n, *insidePoints[0], *outsidePoints[0]);
			outTri1.p[2] = VectorIntersectPlane(plane_p, plane_n, *insidePoints[0], *outsidePoints[1]);

			return 1;
		}

		if (insidePointsCount == 2 && outsidePointsCount == 1)    //A quadrilateral is cut into two triangles
		{
			outTri1.col = inTri.col;
			outTri1.sym = inTri.sym;

			outTri2.col = inTri.col;
			outTri2.sym = inTri.sym;

			outTri1.p[0] = *insidePoints[0];
			outTri1.p[1] = *insidePoints[1];
			outTri1.p[2] = VectorIntersectPlane(plane_p, plane_n, *insidePoints[0], *outsidePoints[0]);

			outTri2.p[0] = *insidePoints[1];
			outTri2.p[1] = outTri1.p[2];
			outTri2.p[2] = VectorIntersectPlane(plane_p, plane_n, *insidePoints[1], *outsidePoints[0]);

			return 2;
		}
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
		meshCube.LoadFromObjectFile("ak-107.obj");

		//Projection Matrix
		matProj = MatrixMakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);

		return true;
	}


	bool OnUserUpdate(float fElapsedTime) override
	{
		if(GetKey(VK_UP).bHeld)
			vCamera.y += 8.0f * fElapsedTime;	// Travel Upwards

		if (GetKey(VK_DOWN).bHeld)
			vCamera.y -= 8.0f * fElapsedTime;	// Travel Downwards


		// Dont use these two in FPS mode, it is confusing :P
		if (GetKey(VK_LEFT).bHeld)
			vCamera.x -= 8.0f * fElapsedTime;	// Travel Along X-Axis

		if (GetKey(VK_RIGHT).bHeld)
			vCamera.x += 8.0f * fElapsedTime;   // Travel along X-Axis

		vec3d vForward = VectorMul(vLookDir, 8.0f * fElapsedTime);

		if (GetKey(L'W').bHeld)
		{
			vCamera = VectorAdd(vCamera, vForward);
		}
		if (GetKey(L'S').bHeld)
		{
			vCamera = VectorSub(vCamera, vForward);
		}
		if (GetKey(L'A').bHeld)
		{
			fYaw -= 2.0f * fElapsedTime;
		}
		if (GetKey(L'D').bHeld)
		{
			fYaw += 2.0f * fElapsedTime;
		}


		// Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		//Rotation Matrix Var
		mat4x4 matRotZ, matRotX;
		// fTheta += 1.0f * fElapsedTime;

		//Rotation Z
		matRotZ = MatrixMakeRotationZ(fTheta * 0.5f);
		//Rotation X
		matRotX = MatrixMakeRotationX(fTheta);

		mat4x4 matTrans;
		matTrans = MatrixMakeTranslation(0.0f, 0.0f, 5.0f);  //Controls Distance to Camera on z axis

		//World Matrix for Translation
		mat4x4 matWorld;
		matWorld = MatrixMakeIdentity();
		matWorld = MatrixMultiplyMatrix(matRotZ, matRotX);
		matWorld = MatrixMultiplyMatrix(matWorld, matTrans);

		//Camera Set-Up
		vec3d vUp = { 0,1,0 };
		vec3d vTarget = { 0,0,1 };

		//Camera Rotation Matrix
		mat4x4 matMakeRotate = MatrixMakeRotationY(fYaw);
		vLookDir = MultiplyMatrixVector(matMakeRotate, vTarget);
		vTarget = VectorAdd(vCamera, vLookDir);

		//Camera Matrix
		mat4x4 matCamera = MatrixPointAt(vCamera, vTarget, vUp);
		//View Matrix is inverse of camera Matrix
		mat4x4 viewMatrix = MatrixQuickInverse(matCamera);

		vector<triangle> vecTrianglesToRaster;

		//Draw Triangles
		for (auto tri : meshCube.tris)
		{
			triangle triProjected, triTransformed, triViewed;

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

			vec3d vCameraRay = VectorSub(triTransformed.p[0], vCamera);


			if (VectorDotP(normal, vCameraRay) < 0.0f)
			{

				//Illuminate Triangle
				vec3d LightDirection = { 0.0f, 1.0f, -1.0f };

				LightDirection = VectorNormalise(LightDirection);

				//Alignment oflight direction and triangle surface model
				float DotProduct = max(0.1f, VectorDotP(LightDirection, normal));

				CHAR_INFO c = GetColour(DotProduct);
				triTransformed.sym = c.Char.UnicodeChar;
				triTransformed.col = c.Attributes;

				//Camera View Space
				triViewed.p[0] = MultiplyMatrixVector(viewMatrix, triTransformed.p[0]);
				triViewed.p[1] = MultiplyMatrixVector(viewMatrix, triTransformed.p[1]);
				triViewed.p[2] = MultiplyMatrixVector(viewMatrix, triTransformed.p[2]);
				triViewed.col = triTransformed.col;
				triViewed.sym = triTransformed.sym;

				int clippedTriangles = 0;
				triangle clipped[2];
				clippedTriangles = TriangleClipAgainstPlane({ 0.0f,0.0f,0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

				for (int n = 0; n < clippedTriangles; n++)
				{

					// Projecting Triangles 3D to 2D
					triProjected.p[0] = MultiplyMatrixVector(matProj, clipped[n].p[0]);
					triProjected.p[1] = MultiplyMatrixVector(matProj, clipped[n].p[1]);
					triProjected.p[2] = MultiplyMatrixVector(matProj, clipped[n].p[2]);
					triProjected.col = clipped[n].col;
					triProjected.sym = clipped[n].sym;

					//Normalising 
					triProjected.p[0] = VectorDiv(triProjected.p[0], triProjected.p[0].w);
					triProjected.p[1] = VectorDiv(triProjected.p[1], triProjected.p[1].w);
					triProjected.p[2] = VectorDiv(triProjected.p[2], triProjected.p[2].w);

					// X/Y are inverted so put them back
					triProjected.p[0].x *= -1.0f;
					triProjected.p[1].x *= -1.0f;
					triProjected.p[2].x *= -1.0f;
					triProjected.p[0].y *= -1.0f;
					triProjected.p[1].y *= -1.0f;
					triProjected.p[2].y *= -1.0f;

					//Offsets Verts into visible normalised space
					vec3d vOffsetView = { 1,1,0 };
					triProjected.p[0] = VectorAdd(triProjected.p[0], vOffsetView);
					triProjected.p[1] = VectorAdd(triProjected.p[1], vOffsetView);
					triProjected.p[2] = VectorAdd(triProjected.p[2], vOffsetView);

					triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
					triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
					triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

					vecTrianglesToRaster.push_back(triProjected);
				}
			}
		}

		//Sort Triangles from back to front
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
			{
				float midPoint1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
				float midPoint2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
				return midPoint1 > midPoint2;
			});

		Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

		for (auto& triToRaster : vecTrianglesToRaster)
		{
			// Clip Triangles against all 4 screen edges

			triangle clipped[2];
			list<triangle> listTriangle;
			listTriangle.push_back(triToRaster);
			int newTriangles = 1;

			for (int p = 0; p < 4; p++)
			{
				int trisToAdd = 0;
				while (newTriangles > 0)
				{
					triangle test = listTriangle.front();
					listTriangle.pop_front();
					newTriangles--;

					switch (p)
					{
					case 0:	
						trisToAdd = TriangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); 
						break;

					case 1:	
						trisToAdd = TriangleClipAgainstPlane({ 0.0f, (float)ScreenHeight() - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]);
						break;

					case 2:	
						trisToAdd = TriangleClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); 
						break;

					case 3:	
						trisToAdd = TriangleClipAgainstPlane({ (float)ScreenWidth() - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]);
						break;
					}

					for (int x = 0; x < trisToAdd; x++)
					{
						listTriangle.push_back(clipped[x]);
					}
				}
				newTriangles = listTriangle.size();
			}

			for (auto& t : listTriangle)
			{
				// Fragment Shader
				FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, t.sym, t.col);

				// WireFrame
				//DrawTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, PIXEL_SOLID, FG_BLACK);
			}
		}

		return true;
	}
};


int main()
{
	GameEngine3D demo;

	if (demo.ConstructConsole(256, 240, 3, 3))
	{
		demo.Start();
	}

	return 0;
}