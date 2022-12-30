#include "polyDetect.h"


//����μ�� https://blog.csdn.net/Sun_tian/article/details/104221658
void polygoDetect(Mat& srcImg, double epsilon, int minAcreage, vector<vector<float>>& lines, Mat& dst)
{
	//��ɫͼת�Ҷ�ͼ
	Mat src_gray;
	cvtColor(srcImg, src_gray, CV_BGR2GRAY);
	//imshow("1.��ɫͼת�Ҷ�ͼ", src_gray);
	//Otsu�Զ���ֵ
	Mat threshold_output;
	threshold(src_gray, threshold_output, 0, 255, THRESH_BINARY | THRESH_OTSU); //Otsu ��ֵ��
	//imshow("2.��ֵ����Otsu�Զ���ֵ��", threshold_output);
	//�˲�
	Mat threshold_output_copy = threshold_output.clone();
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(1, 1)); //����˴�С�ɱ䣬�Ƽ�1��3���˴�Ϊ1
	morphologyEx(threshold_output_copy, threshold_output_copy, MORPH_OPEN, element); //������
	morphologyEx(threshold_output_copy, threshold_output_copy, MORPH_CLOSE, element); //������
	//imshow("3.������+������", threshold_output_copy);
	//��Ե���
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat canny_output;
	Canny(threshold_output_copy, canny_output, 1, 3, 7, true);  //Canny���
	//imshow("4.Canny��Ե���", canny_output);
	//��������
	Mat image = canny_output.clone();
	findContours(image, contours, hierarchy, CV_RETR_LIST, CHAIN_APPROX_SIMPLE, Point());
	Mat Contours = Mat::zeros(image.size(), CV_8UC1);  //����
	//contours[i]������ǵ�i��������contours[i].size()������ǵ�i�����������е����ص���
	for (int i = 0; i < contours.size(); i++)
		for (int j = 0; j < contours[i].size(); j++) {
			Point P = Point(contours[i][j].x, contours[i][j].y);
			Contours.at<uchar>(P) = 255;
		}
	//imshow("5.findContours������ȡ", Contours);
	Mat mat6 = Contours.clone();
	cvtColor(mat6, mat6, CV_GRAY2BGR);
	vector<vector<Point>> contours_poly;
	for (int i = 0; i < contours.size(); i++) {
		double acreage = contourArea(contours[i], true);
		if (acreage > minAcreage) { //���ɸѡ
			vector<Point> contourspoly;
			approxPolyDP(contours[i], contourspoly, epsilon, true);//��ָ�����ȱƽ����������
			contours_poly.push_back(contourspoly);
		}
	}
	vector<vector<Point>> polygonDetectRet = contours_poly;

	//������Ľ������2���Ķ˵�����˵�����������
	if (polygonDetectRet.size() == 2 and polygonDetectRet[0].size() == polygonDetectRet[1].size()) {
		for (int i = 0; i < polygonDetectRet[0].size(); i++) {
			polygonDetectRet[0][i] = (polygonDetectRet[0][i] + polygonDetectRet[1][i]) / 2;
			circle(dst, polygonDetectRet[0][i], 4, Scalar(0, 0, 255), -1);

		}

		//����
		for (int j = 0; j < polygonDetectRet[0].size(); j++) {
			if (j == polygonDetectRet[0].size() - 1) {
				cv::line(dst, polygonDetectRet[0][j], polygonDetectRet[0][0], Scalar(0, 0, 255), 3, LINE_AA);
				float x1 = polygonDetectRet[0][j].x, y1 = polygonDetectRet[0][j].y, x2 = polygonDetectRet[0][0].x, y2 = polygonDetectRet[0][0].y;
				vector<float> temp;
				temp.push_back(x1);
				temp.push_back(y1);
				temp.push_back(x2);
				temp.push_back(y2);
				lines.push_back(temp);
			}
			else {
				cv::line(dst, polygonDetectRet[0][j], polygonDetectRet[0][j + 1], Scalar(0, 0, 255), 3, LINE_AA);
				float x1 = polygonDetectRet[0][j].x, y1 = polygonDetectRet[0][j].y, x2 = polygonDetectRet[0][j + 1].x, y2 = polygonDetectRet[0][j + 1].y;
				vector<float> temp;
				temp.push_back(x1);
				temp.push_back(y1);
				temp.push_back(x2);
				temp.push_back(y2);
				lines.push_back(temp);
			}

		}

		for (int j = 0; j < lines.size(); j++) {

			std::cout << lines[j][0] << "," << lines[j][1] << " " << lines[j][2] << "," << lines[j][3] << std::endl;

		}
	}
}