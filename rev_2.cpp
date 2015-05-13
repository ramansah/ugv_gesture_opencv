#include<iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int slider_max = 255;
int hl,hh,sl,sh,vl,vh;

int main(void)
{
	VideoCapture cap(0);
	Mat img;
	int i;
	cap >> img;
	Mat op = img;
	hl = sl = vl = 0;
	hh = sh = vh = slider_max;

	namedWindow("Calib");

	createTrackbar("H1","Calib",&hl,slider_max);
	createTrackbar("H2","Calib",&hh,slider_max);
	createTrackbar("S1","Calib",&sl,slider_max);
	createTrackbar("S2","Calib",&sh,slider_max);
	createTrackbar("V1","Calib",&vl,slider_max);
	createTrackbar("V2","Calib",&vh,slider_max);

	Vec3b vec;

	while(1)
	{
		cap >> img;
		cvtColor(img,img,CV_BGR2YUV);
		inRange(img, Vec3b(hl, sl, vl), Vec3b(hh, sh, vh), img);

		erode(img,op,getStructuringElement(MORPH_ELLIPSE,Size(5,5)));
		imshow("Erode",op);
		
		waitKey(1);
	}
	return 0;
}