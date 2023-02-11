#include "objectDetect.h"

void myNMSBoxes(const std::vector<Rect>& bboxes, const std::vector<float>& scores, const float score_threshold, const float nms_threshold, std::vector<int>& indices, const float eta, const int top_k) {
	cv::dnn::NMSBoxes( bboxes, scores, score_threshold, nms_threshold, indices, eta , top_k);
}


cv::dnn::Net myReadNet(const String &cfgFile) {
	return cv::dnn::readNet(cfgFile);
}


void myBlobFromImage(InputArray image, OutputArray blob, double scalefactor, const Size& size, const Scalar& mean, bool swapRB, bool crop, int ddepth) {
	cv::dnn::blobFromImage(image, blob, scalefactor, size, mean, swapRB, crop, ddepth);
}
