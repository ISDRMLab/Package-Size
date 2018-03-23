#include "stdafx.h"
#include "ImgRestore.h"

#include <numeric>

struct ImgRestore::Ximpl
{
	enum imageStyle { normal, leanToRight, leanToLeft };
	int cannyThreshold;
	Mat srcImage, dstImage;
	Mat midImage, edgeDetect;
	Mat afterEnhance;
	vector<Point> resultPointsByEdge;
	vector<Point> resultPointsByContours;
	vector<Vec4i> lines;
	vector<Point> axisSort(const vector<Vec4i>& lines);
	vector<Point> pointsFilter(const vector<Point>& candidates);//�Ը��߶���ʼ�����
	vector<Point> findCrossPoint(const vector<Vec4i>& lines);   //����ֱ����ȡȷ������4������
	vector<Point> axisSort(const vector<vector<Point>>& contours);
	vector<Point> findCrossPoint(const vector<vector<Point>>& contours);//����������ȡ4������
	void loadImage(const string& name);//����ͼ��
	void doEdgeDetect();//��Ե��ȡ
	void doFindcontours();//������ȡ
	void doAffineTransform();//͸�ӱ任
	void dstImageEnhance();
 
	//ͼǿ��ǿ
};
ImgRestore::ImgRestore() : pImpl(new Ximpl()) {}
ImgRestore::ImgRestore(const ImgRestore& other) : pImpl(new Ximpl(*other.pImpl)) {}
ImgRestore& ImgRestore::operator=(ImgRestore other)
{
	std::swap(other.pImpl, this->pImpl);
	return *this;
}
ImgRestore::~ImgRestore()
{
	delete pImpl;
	pImpl = nullptr;
}
//ͨ��(�������޵�)��Ե��ȡ�õ��ĸ�����
vector<Point> ImgRestore::Ximpl::pointsFilter(const vector<Point>& candidate)
{
	vector<Point> candidates(candidate);
	vector<Point> filter(candidate);
	for (auto i = candidates.begin(); i != candidates.end();)
		for (auto j = filter.begin(); j != filter.end(); ++j)
		{
			if (abs((*i).x - (*j).x) < 5 && abs((*i).y - (*j).y) < 5 && abs((*i).x - (*j).x) > 0 && abs((*i).y - (*j).y) > 0)
				i = filter.erase(i);
			else
				++i;
		}
	return filter;
}
vector<Point> ImgRestore::Ximpl::axisSort(const vector<Vec4i>& lines)
{
	vector<Point> points(lines.size() * 2);//�����߶ε���ʼ��
	for (size_t i = 0; i < lines.size(); ++i)//��Vec4iתΪpoint
	{
		points[i * 2].x = lines[i][0];
		points[i * 2].y = lines[i][1];
		points[i * 2 + 1].x = lines[i][2];
		points[i * 2 + 1].y = lines[i][3];
	}
	points = this->pointsFilter(points);//���Լ�����һ��
										/*for (auto i : points)
										cout << i.x << " " << i.y << endl;*/
	sort(points.begin(), points.end(), CmpDistanceToZero());
	return points;
}
void ImgRestore::Ximpl::doEdgeDetect()
{
	this->cannyThreshold = 80;
	float factor = 2.5;
	const int maxLinesNum = 20;//��������ֱ������
	Canny(this->srcImage, this->midImage, this->cannyThreshold, this->cannyThreshold * factor);
	threshold(this->midImage, this->midImage, 128, 255, THRESH_BINARY);
	cvtColor(this->midImage, this->edgeDetect, CV_GRAY2RGB);
	HoughLinesP(this->midImage, this->lines, 1, CV_PI / 180, 50, 100, 100);
	while (this->lines.size() >= maxLinesNum)
	{
		this->cannyThreshold += 2;
		Canny(this->srcImage, this->midImage, this->cannyThreshold, this->cannyThreshold * factor);
		threshold(this->midImage, this->midImage, 128, 255, THRESH_BINARY);
		cvtColor(this->midImage, this->edgeDetect, CV_GRAY2RGB);
		HoughLinesP(this->midImage, this->lines, 1, CV_PI / 180, 50, 100, 100);
	}
	cout << "cannyThreshold1:" << this->cannyThreshold << endl;
	Canny(this->srcImage, this->midImage, this->cannyThreshold, this->cannyThreshold * factor);
	threshold(this->midImage, this->midImage, 128, 255, THRESH_BINARY);
	cvtColor(this->midImage, this->edgeDetect, CV_GRAY2RGB);
	HoughLinesP(this->midImage, this->lines, 1, CV_PI / 180, 50, 100, 100);
	const int imageRow = this->midImage.rows;
	const int imageCol = this->midImage.cols;
	lines.erase(remove_if(lines.begin(), lines.end(), IsCloseToEdge()), lines.end());
	for (size_t i = 0; i < this->lines.size(); i++)
	{
		Vec4i l = this->lines[i];
		line(this->edgeDetect, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(186, 88, 255), 1, CV_AA);
	}
	this->findCrossPoint(this->lines);
	/*for (size_t i = 0; i < lines.size(); ++i)
	cout << lines[i] << endl;*/
	imshow("����Ե��ȡЧ��ͼ��", this->edgeDetect);
}
vector<Point> ImgRestore::Ximpl::findCrossPoint(const vector<Vec4i>& lines)//ͨ��ֱ���ҵ�
{
	int rightTopFlag = 0;
	int leftDownFlag = 0;
	int diagLength = 0;//�Խ��߳���
	vector<Point> temp = this->axisSort(lines);
	Point leftTop, rightDown;//���Ϻ����¿���ֱ���ж�
	vector<Point> rightTop(temp.size());
	vector<Point> leftDown(temp.size());//���º������ж������ܷ���
	//����Ƭ���ԣ�һ�������Ͻ���ԭ����������½���ԭ����Զ
	leftTop.x = temp[0].x;
	leftTop.y = temp[0].y;
	rightDown.x = temp[temp.size() - 1].x;
	rightDown.y = temp[temp.size() - 1].y;
	for (auto & i : temp)
		if (i.x > leftTop.x && i.y < rightDown.y)
			rightTop.push_back(i);
	for (auto & i : temp)
		if (i.y > leftTop.y && i.x < rightDown.x)
			leftDown.push_back(i);
	diagLength = (leftTop.x - rightDown.x) * (leftTop.x - rightDown.x) + (leftTop.y - rightDown.y) * (leftTop.y - rightDown.y);
	rightTop.erase(remove(rightTop.begin(), rightTop.end(), Point(0, 0)), rightTop.end());
	leftDown.erase(remove(leftDown.begin(), leftDown.end(), Point(0, 0)), leftDown.end());
	//ɾ����ͼ�����Լ��������������Ե�Ӱ��ĵ�
	for (auto i = rightTop.begin(); i != rightTop.end();)
	{
		if (((*i).x - leftTop.x) * ((*i).x - leftTop.x) + ((*i).y - leftTop.y) * ((*i).y - leftTop.y) < diagLength / 8)
			i = rightTop.erase(i);
		else
			++i;
	}
	/*for (auto i : rightTop)
	cout << "���ϻ��У�" << i.x << " " << i.y << endl;*/
	int maxDistance = (rightTop[0].x - leftDown[0].x) * (rightTop[0].x - leftDown[0].x) + (rightTop[0].y - leftDown[0].y) * (rightTop[0].y - leftDown[0].y);
	for (size_t i = 0; i < rightTop.size(); ++i)
		for (size_t j = 0; j < leftDown.size(); ++j)
			if ((rightTop[i].x - leftDown[j].x) * (rightTop[i].x - leftDown[j].x) + (rightTop[i].y - leftDown[j].y) * (rightTop[i].y - leftDown[j].y) > maxDistance)
			{
				maxDistance = (rightTop[i].x - leftDown[j].x) * (rightTop[i].x - leftDown[j].x) + (rightTop[i].y - leftDown[j].y) * (rightTop[i].y - leftDown[j].y);
				rightTopFlag = i;
				leftDownFlag = j;
			}
	/*cout << rightTop[rightTopFlag].x << " " << rightTop[rightTopFlag].y << endl;
	cout << leftDown[leftDownFlag].x << " " << leftDown[leftDownFlag].y << endl;*/
	/*for (auto i : rightTop)
	cout << i.x << " " << i.y << endl;*/
	/*for (auto i : leftdown)
	cout << i.x << " " << i.y << endl;*/
	this->resultPointsByEdge.push_back(leftTop);
	this->resultPointsByEdge.push_back(rightTop[rightTopFlag]);
	this->resultPointsByEdge.push_back(leftDown[leftDownFlag]);
	this->resultPointsByEdge.push_back(rightDown);
	return this->resultPointsByEdge;
}
//ͨ��������ȡ�õ��ĸ�����
vector<Point> ImgRestore::Ximpl::axisSort(const vector<vector<Point>>& contours)
{
	vector<Point> points(contours.size() * contours[0].size());
	for (auto i : contours)
		for (auto j : i)
			points.push_back(j);
	points = this->pointsFilter(points);//���Լ�����һ��
	points.erase(remove(points.begin(), points.end(), Point(0, 0)), points.end());
	sort(points.begin(), points.end(), CmpDistanceToZero());
	return points;
}
void ImgRestore::Ximpl::doFindcontours()
{
	const float approachMaxThreshold = 15;
	Mat result(this->midImage.size(), CV_8U, Scalar(0));
	vector<vector<Point>> contours;
	findContours(this->midImage, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	sort(contours.begin(), contours.end(), CmpContoursSize());
	for (size_t i = 0; i < 5; ++i)//ȥ������ͼ���Ե������
	{
		size_t j = contours[i].size();
		if (contours[i][j / 5].y - 0 < approachMaxThreshold || this->midImage.cols - contours[i][j / 2].x < approachMaxThreshold)
			contours.erase(remove(contours.begin(), contours.end(), contours[i]), contours.end());
	}
	vector<vector<Point>> biggestContours;
	for (size_t i = 0; i < 3; ++i)
		biggestContours.push_back(contours[i]);
	drawContours(result, biggestContours, -1, Scalar(255), 2);
	imshow("������ȡ", result);
	this->findCrossPoint(biggestContours);
}
vector<Point> ImgRestore::Ximpl::findCrossPoint(const vector<vector<Point>>& contours)//ͨ�������ҵ�
{
	int imageState;//ͼƬ�����б
	vector<Point> temp = this->axisSort(contours);
	Point leftTop, trueRightTop, trueLeftDown, rightDown;//���Ϻ����¿���ֱ���ж�
	vector<Point> rightTop(temp.size());
	vector<Point> leftDown(temp.size());//���º������ж������ܷ���
	//����Ƭ���ԣ�һ�������Ͻ���ԭ����������½���ԭ����Զ
	leftTop.x = temp[0].x-5;
	leftTop.y = temp[0].y-5;
	rightDown.x = temp[temp.size() - 1].x+5;
	rightDown.y = temp[temp.size() - 1].y+5;
	for (auto & i : temp)
		if (i.x > leftTop.x && i.y < rightDown.y)
			rightTop.push_back(i);
	for (auto & i : temp)
		if (i.y > leftTop.y && i.x < rightDown.x)
			leftDown.push_back(i);
	if (rightTop.end() == find_if(rightTop.begin(), rightTop.end(), [leftTop, rightTop](Point p) {return p.y < leftTop.y; }))
		imageState = imageStyle::leanToRight;//����������ϵ��yֵ�� > ���ϵ��yֵ ��˵��ͼ��������б
	else
		imageState = imageStyle::leanToLeft;
	if (imageState == imageStyle::leanToRight)//������б
	{
		sort(rightTop.begin(), rightTop.end(), [rightTop](Point p1, Point p2) {return p1.x > p2.x; });//���������ϵ㰴Xֵ����X���ľ������������ϵ�
		rightTop.erase(remove(rightTop.begin(), rightTop.end(), Point(0, 0)), rightTop.end());
		trueRightTop = rightTop[0];
		sort(leftDown.begin(), leftDown.end(), [leftDown](Point p1, Point p2) {return p1.x < p2.x; });//���������µ㰴Xֵ����X��С�ľ������������µ�
		leftDown.erase(remove(leftDown.begin(), leftDown.end(), Point(0, 0)), leftDown.end());
		trueLeftDown = leftDown[0];
	}
	else //������б
	{
		sort(rightTop.begin(), rightTop.end(), [rightTop](Point p1, Point p2) {return p1.y < p2.y; });//���������ϵ㰴Yֵ����Y��С�ľ������������ϵ�
		rightTop.erase(remove(rightTop.begin(), rightTop.end(), Point(0, 0)), rightTop.end());
		trueRightTop = rightTop[0];
		sort(leftDown.begin(), leftDown.end(), [leftDown](Point p1, Point p2) {return p1.y > p2.y; });//���������µ㰴Yֵ����Y���ľ������������µ�
		leftDown.erase(remove(leftDown.begin(), leftDown.end(), Point(0, 0)), leftDown.end());
		trueLeftDown = leftDown[0];
	}
	this->resultPointsByContours.push_back(leftTop);
	this->resultPointsByContours.push_back(trueRightTop);
	this->resultPointsByContours.push_back(trueLeftDown);
	this->resultPointsByContours.push_back(rightDown);
	return this->resultPointsByContours;
}
void ImgRestore::Ximpl::loadImage(const string& name)
{
	srcImage = imread(name, 1);
	if (!srcImage.data)
	{
		cout << "��ȡͼƬ������ȷ��Ŀ¼���Ƿ���imread����ָ����ͼƬ����" << endl;
	}
	imshow(WINDOW_NAME1, this->srcImage);
}
void ImgRestore::Ximpl::doAffineTransform()
{
	this->doEdgeDetect();//����任ǰ�ȱ�Ե��ֱ����ȡ
	//this->doFindcontours();//��ȡ����
	Point2f _srcTriangle[4];
	Point2f _dstTriangle[4];
	vector<Point2f>srcTriangle(_srcTriangle, _srcTriangle + 4);
	vector<Point2f>dstTriangle(_dstTriangle, _dstTriangle + 4);
	const int leftTopX = (this->resultPointsByEdge[0].x + this->resultPointsByEdge[0].x) / 2;
	const int leftTopY = (this->resultPointsByEdge[0].y + this->resultPointsByEdge[0].y) / 2;
	const int rightTopX = (this->resultPointsByEdge[1].x + this->resultPointsByEdge[1].x) / 2;
	const int rightTopY = (this->resultPointsByEdge[1].y + this->resultPointsByEdge[1].y) / 2;
	const int leftDownX = (this->resultPointsByEdge[2].x + this->resultPointsByEdge[2].x) / 2;
	const int leftDownY = (this->resultPointsByEdge[2].y + this->resultPointsByEdge[2].y) / 2;
	const int rightDownX = (this->resultPointsByEdge[3].x + this->resultPointsByEdge[3].x) / 2;
	const int rightDownY = (this->resultPointsByEdge[3].y + this->resultPointsByEdge[3].y) / 2;
	cout << leftTopX << " " << leftTopY << endl;
	cout << rightTopX << " " << rightTopY << endl;
	cout << leftDownX << " " << leftDownY << endl;
	cout << rightDownX << " " << rightDownY << endl;
	int newWidth = 0;
	int newHeight = 0;
	newWidth = sqrt((leftTopX - rightTopX) * (leftTopX - rightTopX) + (leftTopY - rightTopY) * (leftTopY - rightTopY));
	newHeight = sqrt((leftTopX - leftDownX) * (leftTopX - leftDownX) + (leftTopY - leftDownY) * (leftTopY - leftDownY));
	this->dstImage = Mat::zeros(newHeight, newWidth, this->srcImage.type());
	srcTriangle[0] = Point2f(leftTopX, leftTopY);
	srcTriangle[1] = Point2f(rightTopX, rightTopY);
	srcTriangle[2] = Point2f(leftDownX, leftDownY);
	srcTriangle[3] = Point2f(rightDownX, rightDownY);
	dstTriangle[0] = Point2f(0, 0);
	dstTriangle[1] = Point2f(newWidth, 0);
	dstTriangle[2] = Point2f(0, newHeight);
	dstTriangle[3] = Point2f(newWidth, newHeight);
	Mat m1 = Mat(srcTriangle);
	Mat m2 = Mat(dstTriangle);
	Mat status;
	Mat h = findHomography(m1, m2, status, 0, 3);
	perspectiveTransform(srcTriangle, dstTriangle, h);
	warpPerspective(this->srcImage, this->dstImage, h, this->dstImage.size());
	imshow(WINDOW_NAME2, this->dstImage);
	imwrite("result.jpg", this->dstImage);
 
}
void ImgRestore::Ximpl::dstImageEnhance()
{
	Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);
	//Mat kernel = (Mat_<float>(5, 5) << 0, -1, 0, -1,0,-1,0,-1,0,-1,0,-1,14,-1,0, -1, 0, -1, 0,-1,0,-1,0,-1,0);
	/*Mat kernel(3, 3, CV_32F, Scalar(-1));
	kernel.at<float>(1, 1) =9;*/
	//filter2D(this->dstImage, this->afterEnhance, this->dstImage.depth(), kernel);
	filter2D(this->srcImage, this->afterEnhance, this->srcImage.depth(), kernel);
	//imshow(WINDOW_NAME3, this->afterEnhance);
	//imwrite("result.jpg", afterEnhance);
}
void ImgRestore::imageRestoreAndEnhance(const string name)
{
	//ִ��˳�򣺼���ԭͼ����������ǿ
	this->pImpl->loadImage(name);
	this->pImpl->doAffineTransform();
	this->pImpl->dstImageEnhance();
}

Mat & ImgRestore::getDstMat()
{
	// TODO: �ڴ˴����� return ���
	return this->pImpl->dstImage;
}
