#include "light.hpp"

LightBar::LightBar(const cv::RotatedRect& rect, double conf) {
    rotatedRect = rect;
    confidence = conf;
    calculatePoints();
}

void LightBar::calculatePoints() {
    cv::Point2f vertices[4];
    rotatedRect.points(vertices);
    
    // 按y坐标排序顶点，找到顶部和底部
    std::vector<cv::Point2f> sortedVertices(vertices, vertices + 4);
    std::sort(sortedVertices.begin(), sortedVertices.end(), 
             [](const cv::Point2f& a, const cv::Point2f& b) {
                 return a.y < b.y;
             });
    cv::Point2f topCenter = (sortedVertices[0] + sortedVertices[1]) * 0.5f;
    cv::Point2f bottomCenter = (sortedVertices[2] + sortedVertices[3]) * 0.5f;
    endpoints[0] = topCenter;
    endpoints[1] = bottomCenter;
    center = rotatedRect.center;
}

cv::Point2f LightBar::getTopEndpoint() const {
    return endpoints[0];
}

cv::Point2f LightBar::getBottomEndpoint() const {
    return endpoints[1];
}

cv::Point2f LightBar::getCenter() const {
    return center;
}

double LightBar::getLength() const {
    return cv::norm(endpoints[0] - endpoints[1]);
}

double LightBar::getWidth() const {
    return rotatedRect.size.width < rotatedRect.size.height ? 
           rotatedRect.size.width : rotatedRect.size.height;
}

double LightBar::getAngle() const {
    double angle = rotatedRect.angle;
    if (rotatedRect.size.width > rotatedRect.size.height) {
        angle = 90 + angle;
    }
    return std::abs(angle);
}

LightBarDetector::LightBarDetector() {
    minArea = 50;
    maxArea = 2000;
    minAspectRatio = 0.001;
    maxAspectRatio = 50;
    minFillRatio = 0.2;
    maxFillRatio = 1;
    minAngle = -90;
    maxAngle = 90.0;
    brightnessThreshold = 200;
}

void LightBarDetector::setParams(int min_area, int max_area, double min_ar, double max_ar, 
                                double min_fill, double max_fill, double min_ang, double max_ang, int bright_thresh) {
    minArea = min_area;
    maxArea = max_area;
    minAspectRatio = min_ar;
    maxAspectRatio = max_ar;
    minFillRatio = min_fill;
    maxFillRatio = max_fill;
    minAngle = min_ang;
    maxAngle = max_ang;
    brightnessThreshold = bright_thresh;
}

cv::Mat LightBarDetector::preprocess(const cv::Mat& src) {
    cv::Mat gray, binary, hsv, brightness_mask;
    if (src.channels() == 3) {
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
        std::vector<cv::Mat> hsv_channels;
        cv::split(hsv, hsv_channels);
        cv::Mat v_channel = hsv_channels[2];
        cv::threshold(v_channel, brightness_mask, brightnessThreshold, 255, cv::THRESH_BINARY);
    } else {
        gray = src.clone();
        cv::threshold(gray, brightness_mask, brightnessThreshold, 255, cv::THRESH_BINARY);
    }
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 1.5);
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);
    cv::bitwise_and(binary, brightness_mask, binary);
    cv::Mat kernel_open = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
    cv::Mat kernel_close = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 7));
    cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel_open);
    cv::morphologyEx(binary, binary, cv::MORPH_CLOSE, kernel_close);
    return binary;
}

double LightBarDetector::calculateConfidence(const std::vector<cv::Point>& contour, const cv::RotatedRect& rect) {
    double confidence = 0.0;
    double area = cv::contourArea(contour);
    double rectArea = rect.size.width * rect.size.height;
    double fillRatio = area / rectArea;
    double fillScore = std::min(1.0, fillRatio / 0.7);
    double width = rect.size.width;
    double height = rect.size.height;
    double aspectRatio = (width > height) ? width / height : height / width;
    double aspectScore = 1.0 - std::abs(aspectRatio - 8.0) / 8.0;
    aspectScore = std::max(0.0, std::min(1.0, aspectScore));
    double angle = rect.angle;
    double angleScore = 1.0 - std::min(std::abs(angle - 90), std::abs(angle + 90)) / 90.0;
    confidence = (fillScore + aspectScore + angleScore) / 3.0;
    return confidence;
}

