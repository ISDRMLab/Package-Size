#include "stdafx.h"
#include "RoiSize.h"


RoiSize::RoiSize(Mat &mat)
{
	srcMat = mat.clone();
}


RoiSize::~RoiSize()
{
}

void RoiSize::doing() 
{
 
	Mat grayMat, cannyMat, warpMat;

	//边缘检测
	Canny(srcMat, cannyMat, 90, 200, 3, true);
	imshow("canny", cannyMat);

	//形态学操作

	//检索连通域
	Mat se = getStructuringElement(MORPH_RECT, Size(3, 3));
	//morphologyEx(cannyMat, cannyMat, MORPH_DILATE, se);
	//imshow("morphologyEx", cannyMat);



	//寻找轮廓
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(cannyMat, contours, hierarchy, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	//cout << "contours couts is" << contours.size() << endl;

	vector<Rect> boundRect(contours.size());

	for (int k = 0; k < contours.size(); k++)
	{
		Rect bomen = boundingRect(contours[k]);
		//rectangle(srcMat, bomen, Scalar(0, 0, 255), 1, 8, 0);

		//保留比例为1:1的矩形框
		double whrate = double(bomen.width) / double(bomen.height);
		//0.8--1.2之间的鲁棒
		if (bomen.width >= 5 && bomen.height >= 5 && contours[k].size() >= 10) {
			if (abs(whrate - 1) <= 0.1) {
				drawContours(srcMat, contours, k, Scalar(0, 0, 255), CV_FILLED, 8, hierarchy);
				getROISize(contours[k]);
				//rectangle(srcMat, bomen, Scalar(255, 0, 0), 1, 8, 0);
			}
		}

	}


	//drawContours(srcMat, contours, -1, Scalar(0,0,255), 1);

	imshow("result22", srcMat);

	
	
}


void RoiSize::getROISize(vector<Point> &contour)
{
	CvPoint2D32f rectpoint[4];
	CvBox2D rect = minAreaRect(Mat(contour));

	CvSize2D32f size = rect.size;
	m_MarkerWidth = size.width;
	m_MarkerHeight = size.height;

}


void RoiSize::printSizeInfo() 
{
	double realWidth = srcMat.cols / m_MarkerWidth * 3;//cm
	double realHeight = srcMat.rows / m_MarkerHeight * 3;//cm

	cout << "marker pixel\n" << "width:" << m_MarkerWidth  << "height:" << m_MarkerHeight << endl;
	cout << "box pixel\n" << "width:" << srcMat.cols << "height:" << srcMat.rows << endl;
	cout << "box real size\n" << "width:" << realWidth << "height:" << realHeight << endl;
	

	waitKey();
}



