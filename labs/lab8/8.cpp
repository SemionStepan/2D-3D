#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <vector>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <algorithm>

const int MAP_SIZE = 129;
const float TERRAIN_SIZE = 12.0f;
const float MAX_HEIGHT = 3.5f;

struct Vec3 {
    float x, y, z;

    Vec3 operator+(const Vec3& v) const {
        return {x + v.x, y + v.y, z + v.z};
    }

    Vec3 operator-(const Vec3& v) const {
        return {x - v.x, y - v.y, z - v.z};
    }

    Vec3 operator/(float value) const {
        return {x / value, y / value, z / value};
    }

    Vec3& operator+=(const Vec3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
};

Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vec3 normalize(const Vec3& v) {
    float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);

    if (length < 0.00001f)
        return {0.0f, 1.0f, 0.0f};

    return v / length;
}

struct Vertex {
    Vec3 position;
    Vec3 normal;
    float height;
};

std::vector<unsigned char> heightmap;
std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

float rotationAngle = 0.0f;
HWND windowHandle = nullptr;
HDC deviceContext = nullptr;
HGLRC renderContext = nullptr;

void createHeightmap() {
    CreateDirectoryA("assets", nullptr);

    std::ifstream test("assets/heightmap.bmp", std::ios::binary);

    if (test.good()) {
        test.close();
        return;
    }

    const int width = MAP_SIZE;
    const int height = MAP_SIZE;
    const int rowSize = (width * 3 + 3) & ~3;
    const int imageSize = rowSize * height;

    std::ofstream file("assets/heightmap.bmp", std::ios::binary);

    if (!file)
        return;

    uint32_t fileSize = 54 + imageSize;
    uint32_t dataOffset = 54;
    uint32_t headerSize = 40;
    int32_t planes = 1;
    int32_t bitsPerPixel = 24;
    uint32_t compression = 0;

    file.put('B');
    file.put('M');
    file.write(reinterpret_cast<char*>(&fileSize), 4);
    uint32_t reserved = 0;
    file.write(reinterpret_cast<char*>(&reserved), 4);
    file.write(reinterpret_cast<char*>(&dataOffset), 4);
    file.write(reinterpret_cast<char*>(&headerSize), 4);
    file.write(reinterpret_cast<const char*>(&width), 4);
    file.write(reinterpret_cast<const char*>(&height), 4);
    file.write(reinterpret_cast<char*>(&planes), 2);
    file.write(reinterpret_cast<char*>(&bitsPerPixel), 2);
    file.write(reinterpret_cast<char*>(&compression), 4);
    file.write(reinterpret_cast<const char*>(&imageSize), 4);

    int32_t resolution = 2835;
    uint32_t colors = 0;

    file.write(reinterpret_cast<char*>(&resolution), 4);
    file.write(reinterpret_cast<char*>(&resolution), 4);
    file.write(reinterpret_cast<char*>(&colors), 4);
    file.write(reinterpret_cast<char*>(&colors), 4);

    std::vector<unsigned char> row(rowSize, 0);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float nx = (x - width / 2.0f) / (width / 2.0f);
            float ny = (y - height / 2.0f) / (height / 2.0f);

            float hill1 = std::exp(-(nx * nx * 7.0f + ny * ny * 5.0f));
            float hill2 = 0.75f * std::exp(
                -((nx - 0.45f) * (nx - 0.45f) * 18.0f +
                  (ny + 0.25f) * (ny + 0.25f) * 12.0f)
            );

            float hill3 = 0.55f * std::exp(
                -((nx + 0.4f) * (nx + 0.4f) * 20.0f +
                  (ny - 0.3f) * (ny - 0.3f) * 16.0f)
            );

            float waves = 0.08f *
                (std::sin(nx * 15.0f) + std::cos(ny * 13.0f));

            float value = 0.12f + hill1 * 0.48f +
                          hill2 * 0.35f + hill3 * 0.30f + waves;

            value = std::clamp(value, 0.0f, 1.0f);

            unsigned char pixel =
                static_cast<unsigned char>(value * 255.0f);

            int index = x * 3;

            row[index] = pixel;
            row[index + 1] = pixel;
            row[index + 2] = pixel;
        }

        file.write(reinterpret_cast<char*>(row.data()), rowSize);
    }

    file.close();
}