std::vector<LightBar> LightBarDetector::detect(const cv::Mat& src) {
    std::vector<LightBar> lightBars;
    cv::Mat binary = preprocess(src);
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(binary, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area < minArea || area > maxArea) continue;
        cv::RotatedRect rect = cv::minAreaRect(contour);
        double width = rect.size.width;
        double height = rect.size.height;
        double aspectRatio = (width > height) ? width / height : height / width;
        if (aspectRatio < minAspectRatio || aspectRatio > maxAspectRatio) continue;
        double rectArea = width * height;
        double fillRatio = area / rectArea;
        if (fillRatio < minFillRatio || fillRatio > maxFillRatio) continue;
        double confidence = calculateConfidence(contour, rect);
        LightBar lightBar(rect, confidence);
        lightBars.push_back(lightBar);
    }
    std::sort(lightBars.begin(), lightBars.end(), 
             [](const LightBar& a, const LightBar& b) {
                 return a.confidence > b.confidence;
             });
    return lightBars;
}

std::vector<LightBar> LightBarDetector::nonMaximumSuppression(const std::vector<LightBar>& lightBars, double overlapThreshold) {
    if (lightBars.empty()) return lightBars;
    std::vector<LightBar> result;
    std::vector<bool> suppressed(lightBars.size(), false);
    for (size_t i = 0; i < lightBars.size(); ++i) {
        if (suppressed[i]) continue;
        result.push_back(lightBars[i]);
        for (size_t j = i + 1; j < lightBars.size(); ++j) {
            if (suppressed[j]) continue;
            double overlap = calculateOverlap(lightBars[i], lightBars[j]);
            if (overlap > overlapThreshold) {
                suppressed[j] = true;
            }
        }
    }
    return result;
}

double LightBarDetector::calculateOverlap(const LightBar& lb1, const LightBar& lb2) {
    std::vector<cv::Point2f> intersection;
    cv::rotatedRectangleIntersection(lb1.rotatedRect, lb2.rotatedRect, intersection);
    if (intersection.empty()) return 0.0;
    double intersectionArea = cv::contourArea(intersection);
    double unionArea = lb1.rotatedRect.size.width * lb1.rotatedRect.size.height +
                      lb2.rotatedRect.size.width * lb2.rotatedRect.size.height -
                      intersectionArea;
    return intersectionArea / unionArea;
}

void LightBarDetector::drawResults(cv::Mat& img, const std::vector<LightBar>& lightBars) {
    for (const auto& lightBar : lightBars) {
        cv::Scalar color = cv::Scalar(0, 255, 0);
        cv::Point2f vertices[4];
        lightBar.rotatedRect.points(vertices);
        for (int i = 0; i < 4; i++) {
            cv::line(img, vertices[i], vertices[(i + 1) % 4], color, 2);
        }
        cv::circle(img, lightBar.endpoints[0], 5, cv::Scalar(0, 0, 255), -1);
        cv::circle(img, lightBar.endpoints[1], 5, cv::Scalar(0, 0, 255), -1);
        cv::circle(img, lightBar.center, 5, cv::Scalar(255, 0, 0), -1);
        cv::line(img, lightBar.endpoints[0], lightBar.endpoints[1], cv::Scalar(255, 0, 0), 2);
        std::string info = "C:" + std::to_string(lightBar.confidence).substr(0,4);
        cv::putText(img, info, cv::Point(lightBar.center.x - 20, lightBar.center.y - 20),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    }
}