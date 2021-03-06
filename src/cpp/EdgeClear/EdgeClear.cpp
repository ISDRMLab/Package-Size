// EdgeClear.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2\features2d\features2d.hpp>

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

//#define FILTER 
#define CANNY 
#define MORPHOLOGY 
#define HOUGHLINES
//#define FIND_CONTOURS
//#define CORNER_FAST
//#define GODD_FEATURE
//#define HARRIS
//#define CANNYHOUGH

void removeDensePoints(Mat &input, Mat &output);
void CannyAndHough(Mat &input, Mat &output);
void removeDenseRegions(Mat &input, Mat &output);



void main()
{

	Mat srcMat = imread("9.jpg");
	imshow("origin", srcMat);

	Mat input = srcMat.clone();
	

#ifdef FILTER
	//滤波
	Mat medianMat, guassianMat, meanMat;
	medianBlur(input, medianMat, 3);
	//GaussianBlur(srcMat, guassianMat, Size(3, 3), 1, 1);
	imshow("blur", medianMat);
	input = medianMat.clone();
#endif // FILTER


#ifdef CANNY
	Mat cannyMat, cannyMat2;
	Canny(input, cannyMat, 90, 180, 3);
	//removeDenseRegions(cannyMat, cannyMat2);

	imshow("canny", cannyMat);

	input = cannyMat.clone();
	//Mat temp;
	//removeDensePoints(input, temp);
#endif // CANNY

#ifdef CANNYHOUGH
	Mat ooo= srcMat.clone();
	CannyAndHough(srcMat, ooo);
	imshow("cchh", ooo);
#endif // CANNYHOUGH


	


#ifdef MORPHOLOGY
	//形态学操作
	Mat morMat, morMat2;
	Mat se = getStructuringElement(MORPH_RECT, Size(3, 3));
	morphologyEx(input, morMat, MORPH_DILATE, se);
	morphologyEx(morMat, morMat, MORPH_ERODE, se);
	imshow("morphologyEx", morMat);
	input = morMat.clone();
#endif // MORPHOLOGY



#ifdef HOUGHLINES
	//直线检测
	vector<Vec4i> lines;
	HoughLinesP(input, lines, 1, CV_PI / 180, 90, 100, 100);
	//画直线
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(srcMat, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 60, 255), 1, CV_AA);
	}
	imshow("HoughLinesP", srcMat);

#endif // HOUGHLINES



#ifdef FIND_CONTOURS
	//寻找轮廓
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(input, contours, hierarchy, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	Mat contourMat = Mat(input.size(), input.type(), Scalar(255));
	drawContours(contourMat, contours, -1, Scalar(0), CV_FILLED, 8, hierarchy);
	imshow("contours", contourMat);
	cout << "contours couts is" << contours.size() << endl;
#endif // FIND_CONTOURS

#ifdef CORNER_FAST
	//快速角点检测
	vector<KeyPoint> keypoints;
	Ptr<FastFeatureDetector> fast = FastFeatureDetector::create(10, true);
	fast->detect(input, keypoints);
	drawKeypoints(input, keypoints, input, Scalar::all(255), DrawMatchesFlags::DRAW_OVER_OUTIMG);
	imshow("corner", input);
#endif // !CORNER_FAST

#ifdef GODD_FEATURE 
	vector<Point2f> corners;
	goodFeaturesToTrack(input, corners, 200, 0.01, 150, Mat());
	for (int i = 0;i<corners.size();i++)
	{
		circle(srcMat, corners[i], 2, Scalar(0, 0, 255), 2);
	}
	imshow("good f", srcMat);
#endif // GODD_FEATURE 

#ifdef HARRIS

#endif // HARRIS


	waitKey();

	destroyAllWindows();
}

void removeDensePoints(Mat &input, Mat &output)
{
	Mat morMat, morMat2;
	Mat se = getStructuringElement(MORPH_RECT, Size(3, 3));
	
	morphologyEx(input, morMat, MORPH_DILATE, se);//膨胀
	morphologyEx(morMat, morMat2, MORPH_ERODE, se);//腐蚀

	output = morMat2 - input ;
	

	//morphologyEx(input, output, MORPH_TOPHAT, se);//顶帽

	imshow("morphologyEx", morMat2);
 
}



void CannyAndHough(Mat &input, Mat &output)
{
	int cannyThreshold = 80;
	float factor = 2.5;
	const int maxLinesNum = 25;//最多检测出的直线条数
	vector<Vec4i> lines;

	Mat cannyMat, tempMat1,tempMat2;
	Canny(input, cannyMat, cannyThreshold, cannyThreshold * factor);
	HoughLinesP(cannyMat, lines, 1, CV_PI / 180, 50, 100, 100);

	while (lines.size() >= maxLinesNum)
	{
		cannyThreshold += 2;
		Canny(input, cannyMat, cannyThreshold, cannyThreshold * factor);
		HoughLinesP(cannyMat, lines, 1, CV_PI / 180, 50, 100, 100);
	}
	//画直线
	for (size_t i = 0; i < lines.size(); i++)
	{
		Vec4i l = lines[i];
		line(output, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 60, 255), 1, CV_AA);
	}

	cout << "cannyThreshold1:" << cannyThreshold << endl;
}

void removeDenseRegions(Mat &input, Mat &output)
{
	if (input.channels() != 1) {
		return;
	}
	output = input.clone();
	int kSize = 5;
	int a = (kSize - 1) / 2;

	for (size_t i = a; i < input.rows- a; i++)
	{
		uchar *pCols = input.ptr<uchar>(i);
		for (size_t j = a; j < input.cols- a; j++)
		{
			//区域ROI 
			//Mat se = getStructuringElement(MORPH_RECT, Size(3, 3));
			Rect rect = Rect(j- a, i- a, kSize, kSize);
			Mat roi1 = input(rect);
			Mat roi2 = output(rect);
			int pixSum = cv::sum(roi1)[0];
			if (pixSum > 2500) {
				//过于稠密
				roi2.setTo(Scalar::all(0));
			}
		}

	}
}