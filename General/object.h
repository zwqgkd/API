#pragma once
#include<string>
#include<vector>
#include<opencv2/opencv.hpp>

/**
* @file
* @brief Contains container for various types of data.
*/

/**
* @brief base class for all objects
*/

struct ObjectBase {
public:
	virtual ~ObjectBase() {}
};

/**
 * @brief A template class for wrapping objects of type T.
 *
 * @tparam T The type of object to wrap.
 */

template<typename T>
struct Object : public ObjectBase {
public:
	Object(T* ptr);
	~Object();

	T        inner();
	T&		 inner_ref();
	const T& inner_const_ref();
private:
	T* ptr;
};

/**
* @brief Constructs an Object from a pointer to an object of type T.
*
* @param ptr A pointer to the object to wrap.
*/

template<typename T>
Object<T>::Object(T* ptr) : ptr(ptr) {}

/**
 * @brief The destructor.
 */

template<typename T>
Object<T>::~Object() {
	delete ptr;
}

/**
* @brief Gets the inner object.
*
* @return The inner object.
*/

template<typename T>
T Object<T>::inner() {
	return *ptr;
}

/**
* @brief Gets a const reference to the inner object.
*
* @return A const reference to the inner object.
*/

template<typename T>
const T& Object<T>::inner_const_ref() {
	return *ptr;
}

/**
 * @brief Gets a reference to the inner object.
 *
 * @return A reference to the inner object.
 */

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

/**
 * @brief Gets a copy of the inner object.
 * @param param_ptr pointer to the object.
 * @return A copy of the inner object.
 */
template<typename T>
T get_inner(ParamPtr param_ptr) {
	return dynamic_cast<Object<T>*>(param_ptr.get())->inner();
}
/**
 * @brief Gets a reference of the inner object.
 * @param param_ptr pointer to the object.
 * @return A reference of the inner object.
 */
template<typename T>
T& get_inner_ref(ParamPtr param_ptr) {
	return dynamic_cast<Object<T>*>(param_ptr.get())->inner_ref();
}
/**
 * @brief Gets a const reference of the inner object.
 * @param param_ptr pointer to the object.
 * @return A const reference of the inner object.
 */
template<typename T>
const T& get_inner_const_ref(ParamPtr param_ptr) {
	return dynamic_cast<Object<T>*>(param_ptr.get())->inner_const_ref();
}
/**
 * @brief Creates a ParamPtr of an inner object.
 * @param value_ptr a pointer to the inner object.
 * @return A ParamPtr of the object.
 */
template<typename T>
ParamPtr make_param(T* value_ptr) {
	return ParamPtr(new Object<T>(value_ptr));
}
