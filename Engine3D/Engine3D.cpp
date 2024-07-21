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


class GameEngine3D : public olcConsoleGameEngine
{
public:
    GameEngine3D()
    {
        m_sAppName = L"3D Demo";
    }

private:
    mesh meshCube;

public:
    bool OnUserCreate() override
    {
        meshCube.tris = {
            //South
            {0.0f,0.0f,0.0f,  0.0f,1.0f,0.0f,  1.0f,1.0f,0.0f},
            {0.0f,0.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,0.0f,0.0f},

            //North
            {0.0f,0.0f,1.0f,  0.0f,1.0f,1.0f,  1.0f,1.0f,1.0f},
            {0.0f,0.0f,1.0f,  1.0f,1.0f,1.0f,  1.0f,0.0f,1.0f},

            //East
            {1.0f,0.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,1.0f,1.0f},
            {1.0f,0.0f,0.0f,  1.0f,1.0f,1.0f,  1.0f,0.0f,1.0f},

            //West
            {0.0f,0.0f,1.0f,  0.0f,1.0f,1.0f,  0.0f,1.0f,0.0f},
            {0.0f,0.0f,1.0f,  0.0f,1.0f,0.0f,  0.0f,0.0f,0.0f},

            //TOP
            {0.0f,1.0f,0.0f,  0.0f,1.0f,1.0f,  1.0f,1.0f,1.0f},
            {0.0f,1.0f,0.0f,  1.0f,1.0f,1.0f,  1.0f,1.0f,0.0f},

            //Bottom
            {0.0f,0.0f,0.0f,  0.0f,0.0f,1.0f,  1.0f,0.0f,1.0f},
            {0.0f,0.0f,0.0f,  1.0f,0.0f,1.0f,  1.0f,0.0f,0.0f}
        };

        return true;
    }

    bool OnUserUpdate(float fElapsedTime)
    {
        Fill(0, 0, ScreenWidth(), ScreenHeight(), PIXEL_SOLID, FG_BLACK);

        //Draw Triangles
        for (auto tri : meshCube.tris)
        {

        }

        return true;
    }
};



int main()
{
    GameEngine3D demo;

    if (demo.ConstructConsole(56, 240, 4, 4))
        demo.Start();

    return 0;
}

