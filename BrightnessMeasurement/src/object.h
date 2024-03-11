#pragma once
#include "base.h"
#include <fmt/format.h>
#include <opencv2/opencv.hpp>
#include <string>
using std::string;

struct OperatorInterfaceBase {
public:
    virtual ~OperatorInterfaceBase() {}
};

template <typename T> struct OperatorInterface : public OperatorInterfaceBase {
public:
    OperatorInterface(T* ptr);
    template <typename... Args>
    OperatorInterface(Args &&...args)
        : ptr(new T(std::forward<Args>(args)...)) {}
    ~OperatorInterface();

    T        data();
    T& data_ref();
    const T& data_const_ref();

private:
    T* ptr;
};

using MatObject = OperatorInterface<cv::Mat>;
using SizeObject = OperatorInterface<cv::Size>;
using IntObject = OperatorInterface<int>;
using DoubleObject = OperatorInterface<double>;
using StringObject = OperatorInterface<std::string>;

using ParamPtr = std::shared_ptr<OperatorInterfaceBase>;
using ParamPtrArray = std::vector<ParamPtr>;

template <typename T>
OperatorInterface<T>::OperatorInterface(T* ptr) : ptr(ptr) {}

template <typename T> OperatorInterface<T>::~OperatorInterface() { delete ptr; }

template <typename T> T OperatorInterface<T>::data() { return *ptr; }

template <typename T> const T& OperatorInterface<T>::data_const_ref() {
    return *ptr;
}

template <typename T> T& OperatorInterface<T>::data_ref() { return *ptr; }

template <typename T>
inline auto try_cast(ParamPtr param_ptr) -> OperatorInterface<T>* {
    auto ptr = dynamic_cast<OperatorInterface<T> *>(param_ptr.get());
    if (ptr == nullptr) {
        throw std::runtime_error("Param type mismatch");
    }
    return ptr;
}

template <typename T> inline T get_data(ParamPtr param_ptr) {
    return try_cast<T>(param_ptr)->data();
}

template <typename T> inline T& get_data_ref(ParamPtr param_ptr) {
    return try_cast<T>(param_ptr)->data_ref();
}

template <typename T> inline const T& get_data_const_ref(ParamPtr param_ptr) {
    return try_cast<T>(param_ptr)->data_const_ref();
}

template <typename T, typename... Args> ParamPtr make_param(Args &&...args) {
    return std::make_shared<OperatorInterface<T>>(std::forward<Args>(args)...);
}

/**
 * @brief 异常类
 *
 * 代表了一种执行失败的异常情况，并提供了构造函数来设置异常的描述信息，以及重写了
 * what() 函数来返回异常的详细描述字符串。
 */
class ExecutionFailedException : public std::exception {
private:
    std::string error_kind_;
    std::string description_;
    std::string msg_;

public:
    /**
     * @brief 设置异常的描述信息
     * @param error_kind - 异常的种类
     * @param error_description - 异常的描述信息
     */
    ExecutionFailedException(const std::string& error_kind,
        const std::string& description)
        : error_kind_(error_kind), description_(description),
        msg_(error_kind + ": " + description) {}
    virtual const char* what() const throw() { return msg_.c_str(); }

    std::string get_error_kind() const { return error_kind_; }
    std::string get_description() const { return description_; }
};

/**
 *  @brief 算子状态枚举
 */
enum class OperatorStatus {
    NG, // 算子执行失败
    OK, // 算子执行成功
    //...
};

/**
 *  @brief 图像类型
 */
struct GeliMat {

    cv::Mat mat;    // 图像数据
    int     width;  // 图像宽度
    int     height; // 图像高度
    int     type;   // 图像像素格式

    GeliMat(const cv::Mat& mat) : mat(mat) {
        width = mat.cols;
        height = mat.rows;
        type = mat.type();
    }
    GeliMat(cv::Mat& mat, int width, int height, int type)
        : mat(mat), width(width), height(height), type(type) {}
    GeliMat(const GeliMat& other)
        : mat(other.mat), width(other.width), height(other.height),
        type(other.type) {}
    GeliMat(GeliMat&& other) noexcept
        : mat(std::move(other.mat)), width(other.width), height(other.height),
        type(other.type) {}
    GeliMat& operator=(const GeliMat& other) {
        mat = other.mat;
        width = other.width;
        height = other.height;
        type = other.type;
        return *this;
    }
    ~GeliMat() {}

