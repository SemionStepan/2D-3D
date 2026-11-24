
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

const int MAX_PARTICLES = 700;

struct Particle
{
    float x, y, z;
    float vx, vy, vz;
    float life;
    float maxLife;
    float size;
    float r, g, b;
};

std::vector<Particle> particles;

HDC hDC;
HGLRC hRC;

float cameraAngle = 0.0f;

float randomFloat(float min, float max)
{
    return min + static_cast<float>(rand()) / RAND_MAX * (max - min);
}

void createParticle()
{
    Particle p;

    p.x = randomFloat(-0.15f, 0.15f);
    p.y = 0.0f;
    p.z = randomFloat(-0.15f, 0.15f);

    p.vx = randomFloat(-0.5f, 0.5f);
    p.vy = randomFloat(1.5f, 3.5f);
    p.vz = randomFloat(-0.5f, 0.5f);

    p.maxLife = randomFloat(1.0f, 2.0f);
    p.life = p.maxLife;

    p.size = randomFloat(0.12f, 0.25f);

    p.r = 1.0f;
    p.g = randomFloat(0.2f, 0.8f);
    p.b = randomFloat(0.0f, 0.1f);

    particles.push_back(p);
}

void updateParticles(float deltaTime)
{
    for (auto& p : particles)
    {
        p.life -= deltaTime;

        p.x += p.vx * deltaTime;
        p.y += p.vy * deltaTime;
        p.z += p.vz * deltaTime;

        p.vx += randomFloat(-0.5f, 0.5f) * deltaTime;
        p.vz += randomFloat(-0.5f, 0.5f) * deltaTime;

        p.vy -= 0.5f * deltaTime;

        float age = 1.0f - p.life / p.maxLife;

        p.r = 1.0f;
        p.g = 0.9f - age * 0.8f;
        p.b = 0.1f + age * 0.15f;
    }

    particles.erase(
        std::remove_if(
            particles.begin(),
            particles.end(),
            [](const Particle& p)
            {
                return p.life <= 0.0f;
            }
        ),
        particles.end()
    );

    while (particles.size() < MAX_PARTICLES)
        createParticle();
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

void drawGround()
{
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    glColor3f(0.12f, 0.12f, 0.14f);

    glBegin(GL_QUADS);

    glVertex3f(-5.0f, -0.05f, -5.0f);
    glVertex3f(5.0f, -0.05f, -5.0f);
    glVertex3f(5.0f, -0.05f, 5.0f);
    glVertex3f(-5.0f, -0.05f, 5.0f);

    glEnd();
}

void drawFireBase()
{
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    glColor3f(0.25f, 0.08f, 0.02f);

    glPushMatrix();
    glTranslatef(0.0f, 0.15f, 0.0f);
    glScalef(0.5f, 0.3f, 0.5f);

    GLUquadric* quadric = gluNewQuadric();
    gluSphere(quadric, 1.0, 20, 20);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}

void drawParticle(const Particle& p)
{
    float alpha = p.life / p.maxLife;

    float age = 1.0f - alpha;
    float size = p.size * (1.0f - age * 0.7f);

    glColor4f(p.r, p.g, p.b, alpha * 0.8f);

    glPushMatrix();
    glTranslatef(p.x, p.y, p.z);

    GLUquadric* quadric = gluNewQuadric();
    gluSphere(quadric, size, 8, 8);
    gluDeleteQuadric(quadric);

    glPopMatrix();
}

void drawParticles()
{
    std::sort(
        particles.begin(),
        particles.end(),
        [](const Particle& a, const Particle& b)
        {
            return a.z < b.z;
        }
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glDepthMask(GL_FALSE);

    for (const auto& p : particles)
        drawParticle(p);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        0.0, 2.0, 7.0,
        0.0, 1.0, 0.0,
        0.0, 1.0, 0.0
    );

    glRotatef(cameraAngle, 0.0f, 1.0f, 0.0f);

    drawGround();
    drawFireBase();
    drawParticles();

    SwapBuffers(hDC);
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
    srand(static_cast<unsigned int>(time(nullptr)));

    WNDCLASS wc = {};
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "ParticleFireWindow";

    RegisterClass(&wc);

    HWND hwnd = CreateWindow(
        "ParticleFireWindow",
        "Lab 7 - Particle System",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        1000, 750,
        nullptr, nullptr, hInstance, nullptr
    );

    hDC = GetDC(hwnd);

    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int format = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, format, &pfd);

    hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.015f, 0.015f, 0.025f, 1.0f);

    for (int i = 0; i < MAX_PARTICLES; i++)
        createParticle();

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};

    DWORD previousTime = GetTickCount();

    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                goto cleanup;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        DWORD currentTime = GetTickCount();

        float deltaTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        if (deltaTime > 0.05f)
            deltaTime = 0.05f;

        updateParticles(deltaTime);

        display();

        Sleep(16);
    }

cleanup:
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);

    return 0;
}
