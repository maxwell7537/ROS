#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <cmath>

class LightBar {
public:
    cv::RotatedRect rotatedRect;  // 旋转矩形
    cv::Point2f endpoints[2];     // 灯条两端点
    cv::Point2f center;           // 灯条中心点
    double confidence;            // 置信度

    LightBar(const cv::RotatedRect& rect, double conf = 0.0);
    void calculatePoints();
    cv::Point2f getTopEndpoint() const;
    cv::Point2f getBottomEndpoint() const;
    cv::Point2f getCenter() const;
    double getLength() const;
    double getWidth() const;
    double getAngle() const;
};

class LightBarDetector {
private:
    int minArea;
    int maxArea;
    double minAspectRatio;
    double maxAspectRatio;
    double minFillRatio;
    double maxFillRatio;
    double minAngle;
    double maxAngle;
    int brightnessThreshold;

public:
    LightBarDetector();
    void setParams(int min_area, int max_area, double min_ar, double max_ar, 
                  double min_fill, double max_fill, double min_ang, double max_ang, int bright_thresh);
    cv::Mat preprocess(const cv::Mat& src);
    double calculateConfidence(const std::vector<cv::Point>& contour, const cv::RotatedRect& rect);
    std::vector<LightBar> detect(const cv::Mat& src);
    std::vector<LightBar> nonMaximumSuppression(const std::vector<LightBar>& lightBars, double overlapThreshold = 0.3);
    double calculateOverlap(const LightBar& lb1, const LightBar& lb2);
    void drawResults(cv::Mat& img, const std::vector<LightBar>& lightBars);
};

#endif // LIGHT_HPP