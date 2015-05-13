#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/ocl/ocl.hpp>
#include<cmath>

using namespace std;
using namespace cv::ocl;
using namespace cv;

const int slider_max = 255;
int hl,hh,sl,sh,vl,vh,ero,dil;
int lx,ly,rx,ry,tx,ty,ax,ay,min_x,min_y,bil,bih,bjl,bjh;
float Max,dl,dr;

int main(void)
{
	VideoCapture cap(0);
	Mat img,capt,large,small,op;
	oclMat gmat1,gmat2;

	cap >> img;
	Rect window;
	int i,j;
	hl = sl = vl = 0;
	ero = dil = 1;
	hh = sh = vh = slider_max;

	namedWindow("Calib",CV_WINDOW_FREERATIO);

	createTrackbar("H1","Calib",&hl,slider_max);
	createTrackbar("H2","Calib",&hh,slider_max);
	createTrackbar("S1","Calib",&sl,slider_max);
	createTrackbar("S2","Calib",&sh,slider_max);
	createTrackbar("V1","Calib",&vl,slider_max);
	createTrackbar("V2","Calib",&vh,slider_max);
	createTrackbar("Erode","Calib",&ero,10);
	createTrackbar("Dilate","Calib",&dil,10);

	bil = bjl = 0;
	bih = img.rows;
	bjh = img.cols;

	while(1)
	{
		cap >> capt;
		small = capt.clone();
		cvtColor(small,small,CV_BGR2HSV);

//		window = Rect(5,5,img.cols-10,img.rows-10);
//		large = img(window);

//		window = Rect(lx-5,ty-5,abs(rx-lx+10),abs(ty-ly+10));
//		small = large(window);
				
		inRange(small, Vec3b(hl, sl, vl), Vec3b(hh, sh, vh), small);

		gmat1.upload(small);
		ocl::erode(gmat1,gmat2,getStructuringElement(MORPH_ELLIPSE,Size(9,9)));
		//erode(small,small,getStructuringElement(MORPH_ELLIPSE,Size(ero,ero)));
		ocl::dilate(gmat2,gmat1,getStructuringElement(MORPH_ELLIPSE,Size(9,9)));
		gmat1.download(small);

		tx = ly = ry = rx = 0;
		ty = small.rows;
		lx = small.cols;
//		min_x = small.cols;
//		min_y = small.rows;
/*
		if(bil<0)			bil = 0;
		if(bih>small.rows)	bih = small.rows;
		if(bjl<0)			bjl = 0;
		if(bjh>small.cols)	bjh = small.cols;
*/
		for(i=0;i<small.rows;++i)
			for(j=0;j<small.cols;++j)
				if(small.at<char>(i,j))
				{
					if(i<ty)	
					{
						tx = j;
						ty = i;
					}
					if(j<lx)
					{
						lx = j;
						ly = i;
					}
					if(j>rx)
					{
						rx = j;
						ry = i;
					}
				}
/*		if(lx<min_x)	min_x = lx;
		if(rx<min_x)	min_x = rx;
		if(tx<min_x)	min_x = tx;

		if(ly<min_y)	min_y = ly;
		if(ry<min_y)	min_y = ry;
		if(ty<min_y)	min_y = ty;
*/
/*		lx -= min_x;
		rx -= min_x;
		tx -= min_x;

		ly -= min_y;
		ry -= min_y;
		ty -= min_y;
*/
//		ax = (lx+rx+tx)/3;
//		ay = (ly+ry+ty)/3;

/*		bil = lx-5;
		bih = rx+5;
		bjl = ty-5;
		bjh = ry+5;
*/

		dl = abs(ty-ly);
		dr = abs(ty-ry);
		Max = sqrt(pow(lx-rx,2)+pow(ly-ry,2));
		dl = dl/Max*8.25;
		dr = dr/Max*8.25;
		if(dl>3.3)	dl=3.3;
		if(dr>3.3)	dr=3.3;
		cout<<dl<<" "<<dr<<" "<<rx-lx<<"\n";

		circle(capt,Point(lx,ly),20,Scalar(255,0,0),5);
		circle(capt,Point(rx,ry),20,Scalar(0,255,0),5);
		circle(capt,Point(tx,ty),20,Scalar(0,0,255),5);
		circle(capt,Point(lx,ty),20,Scalar(255,0,0),5);
		circle(capt,Point(rx,ty),20,Scalar(0,255,0),5);
		line(capt,Point(lx,ly),Point(lx,ty),Scalar(255,0,0),5);
		line(capt,Point(rx,ry),Point(rx,ty),Scalar(0,255,0),5);

		imshow("Erode",small);
		imshow("Camera",capt);

		waitKey(1);
	}
	return 0;
}