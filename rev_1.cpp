#include<iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int DELAY_BLUR = 10;

const int slider_max = 255;
int r,g,b,ero,dil,thresh=200;
char ch;

Mat img,diff,back,op;

int main(void)
{
	VideoCapture cap(0);
	
	r = b = g = 0;
	ero = 2;
	dil = 2;

	namedWindow("Calib");

	createTrackbar("BLUE","Calib",&b,slider_max);
	createTrackbar("GREEN","Calib",&g,slider_max);
	createTrackbar("RED","Calib",&r,slider_max);
	createTrackbar("ERODE","Calib",&ero,10);
	createTrackbar("DILATE","Calib",&dil,10);
	createTrackbar("CORNER","Calib",&thresh,slider_max);

	Vec3b vec;

	cap >> img;
	cvtColor(img,img,CV_BGR2HSV);
	back = img.clone();

	while(1)
	{
		cap >> img;
		cvtColor(img,img,CV_BGR2HSV);

		diff = abs(img-back);

		erode(diff,diff,getStructuringElement(MORPH_ELLIPSE,Size(ero,ero)));
		dilate(diff,diff,getStructuringElement(MORPH_ELLIPSE,Size(dil,dil)));
		
		inRange(diff,Vec3b(b,g,r),Vec3b(255,255,255),op);
	//	imshow("Diff",op);
		
		imshow("Erode&Dilate",op);
		
		switch(waitKey(1))
		{
		case 'b'	:	back = img.clone();
						break;
		}
	}
	return 0;
}

