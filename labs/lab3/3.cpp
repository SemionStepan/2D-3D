
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdint>
#include <filesystem>

const int WIDTH = 800;
const int HEIGHT = 600;

struct Color
{
    uint8_t r, g, b;
};

struct Point
{
    int x, y;
};

struct Triangle
{
    Point a, b, c;
    Color color;
};

struct Circle
{
    Point center;
    int radius;
    Color color;
};

std::vector<Color> colors = {
    {255, 80, 80},
    {80, 160, 255},
    {80, 200, 120},
    {220, 120, 255},
    {255, 180, 50},
    {50, 200, 200}
};

std::vector<uint8_t> image(WIDTH * HEIGHT * 3, 255);

void putPixel(int x, int y, Color color)
{
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        return;

    int index = (y * WIDTH + x) * 3;

    image[index] = color.r;
    image[index + 1] = color.g;
    image[index + 2] = color.b;
}

double edge(Point a, Point b, double x, double y)
{
    return (x - a.x) * (b.y - a.y)
         - (y - a.y) * (b.x - a.x);
}

void drawTriangle(const Triangle& t)
{
    int minX = std::max(0, std::min({t.a.x, t.b.x, t.c.x}));
    int maxX = std::min(WIDTH - 1, std::max({t.a.x, t.b.x, t.c.x}));
    int minY = std::max(0, std::min({t.a.y, t.b.y, t.c.y}));
    int maxY = std::min(HEIGHT - 1, std::max({t.a.y, t.b.y, t.c.y}));

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            double px = x + 0.5;
            double py = y + 0.5;

            double e1 = edge(t.a, t.b, px, py);
            double e2 = edge(t.b, t.c, px, py);
            double e3 = edge(t.c, t.a, px, py);

            bool hasNegative = e1 < 0 || e2 < 0 || e3 < 0;
            bool hasPositive = e1 > 0 || e2 > 0 || e3 > 0;

            if (!(hasNegative && hasPositive))
                putPixel(x, y, t.color);
        }
    }
}

void drawCircle(const Circle& c)
{
    int minX = std::max(0, c.center.x - c.radius);
    int maxX = std::min(WIDTH - 1, c.center.x + c.radius);
    int minY = std::max(0, c.center.y - c.radius);
    int maxY = std::min(HEIGHT - 1, c.center.y + c.radius);

    int radiusSquared = c.radius * c.radius;

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            int dx = x - c.center.x;
            int dy = y - c.center.y;

            if (dx * dx + dy * dy <= radiusSquared)
                putPixel(x, y, c.color);
        }
    }
}

void write16(std::ofstream& file, uint16_t value)
{
    file.put(value & 255);
    file.put((value >> 8) & 255);
}

void write32(std::ofstream& file, uint32_t value)
{
    for (int i = 0; i < 4; i++)
        file.put((value >> (i * 8)) & 255);
}

void saveBMP(const std::string& filename)
{
    std::ofstream file(filename, std::ios::binary);

    if (!file)
    {
        std::cout << "Cannot create output file\n";
        return;
    }

    int rowSize = (WIDTH * 3 + 3) & ~3;
    int imageSize = rowSize * HEIGHT;

    file.put('B');
    file.put('M');
    write32(file, 54 + imageSize);
    write32(file, 0);
    write32(file, 54);

    write32(file, 40);
    write32(file, WIDTH);
    write32(file, HEIGHT);
    write16(file, 1);
    write16(file, 24);
    write32(file, 0);
    write32(file, imageSize);
    write32(file, 2835);
    write32(file, 2835);
    write32(file, 0);
    write32(file, 0);

    int padding = rowSize - WIDTH * 3;

    for (int y = HEIGHT - 1; y >= 0; y--)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            int index = (y * WIDTH + x) * 3;

            file.put(image[index + 2]);
            file.put(image[index + 1]);
            file.put(image[index]);
        }

        for (int i = 0; i < padding; i++)
            file.put(0);
    }

    file.close();
}

int main()
{
    int triangleCount;
    int circleCount;

    std::cout << "Number of triangles: ";
    std::cin >> triangleCount;

    if (!std::cin || triangleCount < 0 || triangleCount > 100)
    {
        std::cout << "Invalid number of triangles\n";
        return 1;
    }

    std::vector<Triangle> triangles;

    for (int i = 0; i < triangleCount; i++)
    {
        Triangle t;

        std::cout << "Triangle " << i + 1
                  << " (x1 y1 x2 y2 x3 y3): ";

        std::cin >> t.a.x >> t.a.y
                 >> t.b.x >> t.b.y
                 >> t.c.x >> t.c.y;

        if (!std::cin)
        {
            std::cout << "Invalid coordinates\n";
            return 1;
        }

        t.color = colors[i % colors.size()];
        triangles.push_back(t);
    }

    std::cout << "Number of circles: ";
    std::cin >> circleCount;

    if (!std::cin || circleCount < 0 || circleCount > 100)
    {
        std::cout << "Invalid number of circles\n";
        return 1;
    }

    std::vector<Circle> circles;

    for (int i = 0; i < circleCount; i++)
    {
        Circle c;

        std::cout << "Circle " << i + 1
                  << " (centerX centerY radius): ";

        std::cin >> c.center.x >> c.center.y >> c.radius;

        if (!std::cin || c.radius <= 0)
        {
            std::cout << "Invalid circle parameters\n";
            return 1;
        }

        c.color = colors[(i + triangleCount) % colors.size()];
        circles.push_back(c);
    }

    for (const auto& t : triangles)
        drawTriangle(t);

    for (const auto& c : circles)
        drawCircle(c);

    std::filesystem::create_directories("output");
    saveBMP("output/rasterized.bmp");

    std::cout << "Image saved to output/rasterized.bmp\n";

    return 0;
}