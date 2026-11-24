
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>

int main()
{
    cv::Mat image = cv::imread("assets/coins.jpg");

    if (image.empty())
    {
        std::cout << "Failed to load image" << std::endl;
        return 1;
    }

    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(9, 9), 2);

    std::vector<cv::Vec3f> circles;

    cv::HoughCircles(
        blurred,
        circles,
        cv::HOUGH_GRADIENT,
        1.5,
        40,
        200,
        60,
        50,
        150
    );

    for (const auto& circle : circles)
    {
        cv::Point center(
            cvRound(circle[0]),
            cvRound(circle[1])
        );

        int radius = cvRound(circle[2] / 3.0);

        cv::circle(image, center, radius, cv::Scalar(0, 255, 0), 3);
        cv::circle(image, center, 3, cv::Scalar(0, 0, 255), -1);
    }

    std::filesystem::create_directories("output");

    cv::imwrite("output/coins_detected.jpg", image);

    std::cout << "Detected circles: " << circles.size() << std::endl;
    std::cout << "Result saved to output/coins_detected.jpg" << std::endl;

    return 0;
}