    ParamPtr reflect(const string& token, string& rftype) {
        if (token == "height") {
            rftype = "int";
            return make_param<int>(height);
        }
        else if (token == "width") {
            rftype = "int";
            return make_param<int>(width);
        }
        else if (token == "type") {
            rftype = "int";
            return make_param<int>(type);
        }
        else {
            throw ExecutionFailedException(
                "runtime_error", fmt::format("reflect {} not support", token));
        }
    }
};

/**
 *  @brief 点类型
 */
struct GeliPoint {
    double x;
    double y;

    GeliPoint() : x(0), y(0) {}
    GeliPoint(double x, double y) : x(x), y(y) {}
    GeliPoint(const GeliPoint& other) : x(other.x), y(other.y) {}
    GeliPoint& operator=(const GeliPoint& other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    ParamPtr reflect(const string& token, string& rftype) {
        if (token == "x") {
            rftype = "double";
            return make_param<double>(x);
        }
        else if (token == "y") {
            rftype = "double";
            return make_param<double>(y);
        }
        else {
            throw ExecutionFailedException(
                "runtime_error", fmt::format("reflect {} not support", token));
        }
    }
};

/**
 * @brief 矩形类型
 */
struct GeliRect {
    GeliPoint center;
    double    width;
    double    height;
    double    angle;

    GeliRect(double x, double y, double width, double height, double angle)
        : center({ x, y }), width(width), height(height), angle(angle) {}
    GeliRect(const GeliRect& other)
        : center(other.center), width(other.width), height(other.height),
        angle(other.angle) {}
    GeliRect& operator=(const GeliRect& other) {
        center = other.center;
        width = other.width;
        height = other.height;
        angle = other.angle;
        return *this;
    }
    ParamPtr reflect(const string& token, string& rftype) {
        if (token == "center") {
            rftype = "GeliPoint";
            return make_param<GeliPoint>(center);
        }
        else if (token == "center.x") {
            rftype = "double";
            return make_param<double>(center.x);
        }
        else if (token == "center.y") {
            rftype = "double";
            return make_param<double>(center.y);
        }
        else if (token == "width") {
            rftype = "double";
            return make_param<double>(width);
        }
        else if (token == "height") {
            rftype = "double";
            return make_param<double>(height);
        }
        else if (token == "angle") {
            rftype = "double";
            return make_param<double>(angle);
        }
        else {
            throw ExecutionFailedException(
                "runtime_error", fmt::format("reflect {} not support", token));
        }
    }
};

/**
 *  @brief 圆类型
 */
struct GeliCircle {
    GeliPoint center;
    double    dI;
    double    dO;
    double    angleStart;
    double    angleRange;
    GeliCircle(double x, double y, double dI, double dO, double angleStart,
        double angleRange)
        : center({ x, y }), dI(dI), dO(dO), angleStart(angleStart),
        angleRange(angleRange) {}
    GeliCircle(const GeliCircle& other)
        : center(other.center), dI(other.dI), dO(other.dO),
        angleStart(other.angleStart), angleRange(other.angleRange) {}
    GeliCircle& operator=(const GeliCircle& other) {
        center = other.center;
        dI = other.dI;
        dO = other.dO;
        angleStart = other.angleStart;
        angleRange = other.angleRange;
        return *this;
    }
    ParamPtr reflect(const string& token, string& rftype) {
        if (token == "center") {
            rftype = "GeliPoint";
            return make_param<GeliPoint>(center);
        }
        else if (token == "center.x") {
            rftype = "double";
            return make_param<double>(center.x);
        }
        else if (token == "center.y") {
            rftype = "double";
            return make_param<double>(center.y);
        }
        else if (token == "dI") {
            rftype = "double";
            return make_param<double>(dI);
        }
        else if (token == "dO") {
            rftype = "double";
            return make_param<double>(dO);
        }
        else if (token == "angleStart") {
            rftype = "double";
            return make_param<double>(angleStart);
        }
        else if (token == "angleRange") {
            rftype = "double";
            return make_param<double>(angleRange);
        }
        else {
            throw ExecutionFailedException(
                "runtime_error", fmt::format("reflect {} not support", token));
        }
    }
};

/**
 *  @brief 屏蔽区多边形类型
 */
struct GeliShield {
    // to_string
    std::vector<std::vector<GeliPoint>> shields;
    GeliShield() {}
    GeliShield(const std::vector<std::vector<GeliPoint>>& shields)
        : shields(shields) {}
    GeliShield(const GeliShield& other) : shields(other.shields) {}
    GeliShield(GeliShield&& other) noexcept
        : shields(std::move(other.shields)) {}