bool loadHeightmap() {
    std::ifstream file("assets/heightmap.bmp", std::ios::binary);

    if (!file)
        return false;

    unsigned char header[54];

    file.read(reinterpret_cast<char*>(header), 54);

    if (!file || header[0] != 'B' || header[1] != 'M')
        return false;

    int width = *reinterpret_cast<int*>(&header[18]);
    int height = *reinterpret_cast<int*>(&header[22]);
    int bits = *reinterpret_cast<short*>(&header[28]);
    int compression = *reinterpret_cast<int*>(&header[30]);
    int offset = *reinterpret_cast<int*>(&header[10]);

    if (width <= 0 || height <= 0 || bits != 24 || compression != 0)
        return false;

    int rowSize = (width * 3 + 3) & ~3;

    std::vector<unsigned char> row(rowSize);
    heightmap.resize(width * height);

    file.seekg(offset, std::ios::beg);

    for (int y = 0; y < height; y++) {
        file.read(reinterpret_cast<char*>(row.data()), rowSize);

        if (!file)
            return false;

        for (int x = 0; x < width; x++) {
            int index = x * 3;

            unsigned char blue = row[index];
            unsigned char green = row[index + 1];
            unsigned char red = row[index + 2];

            unsigned char brightness =
                static_cast<unsigned char>((red + green + blue) / 3);

            heightmap[y * width + x] = brightness;
        }
    }

    vertices.resize(width * height);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float u = static_cast<float>(x) / (width - 1);
            float v = static_cast<float>(y) / (height - 1);

            float h = heightmap[y * width + x] / 255.0f;

            Vertex& vertex = vertices[y * width + x];

            vertex.position = {
                (u - 0.5f) * TERRAIN_SIZE,
                h * MAX_HEIGHT,
                (v - 0.5f) * TERRAIN_SIZE
            };

            vertex.normal = {0.0f, 1.0f, 0.0f};
            vertex.height = h;
        }
    }

    indices.clear();

    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            unsigned int a = y * width + x;
            unsigned int b = a + 1;
            unsigned int c = (y + 1) * width + x;
            unsigned int d = c + 1;

            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);

            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }

    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex& a = vertices[indices[i]];
        Vertex& b = vertices[indices[i + 1]];
        Vertex& c = vertices[indices[i + 2]];

        Vec3 edge1 = b.position - a.position;
        Vec3 edge2 = c.position - a.position;

        Vec3 normal = normalize(cross(edge1, edge2));

        a.normal += normal;
        b.normal += normal;
        c.normal += normal;
    }

    for (auto& vertex : vertices)
        vertex.normal = normalize(vertex.normal);

    return true;
}

void setTerrainColor(float height) {
    if (height < 0.25f) {
        glColor3f(0.12f, 0.35f, 0.10f);
    }
    else if (height < 0.50f) {
        glColor3f(0.25f, 0.55f, 0.16f);
    }
    else if (height < 0.70f) {
        glColor3f(0.48f, 0.40f, 0.22f);
    }
    else {
        glColor3f(0.78f, 0.78f, 0.72f);
    }
}

void drawTerrain() {
    glBegin(GL_TRIANGLES);

    for (unsigned int index : indices) {
        const Vertex& vertex = vertices[index];

        glNormal3f(
            vertex.normal.x,
            vertex.normal.y,
            vertex.normal.z
        );

        setTerrainColor(vertex.height);

        glVertex3f(
            vertex.position.x,
            vertex.position.y,
            vertex.position.z
        );
    }

    glEnd();
}

void initializeOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lightPosition[] = { -5.0f, 10.0f, 8.0f, 1.0f };
    GLfloat ambient[] = { 0.35f, 0.35f, 0.35f, 1.0f };
    GLfloat diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    glClearColor(0.48f, 0.68f, 0.88f, 1.0f);

    glShadeModel(GL_SMOOTH);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, -1.0f, -18.0f);
    glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

    drawTerrain();

    SwapBuffers(deviceContext);

    rotationAngle += 0.25f;

    if (rotationAngle >= 360.0f)
        rotationAngle -= 360.0f;
}

void resizeWindow(int width, int height) {
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        45.0,
        static_cast<double>(width) / height,
        0.1,
        100.0
    );

    glMatrixMode(GL_MODELVIEW);
}

LRESULT CALLBACK WindowProcedure(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) {
    switch (message) {
        case WM_SIZE:
            if (renderContext) {
                wglMakeCurrent(deviceContext, renderContext);

                resizeWindow(
                    LOWORD(lParam),
                    HIWORD(lParam)
                );
            }
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(
    HINSTANCE instance,
    HINSTANCE,
    LPSTR,
    int showCommand
) {
    createHeightmap();

    if (!loadHeightmap()) {
        MessageBoxA(
            nullptr,
            "Failed to load heightmap.bmp",
            "Error",
            MB_ICONERROR
        );

        return 1;
    }

    WNDCLASSA windowClass = {};
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "TerrainWindow";
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClassA(&windowClass);

    windowHandle = CreateWindowA(
        "TerrainWindow",
        "Lab 8 - 3D Terrain Generation",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1000,
        750,
        nullptr,
        nullptr,
        instance,
        nullptr
    );

    if (!windowHandle)
        return 1;

    deviceContext = GetDC(windowHandle);

    PIXELFORMATDESCRIPTOR pixelFormat = {};
    pixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixelFormat.nVersion = 1;
    pixelFormat.dwFlags =
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER;
    pixelFormat.iPixelType = PFD_TYPE_RGBA;
    pixelFormat.cColorBits = 24;
    pixelFormat.cDepthBits = 24;
    pixelFormat.iLayerType = PFD_MAIN_PLANE;

    int format = ChoosePixelFormat(deviceContext, &pixelFormat);
    SetPixelFormat(deviceContext, format, &pixelFormat);

    renderContext = wglCreateContext(deviceContext);
    wglMakeCurrent(deviceContext, renderContext);

    initializeOpenGL();
    resizeWindow(1000, 750);

    ShowWindow(windowHandle, showCommand);
    UpdateWindow(windowHandle);

    MSG message = {};

    while (message.message != WM_QUIT) {
        if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        else {
            renderScene();
            Sleep(16);
        }
    }

    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(renderContext);
    ReleaseDC(windowHandle, deviceContext);

    return 0;
}