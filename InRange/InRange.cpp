
#include <opencv2/core/core.hpp>                   
#include <iostream> 
using namespace cv;

#if defined(__cplusplus)//
extern "C" {
#endif
	__declspec(dllexport) void myInRange(InputArray src, InputArray lowerb, InputArray upperb, OutputArray dst);
#ifdef __cplusplus
}
#endif
void myInRange(InputArray src,InputArray lowerb,InputArray upperb, OutputArray dst) {

	inRange(src,
		lowerb,
		upperb,
		dst
	);
}
