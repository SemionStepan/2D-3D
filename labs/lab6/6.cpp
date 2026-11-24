
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <filesystem>

const int WIDTH = 1000;
const int HEIGHT = 700;

struct Vec3
{
    double x, y, z;

    Vec3 operator+(const Vec3& v) const
    {
        return {x + v.x, y + v.y, z + v.z};
    }

    Vec3 operator-(const Vec3& v) const
    {
        return {x - v.x, y - v.y, z - v.z};
    }

    Vec3 operator*(double k) const
    {
        return {x * k, y * k, z * k};
    }

    Vec3 operator/(double k) const
    {
        return {x / k, y / k, z / k};
    }
};

double dot(Vec3 a, Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double length(Vec3 v)
{
    return std::sqrt(dot(v, v));
}

Vec3 normalize(Vec3 v)
{
    double len = length(v);
    return len > 0.0 ? v / len : Vec3{0, 0, 0};
}

struct Ray
{
    Vec3 origin;
    Vec3 direction;
};

struct Sphere
{
    Vec3 center;
    double radius;
    Vec3 color;
    double reflectivity;
};

struct Hit
{
    double distance;
    Vec3 point;
    Vec3 normal;
    Vec3 color;
    double reflectivity;
};

std::vector<Sphere> spheres = {
    {{-1.5, 0.0, 0.0}, 1.0, {1.0, 0.1, 0.1}, 0.35},
    {{ 0.8, 0.0, -0.5}, 1.0, {0.1, 0.3, 1.0}, 0.45},
    {{ 2.5, 0.0, 0.7}, 1.0, {0.1, 1.0, 0.2}, 0.25}
};

Vec3 lightPosition = {-4.0, 7.0, 5.0};

bool intersectSphere(const Ray& ray, const Sphere& sphere, double& t)
{
    Vec3 oc = ray.origin - sphere.center;

    double a = dot(ray.direction, ray.direction);
    double b = 2.0 * dot(oc, ray.direction);
    double c = dot(oc, oc) - sphere.radius * sphere.radius;

    double discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
        return false;

    double sqrtD = std::sqrt(discriminant);

    double t1 = (-b - sqrtD) / (2.0 * a);
    double t2 = (-b + sqrtD) / (2.0 * a);

    t = t1 > 0.001 ? t1 : t2;

    return t > 0.001;
}

bool intersectPlane(const Ray& ray, double& t)
{
    if (std::abs(ray.direction.y) < 0.0001)
        return false;

    t = (-1.0 - ray.origin.y) / ray.direction.y;

    return t > 0.001;
}

bool findClosestHit(const Ray& ray, Hit& hit)
{
    double closest = 1e30;
    bool found = false;

    for (const Sphere& sphere : spheres)
    {
        double t;

        if (intersectSphere(ray, sphere, t) && t < closest)
        {
            closest = t;
            hit.distance = t;
            hit.point = ray.origin + ray.direction * t;
            hit.normal = normalize(hit.point - sphere.center);
            hit.color = sphere.color;
            hit.reflectivity = sphere.reflectivity;
            found = true;
        }
    }

    double planeT;

    if (intersectPlane(ray, planeT) && planeT < closest)
    {
        closest = planeT;
        hit.distance = planeT;
        hit.point = ray.origin + ray.direction * planeT;
        hit.normal = {0, 1, 0};

        int checkerX = static_cast<int>(std::floor(hit.point.x));
        int checkerZ = static_cast<int>(std::floor(hit.point.z));

        bool dark = (checkerX + checkerZ) % 2 != 0;

        hit.color = dark ? Vec3{0.15, 0.15, 0.15}
                         : Vec3{0.75, 0.75, 0.75};

        hit.reflectivity = 0.25;
        found = true;
    }

    return found;
}

bool isInShadow(Vec3 point)
{
    Vec3 toLight = lightPosition - point;
    double lightDistance = length(toLight);

    Ray shadowRay;
    shadowRay.origin = point + normalize(toLight) * 0.002;
    shadowRay.direction = normalize(toLight);

    for (const Sphere& sphere : spheres)
    {
        double t;

        if (intersectSphere(shadowRay, sphere, t) && t < lightDistance)
            return true;
    }

    return false;
}

Vec3 trace(const Ray& ray, int depth)
{
    Hit hit;

    if (!findClosestHit(ray, hit))
    {
        double sky = 0.5 * (ray.direction.y + 1.0);

        return Vec3{0.08, 0.12, 0.2} * (1.0 - sky)
             + Vec3{0.35, 0.5, 0.75} * sky;
    }

    Vec3 lightDir = normalize(lightPosition - hit.point);
    Vec3 viewDir = normalize(ray.direction * -1.0);

    double diffuse = std::max(0.0, dot(hit.normal, lightDir));

    double illumination = 0.15;

    if (!isInShadow(hit.point))
        illumination += 0.85 * diffuse;

    Vec3 color = hit.color * illumination;

    if (depth > 0 && hit.reflectivity > 0.0)
    {
        Vec3 reflectedDirection =
            ray.direction - hit.normal * (2.0 * dot(ray.direction, hit.normal));

        Ray reflectedRay;
        reflectedRay.origin = hit.point + hit.normal * 0.002;
        reflectedRay.direction = normalize(reflectedDirection);

        Vec3 reflectedColor = trace(reflectedRay, depth - 1);

        color = color * (1.0 - hit.reflectivity)
              + reflectedColor * hit.reflectivity;
    }

    return color;
}

unsigned char toByte(double value)
{
    value = std::clamp(value, 0.0, 1.0);
    value = std::pow(value, 1.0 / 2.2);

    return static_cast<unsigned char>(value * 255.0);
}

void saveBMP(const std::string& filename, const std::vector<unsigned char>& pixels)
{
    int rowSize = (WIDTH * 3 + 3) & ~3;
    int imageSize = rowSize * HEIGHT;
    int fileSize = 54 + imageSize;

    std::ofstream file(filename, std::ios::binary);

    if (!file)
    {
        std::cout << "Cannot save image\n";
        return;
    }

    unsigned char header[54] = {};

    header[0] = 'B';
    header[1] = 'M';

    auto write32 = [&](int offset, uint32_t value)
    {
        header[offset] = value & 0xFF;
        header[offset + 1] = (value >> 8) & 0xFF;
        header[offset + 2] = (value >> 16) & 0xFF;
        header[offset + 3] = (value >> 24) & 0xFF;
    };

    header[2] = fileSize & 0xFF;
    header[3] = (fileSize >> 8) & 0xFF;
    header[4] = (fileSize >> 16) & 0xFF;
    header[5] = (fileSize >> 24) & 0xFF;

    header[10] = 54;
    header[14] = 40;

    write32(18, WIDTH);
    write32(22, HEIGHT);

    header[26] = 1;
    header[28] = 24;

    write32(34, imageSize);

    file.write(reinterpret_cast<char*>(header), 54);

    std::vector<unsigned char> row(rowSize, 0);

    for (int y = HEIGHT - 1; y >= 0; y--)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            int source = (y * WIDTH + x) * 3;
            int target = x * 3;

            row[target] = pixels[source + 2];
            row[target + 1] = pixels[source + 1];
            row[target + 2] = pixels[source];
        }

        file.write(reinterpret_cast<char*>(row.data()), rowSize);
    }

    file.close();

    std::cout << "Image saved: " << filename << "\n";
}

int main()
{
    std::vector<unsigned char> pixels(WIDTH * HEIGHT * 3);

    Vec3 camera = {0.0, 2.0, 7.0};

    double aspectRatio = static_cast<double>(WIDTH) / HEIGHT;
    double fov = std::tan(45.0 * 3.14159265 / 360.0);

    std::cout << "Ray Tracing started...\n";

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            double px = (2.0 * (x + 0.5) / WIDTH - 1.0) * aspectRatio * fov;
            double py = (1.0 - 2.0 * (y + 0.5) / HEIGHT) * fov;

            Ray ray;
            ray.origin = camera;
            ray.direction = normalize({px, py - 0.15, -1.0});

            Vec3 color = trace(ray, 2);

            int index = (y * WIDTH + x) * 3;

            pixels[index] = toByte(color.x);
            pixels[index + 1] = toByte(color.y);
            pixels[index + 2] = toByte(color.z);
        }
    }

    std::filesystem::create_directories("output");
    saveBMP("output/raytracing.bmp", pixels);

    std::cout << "Rendering completed!\n";

    return 0;
}