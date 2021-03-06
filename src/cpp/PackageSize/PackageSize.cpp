// PackageSize.cpp: 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "ImgRestore.h"
#include "RoiSize.h"

#include <iostream>
#include <vector>


#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>

using namespace std;
using namespace cv;



int main()
{
	//先进行图像的校正 得到reasult.jgp 然后计算大小

	//步骤1
	const string testName = "6.jpg";
	ImgRestore imgrestore;
	imgrestore.imageRestoreAndEnhance(testName);

	//步骤2
	Mat restoreMat = imgrestore.getDstMat();
	RoiSize roi(restoreMat);

	roi.doing();
	roi.printSizeInfo();

	waitKey();

	destroyAllWindows();

	return 0;
}

