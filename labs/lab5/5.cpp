
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

struct Vec3
{
    float x, y, z;
};

struct Face
{
    std::vector<int> vertices;
};

std::vector<Vec3> vertices;
std::vector<Face> faces;

HDC hDC;
HGLRC hRC;
float angle = 0.0f;

Vec3 subtract(Vec3 a, Vec3 b)
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 cross(Vec3 a, Vec3 b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 normalize(Vec3 v)
{
    float length = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    if (length == 0.0f)
        return {0, 0, 0};

    return {v.x / length, v.y / length, v.z / length};
}

bool loadOBJ(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file.is_open())
        return false;

    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream stream(line);
        std::string type;
        stream >> type;

        if (type == "v")
        {
            Vec3 vertex;
            stream >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (type == "f")
        {
            Face face;
            std::string value;

            while (stream >> value)
            {
                size_t separator = value.find('/');
                std::string index = value.substr(0, separator);

                int vertexIndex = std::stoi(index);

                if (vertexIndex < 0)
                    vertexIndex = static_cast<int>(vertices.size()) + vertexIndex + 1;

                face.vertices.push_back(vertexIndex - 1);
            }

            if (face.vertices.size() >= 3)
                faces.push_back(face);
        }
    }

    return !vertices.empty() && !faces.empty();
}

void resize(int width, int height)
{
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

void setupLighting()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat diffuse[] = {0.9f, 0.9f, 0.9f, 1.0f};
    GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat position[] = {3.0f, 4.0f, 5.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    GLfloat materialAmbient[] = {0.2f, 0.2f, 0.5f, 1.0f};
    GLfloat materialDiffuse[] = {0.3f, 0.5f, 0.9f, 1.0f};
    GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat shininess[] = {64.0f};

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glShadeModel(GL_SMOOTH);
}

void drawModel()
{
    glBegin(GL_TRIANGLES);

    for (const Face& face : faces)
    {
        Vec3 a = vertices[face.vertices[0]];
        Vec3 b = vertices[face.vertices[1]];
        Vec3 c = vertices[face.vertices[2]];

        Vec3 normal = normalize(cross(subtract(b, a), subtract(c, a)));

        for (size_t i = 1; i + 1 < face.vertices.size(); i++)
        {
            Vec3 points[3] = {
                vertices[face.vertices[0]],
                vertices[face.vertices[i]],
                vertices[face.vertices[i + 1]]
            };

            glNormal3f(normal.x, normal.y, normal.z);

            for (const Vec3& point : points)
                glVertex3f(point.x, point.y, point.z);
        }
    }

    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glRotatef(angle, 0.0f, 1.0f, 0.0f);
    glRotatef(-15.0f, 1.0f, 0.0f, 0.0f);

    drawModel();

    SwapBuffers(hDC);

    angle += 0.5f;

    if (angle >= 360.0f)
        angle -= 360.0f;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
        resize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;

    case WM_DESTROY:
        return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    if (!loadOBJ("assets/model.obj"))
    {
        MessageBoxA(nullptr, "Cannot load assets/model.obj", "Error", MB_ICONERROR);
        return 1;
    }

    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "OpenGLLightingWindow";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        "OpenGLLightingWindow",
        "Lab 5 - Phong Lighting",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        900, 700,
        nullptr, nullptr, hInstance, nullptr
    );

    hDC = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int format = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, format, &pfd);

    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);

    setupLighting();

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};

    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                goto cleanup;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        display();
        Sleep(16);
    }

cleanup:
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);

    return 0;
}