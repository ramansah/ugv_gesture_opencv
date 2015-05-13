#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2/ocl/ocl.hpp>
#include<cmath>
#include<WinSock2.h>
#include<ws2tcpip.h>

using namespace std;
using namespace cv::ocl;
using namespace cv;

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "27015"
#define DEFAULT_ADDRESS "192.168.137.1"

//	Max Value of Sliders ... changing comment
const int slider_max = 255;
//	Variables used in TrackBars		
int hl,hh,sl,sh,vl,vh,ero,dil;
//	Variables used in calculation of motor speed & direction
int lx,ly,rx,ry,tx,ty,ax,ay,min_x,min_y,bil,bih,bjl,bjh,Send;
float Max,dl,dr;

//	Main Function
int main(void)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);

	struct addrinfo *result = NULL,*ptr = NULL,hints;

	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	getaddrinfo(DEFAULT_ADDRESS, DEFAULT_PORT, &hints, &result);
	SOCKET ConnectSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr=result;

	// Create a SOCKET for connecting to server
	ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);

	// Connect to server.
	connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

	// Should really try the next address returned by getaddrinfo
	// if the connect call failed
	// But for this simple example we just free the resources
	// returned by getaddrinfo and print an error message
	freeaddrinfo(result);

	//	Camera Input Object
	VideoCapture cap(0);
	//	Mat Image format matrix used in code
	Mat img,capt,large,Small,op;
	//	Mat used for GPU optimization
	oclMat gmat1,gmat2;

	cap >> img;
	Rect window;
	int i,j;
	
	//	Default coordinates for 3-Points
	hl = sl = vl = 0;
	ero = dil = 1;
	hh = sh = vh = slider_max;

	//	Window for Trackbars
	namedWindow("Calib",CV_WINDOW_FREERATIO);
	//	Trackbars init...
	createTrackbar("H1","Calib",&hl,slider_max);
	createTrackbar("H2","Calib",&hh,slider_max);
	createTrackbar("S1","Calib",&sl,slider_max);
	createTrackbar("S2","Calib",&sh,slider_max);
	createTrackbar("V1","Calib",&vl,slider_max);
	createTrackbar("V2","Calib",&vh,slider_max);
	createTrackbar("Erode","Calib",&ero,10);
	createTrackbar("Dilate","Calib",&dil,10);

	//	Currently unused optimizations
	bil = bjl = 0;
	bih = img.rows;
	bjh = img.cols;

	while(1)
	{
		//	Input image frame from camera
		cap >> capt;
		Small = capt.clone();
		cvtColor(Small,Small,CV_BGR2HSV);

//		window = Rect(5,5,img.cols-10,img.rows-10);
//		large = img(window);

//		window = Rect(lx-5,ty-5,abs(rx-lx+10),abs(ty-ly+10));
//		Small = large(window);
				
		//	Separate 3-Points based on colour model
		inRange(Small, Vec3b(hl, sl, vl), Vec3b(hh, sh, vh), Small);

		//	Upload to GPU memory for GPU functions
		gmat1.upload(Small);
		//	GPU alternatives for erode & dilate
		ocl::erode(gmat1,gmat2,getStructuringElement(MORPH_ELLIPSE,Size(9,9)));
		//erode(Small,Small,getStructuringElement(MORPH_ELLIPSE,Size(ero,ero)));
		ocl::dilate(gmat2,gmat1,getStructuringElement(MORPH_ELLIPSE,Size(9,9)));
		//	Download to CPU Mat
		gmat1.download(Small);

		//	3-Points init...
		tx = ly = ry = rx = 0;
		ty = Small.rows;
		lx = Small.cols;
//		min_x = Small.cols;
//		min_y = Small.rows;
/*
		if(bil<0)			bil = 0;
		if(bih>Small.rows)	bih = Small.rows;
		if(bjl<0)			bjl = 0;
		if(bjh>Small.cols)	bjh = Small.cols;
*/
	//	3-Points Calculations
		for(i=0;i<Small.rows;++i)
			for(j=0;j<Small.cols;++j)
				if(Small.at<char>(i,j))
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
		ax = (lx+rx+tx)/3;
		ay = (ly+ry+ty)/3;

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
		cout<<"\n"<<dl<<" "<<dr<<" ";

		circle(capt,Point(lx,ly),20,Scalar(255,0,0),5);
		circle(capt,Point(rx,ry),20,Scalar(0,255,0),5);
		circle(capt,Point(tx,ty),20,Scalar(0,0,255),5);
		circle(capt,Point(lx,ty),20,Scalar(255,0,0),5);
		circle(capt,Point(rx,ty),20,Scalar(0,255,0),5);
		circle(capt,Point(ax,ay),sqrt(pow(lx-rx,2)+pow(ly-ry,2))/1.5,Scalar(255,255,255),5);
		line(capt,Point(lx,ly),Point(lx,ty),Scalar(255,255,255),5);
		line(capt,Point(rx,ry),Point(rx,ty),Scalar(255,255,255),5);

		imshow("Erode",Small);
		imshow("Camera",capt);

		if((dl<2.5)&&(dr<2.5))
			Send = 0;
		else if(dl<2.5)
			Send = 2;
		else if(dr<2.5)
			Send = 3;
		else
			Send = 1;
		cout<<Send;
		char Sendbuf[1];
		_itoa(Send,Sendbuf,10);
		// Send initial buffer
		send(ConnectSocket, Sendbuf, (int) strlen(Sendbuf),0);

		waitKey(1);
	}

	closesocket(ConnectSocket);
    WSACleanup();
	return 0;
}
