#pragma once
#include<string>
#include<vector>
#include<opencv2/opencv.hpp>

struct ObjectBase {
public:
    virtual ~ObjectBase() {}
};

template<typename T>
struct Object : public ObjectBase {
public:
    Object(T* ptr);
    ~Object();

    T        inner();
    T& inner_ref();
    const T& inner_const_ref();
private:
    T* ptr;
};

template<typename T>
Object<T>::Object(T* ptr) : ptr(ptr) {}

template<typename T>
Object<T>::~Object() {
    delete ptr;
}

template<typename T>
T Object<T>::inner() {
    return *ptr;
}

template<typename T>
const T& Object<T>::inner_const_ref() {
    return *ptr;
}

template<typename T>
T& Object<T>::inner_ref() {
    return *ptr;
}

using ParamPtr = std::shared_ptr<ObjectBase>;
using ParamPtrArray = std::vector<ParamPtr>;

using MatObject = Object<cv::Mat>;
using SizeObject = Object<cv::Size>;
using IntObject = Object<int>;
using DoubleObject = Object<double>;
using StringObject = Object<std::string>;

template<typename T>
T get_inner(ParamPtr param_ptr) {
    return dynamic_cast<Object<T>*>(param_ptr.get())->inner();
}

template<typename T>
T& get_inner_ref(ParamPtr param_ptr) {
    return dynamic_cast<Object<T>*>(param_ptr.get())->inner_ref();
}

template<typename T>
const T& get_inner_const_ref(ParamPtr param_ptr) {
    return dynamic_cast<Object<T>*>(param_ptr.get())->inner_const_ref();
}

template<typename T>
ParamPtr make_param(T* value_ptr) {
    return ParamPtr(new Object<T>(value_ptr));
}
