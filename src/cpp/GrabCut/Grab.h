#pragma once

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <ctype.h>


using namespace std;
using namespace cv;


class Grab
{
public:
	Grab();
	~Grab();

public:
	enum { NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	void reset();
	void setImageAndWinName(const Mat& _image, const string& _winName);
	void showImage() const;
	void mouseClick(int event, int x, int y, int flags, void* param);
	int nextIter();
	 
	int getIterCount() const { return iterCount; }


private:
	void setRectInMask();
	void setLblsInMask(int flags, Point p, bool isPr);
	const string* winName;
	const Mat* image;
	Mat mask;
	Mat bgdModel, fgdModel;
	
	uchar rectState, lblsState, prLblsState;
	bool isInitialized;
	Rect rect;
	vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
	int iterCount;
};