    ParamPtr reflect(const string& token, string& rftype) {
        if (token == "polygon_count") {
            rftype = "vector<int>";
            std::vector<int> ans;
            for (const auto& s : shields) {
                ans.push_back(s.size());
            }
            return make_param<std::vector<int>>(std::move(ans));
        }
        else if (token == "points") {
            rftype = "vector<GeliPoint>";
            std::vector<GeliPoint> ans;
            for (const auto& s : shields) {
                for (const auto& p : s) {
                    ans.push_back(p);
                }
            }
            return make_param<std::vector<GeliPoint>>(std::move(ans));
        }
        else if (token == "points.xs") {
            rftype = "vector<double>";
            std::vector<double> xs;
            for (const auto& s : shields) {
                for (const auto& p : s) {
                    xs.push_back(p.x);
                }
            }
            return make_param<std::vector<double>>(std::move(xs));
        }
        else if (token == "points.ys") {
            rftype = "vector<double>";
            std::vector<double> ys;
            for (const auto& s : shields) {
                for (const auto& p : s) {
                    ys.push_back(p.y);
                }
            }
            return make_param<std::vector<double>>(std::move(ys));
        }
        else {
            throw ExecutionFailedException(
                "runtime_error", fmt::format("reflect {} not support", token));
        }
    }
};

struct RoiData {
public:
    virtual ~RoiData() {}
};

struct RectRoiData : public RoiData {
public:
    double center_x;
    double center_y;
    double width;
    double height;
    double angle;

public:
    RectRoiData(double center_x, double center_y, double width, double height,
        double angle)
        : center_x(center_x), center_y(center_y), width(width), height(height),
        angle(angle) {}
    virtual ~RectRoiData() {}
};

struct PolygonRoiData : public RoiData {
    std::vector<GeliPoint> points;
    PolygonRoiData(const std::vector<GeliPoint>& points) : points(points) {}
};

struct CircleRoiData : public RoiData {
public:
    double center_x;
    double center_y;
    double d_i;
    double d_o;
    double angle_start;
    double angle_range;

public:
    CircleRoiData(double center_x, double center_y, double di, double do_,
        double angle_start, double angle_range)
        : center_x(center_x), center_y(center_y), d_i(di), d_o(do_),
        angle_start(angle_start), angle_range(angle_range) {}
    virtual ~CircleRoiData() {}
};

struct Caliper : public RoiData {
public:
    std::vector<GeliRect> rects;

public:
    Caliper(const std::vector<GeliRect>& rects) : rects(rects) {}
    virtual ~Caliper() {}
    // 计算两点之间的距离
    double distance(cv::Point2f p1, cv::Point2f p2) {
        return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
    }
    // 计算线段上的点的周围4个整数点
    std::vector<cv::Point> findPointsOnSegment(cv::Point2f p1, cv::Point2f p2,
        int step) {
        double                 length = distance(p1, p2);
        double                 x = p1.x + (p2.x - p1.x) * step / length;
        double                 y = p1.y + (p2.y - p1.y) * step / length;
        std::vector<cv::Point> points;
        points.emplace_back(ceil(x), floor(y));
        points.emplace_back(ceil(x), ceil(y));
        points.emplace_back(floor(x), floor(y));
        points.emplace_back(floor(x), ceil(y));
        return points;
    }
    void caliperPaint(cv::Mat& src, const std::vector<cv::RotatedRect>& rects) {
        std::vector<cv::Point2f> vertices;
        for (const auto& rect : rects) {
            rect.points(vertices);
            line(src, vertices[0], vertices[1], cv::Scalar(0));
            line(src, vertices[2], vertices[3], cv::Scalar(0));
        }
    }
    std::vector<GeliPoint> get_points(GeliMat& src) {
        std::vector<cv::RotatedRect> cv_rects;
        for (const auto& rect : rects) {
            cv::RotatedRect cv_rect(cv::Point2f(rect.center.x, rect.center.y),
                cv::Size2f(rect.width, rect.height),
                rect.angle);
            cv_rects.push_back(cv_rect);
        }

        cv::Mat gray;
        cv::cvtColor(src.mat, gray, cv::COLOR_BGR2GRAY);

        caliperPaint(gray, cv_rects);
        // Harris角点检测参数
        int    blockSize = 2;    // 角点检测中考虑的邻域大小
        int    apertureSize = 3;    // Sobel算子的孔径大小
        double k = 0.04; // Harris参数

        // 执行Harris角点检测
        cv::Mat dst =
            cv::Mat::zeros(src.mat.size(), CV_32FC1); // 存储角点响应的矩阵
        cornerHarris(gray, dst, blockSize, apertureSize, k, cv::BORDER_DEFAULT);
        int thresh = 100; // 阈值
        // 将角点响应规范化
        cv::Mat dstNorm;
        cv::Mat dstNormTODisplay;
        normalize(dst, dstNorm, 0, 255, cv::NORM_MINMAX, CV_32FC1, cv::Mat());

        std::vector<cv::Point2f> vertices;
        std::vector<GeliPoint>   ans;

        for (const auto& rect : cv_rects) {
            int step; // 遍历步长
            rect.points(vertices);
            std::vector<cv::Point> points;

            for (int i = 0; i < 3; i += 2) {
                int j;
                if (i == 0)
                    j = 1;
                else
                    j = 3;

                step = 2; // 只考虑竖直边
                while (step < distance(vertices[i], vertices[j]) - 1) {
                    std::vector<cv::Point> points =
                        findPointsOnSegment(vertices[i], vertices[j], step);
                    for (const auto& p : points) {
                        if (static_cast<int>(dstNorm.at<float>(p.y, p.x) >
                            thresh)) {
                            ans.emplace_back(p.x, p.y);
                        }
                    }
                }
            }
        }
        return ans;
    }
};

struct LineCaliper : public Caliper {
public:
    double start_x;
    double start_y;
    double end_x;
    double end_y;
    int    caliper_cnt;
    double caliper_width;
    double caliper_height;

public:
    LineCaliper(double start_x, double start_y, double end_x, double end_y,
        int caliper_cnt, double caliper_height, double caliper_width,
        const std::vector<GeliRect>& rects)
        : start_x(start_x), start_y(start_y), end_x(end_x), end_y(end_y),
        caliper_cnt(caliper_cnt), caliper_width(caliper_width),
        caliper_height(caliper_height), Caliper(rects) {
        if (rects.size() != caliper_cnt) {
            throw ExecutionFailedException(
                "runtime_error",
                fmt::format("rects size {} not equal to caliper_cnt {}",
                    rects.size(), caliper_cnt));
        }
    }
    virtual ~LineCaliper() {}
};

struct CircleCaliper : public Caliper {
public:
    double center_x;
    double center_y;
    double r;
    int    caliper_cnt;
    double caliper_height;
    double caliper_width;

public:
    CircleCaliper(double center_x, double center_y, double r, int caliper_cnt,
        double caliper_height, double caliper_width,
        const std::vector<GeliRect>& rects)
        : center_x(center_x), center_y(center_y), r(r),
        caliper_cnt(caliper_cnt), caliper_width(caliper_width),
        caliper_height(caliper_height), Caliper(rects) {
        if (rects.size() != caliper_cnt) {
            throw ExecutionFailedException(
                "runtime_error", fmt::format("rects size {} not equal to "
                    "caliper_cnt {}",
                    rects.size(), caliper_cnt));
        }
    }
    virtual ~CircleCaliper() {}
};

struct Roi {
private:
    // 函数用于在掩膜图像上扣去屏蔽区域
    void subtractShieldAreas(const GeliShield& shield, cv::Mat& mask) {
        for (const auto& polygon : shield.shields) {
            std::vector<cv::Point> cvPolygon;
            for (const auto& point : polygon) {
                cvPolygon.push_back(cv::Point(point.x, point.y));
            }

            // 将多边形绘制到掩膜图像上
            cv::fillPoly(mask, std::vector<std::vector<cv::Point>>{cvPolygon},
                cv::Scalar(0));
        }
    }

public:
    bool mask;     // 是否输出掩膜
    bool mat_flag; // 选择图像类型true还是图形类型false
    GeliMat
        mask_mat; // 掩膜图像,ROI选择图像类型时有效，图形类型的时候数据在data_和shield中

