#pragma once

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;


class RoiSize
{
public:
 
	RoiSize(Mat & mat);
	~RoiSize();
	void doing();
	void printSizeInfo();


private:

	Mat srcMat;
	double m_MarkerWidth;
	double m_MarkerHeight;

  
	void getROISize(vector<Point>& contour);
	
};