    std::string              type;   // roi类型 {"rect" | "circle"}
    std::shared_ptr<RoiData> data;   // RoiData数据
    GeliShield               shield; // 屏蔽区

public:
    Roi(bool mask, bool mat_flag, GeliMat mask_mat, const std::string& type,
        RoiData* roi_ptr, const GeliShield& shield)
        : mask(mask), mat_flag(mat_flag), mask_mat(mask_mat), type(type),
        data(roi_ptr), shield(shield) {}
    ~Roi() {}
    void generate_mask_mat(GeliMat& src_mat) {
        mask_mat = GeliMat(
            cv::Mat(src_mat.height, src_mat.width, CV_8UC1, cv::Scalar(0)));
        if (type == "") {
            return;
        }
        else if (type == "rect") {
            RectRoiData* rect_roi_ptr = dynamic_cast<RectRoiData*>(data.get());
            if (rect_roi_ptr == nullptr) {
                throw ExecutionFailedException(
                    "runtime_error",
                    fmt::format("roi type {} not support", type));
            }
            cv::RotatedRect rect(
                cv::Point2f(rect_roi_ptr->center_x, rect_roi_ptr->center_y),
                cv::Size2f(rect_roi_ptr->width, rect_roi_ptr->height),
                rect_roi_ptr->angle);
            cv::Point2f vertices[4];
            rect.points(vertices);
            std::vector<std::vector<cv::Point>> contours;
            contours.push_back(std::vector<cv::Point>(vertices, vertices + 4));
            cv::drawContours(mask_mat.mat, contours, 0, cv::Scalar(255), -1);
            subtractShieldAreas(shield, mask_mat.mat);
            // mask_mat(mask_mat);
        }
        else if (type == "circle") {
            CircleRoiData* circle_roi_ptr =
                dynamic_cast<CircleRoiData*>(data.get());
            if (circle_roi_ptr == nullptr) {
                throw ExecutionFailedException(
                    "runtime_error",
                    fmt::format("roi type {} not support", type));
            }
            cv::Point center(circle_roi_ptr->center_x,
                circle_roi_ptr->center_y);
            cv::circle(mask_mat.mat, center, circle_roi_ptr->d_o,
                cv::Scalar(255), -1);
            cv::circle(mask_mat.mat, center, circle_roi_ptr->d_i, cv::Scalar(0),
                -1);
            cv::Mat element =
                cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
            cv::morphologyEx(mask_mat.mat, mask_mat.mat, cv::MORPH_OPEN,
                element);
            subtractShieldAreas(shield, mask_mat.mat);
        }
        else if (type == "polygon") {
            PolygonRoiData* polygon_roi_ptr =
                dynamic_cast<PolygonRoiData*>(data.get());
            if (polygon_roi_ptr == nullptr) {
                throw ExecutionFailedException(
                    "runtime_error",
                    fmt::format("roi type {} not support", type));
            }
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Point>              cvPolygon;
            for (const auto& point : polygon_roi_ptr->points) {
                cvPolygon.push_back(cv::Point(point.x, point.y));
            }
            contours.push_back(cvPolygon);
            cv::drawContours(mask_mat.mat, contours, 0, cv::Scalar(255), -1);
            subtractShieldAreas(shield, mask_mat.mat);
        }
        else {
            throw ExecutionFailedException(
                "runtime_error", fmt::format("roi type {} not support", type));
        }
    }
    /*
     * @brief 生成检测区域
     * @return 检测区域
     */
    GeliRect generate_area() {
        RectRoiData* rect_roi_ptr = dynamic_cast<RectRoiData*>(data.get());
        return GeliRect(rect_roi_ptr->center_x, rect_roi_ptr->center_y,
            rect_roi_ptr->width, rect_roi_ptr->height,
            rect_roi_ptr->angle);
    }
    /*
     * @brief 生成圆弧区域
     * @return 圆弧
     */
    GeliCircle generate_circle() {
        CircleRoiData* circle_roi_ptr =
            dynamic_cast<CircleRoiData*>(data.get());
        return GeliCircle(circle_roi_ptr->center_x, circle_roi_ptr->center_y,
            circle_roi_ptr->d_i, circle_roi_ptr->d_o,
            circle_roi_ptr->angle_start,
            circle_roi_ptr->angle_range);
    }

    /*
     *  @brief 生成卡尺交点
     */
    std::vector<GeliPoint> generate_corner_points(GeliMat& gelimat) {
        if (type == "lineCaliper" || type == "circleCaliper") {
            Caliper* caliper_ptr = dynamic_cast<Caliper*>(data.get());
            return caliper_ptr->get_points(gelimat);
        }
        else {
            throw ExecutionFailedException(
                "runtime_error", fmt::format("roi type {} not support", type));
        }
    }
};