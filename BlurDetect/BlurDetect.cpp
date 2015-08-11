#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include "cvaux.h"

using namespace cv;
using namespace std;

Mat image;
bool backprojMode = false;
bool selectObject = false;
int trackObject = 0;
bool showHist = true;
Point origin;
Rect selection;
int vmin = 10, vmax = 256, smin = 30;

IplImage* praw;
double VideoBlurDetect2(Mat frame)  
{  
    Mat gray;  
    cvtColor(frame,gray,CV_BGR2GRAY);  
    IplImage *img = &(IplImage(gray));  
    double temp = 0;  
    double DR = 0;  
    int i,j;//Ñ­»·±äÁ¿  
    int height=img->height;  
    int width=img->width;  
    int step=img->widthStep/sizeof(uchar);  
    uchar *data=(uchar*)img->imageData;  
    double num = width*height;  
  
    for(i=0;i<height;i++)  
    {  
        for(j=0;j<width;j++)  
        {  
            temp += sqrt((pow((double)(data[(i+1)*step+j]-data[i*step+j]),2) + pow((double)(data[i*step+j+1]-data[i*step+j]),2)));  
            temp += abs(data[(i+1)*step+j]-data[i*step+j])+abs(data[i*step+j+1]-data[i*step+j]);  
        }
    }
    DR = temp/num;  
    return DR;  
}



static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )
    {
        selection.x = MIN(x, origin.x);
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);
        selection.height = std::abs(y - origin.y);

        selection &= Rect(0, 0, image.cols, image.rows);
    }

    switch( event )
    {
    case CV_EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;
        break;
    case CV_EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;
        break;
    }
}

static void help()
{
    cout << "\nThis is a demo that shows Video Blur Detect\n"
            "The number shows if the image is clear enough.\n"
			"The bigger the number is,means the image is clearer otherwise it's more blur.\n"
            "This reads from video camera (0 by default, or the camera number the user enters\n"
            "Usage: \n"
            "   ./VideoBlurDetect [camera number]\n\n";

    cout << "\n\nHot keys: \n"
            "\tESC - quit the program\n"
            "\tp - pause video\n\n\n";
}

const char* keys =
{
    "{1|  | 0 | camera number}"
};

int main( int argc, const char** argv )
{
	bool paused = false;
    help();
	
    VideoCapture cap;

    CommandLineParser parser(argc, argv, keys);
    int camNum = parser.get<int>("1");
	int threshold =0;
    cap.open(camNum);
	cap.set(CV_CAP_PROP_FRAME_WIDTH,800);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,600);
    if( !cap.isOpened() )
    {
        help();
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
        //parser.printParams();
        return -1;
    }

    
    namedWindow( "VideoBlurDetect", CV_WINDOW_AUTOSIZE );
	//namedWindow("Example",CV_WINDOW_AUTOSIZE);
	createTrackbar( "Threshold", "VideoBlurDetect", &threshold, 25, 0 );
    Mat frame, hsv, hue, mask, backproj;
	//IplImage qImg,dst;
	//float re;
	double re2;
    

    for(;;)
    {
        if( !paused )
        {
            cap >> frame;
            if( frame.empty() )
                break;
        }

        frame.copyTo(image);
		re2 = VideoBlurDetect2(image);
		std::cout<<"          \r"<<re2;
		char temp[20];
		sprintf_s(temp,"%0.2f",re2);
		string numm = temp;
		putText( image, numm, Point( image.rows/1.01,image.cols/20),CV_FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 0) );

		if(re2>=(double)threshold)
			putText( image, "Pass", Point( image.rows/8,image.cols/10),CV_FONT_HERSHEY_DUPLEX, 2, Scalar(0, 255, 0) );
		else
			putText( image, "NG", Point( image.rows/8,image.cols/10),CV_FONT_HERSHEY_DUPLEX, 2, Scalar(0, 0, 255) );
        imshow( "VideoBlurDetect", image );
        

        char c = (char)waitKey(10);
        if( c == 27 )
            break;
        switch(c)
        {
        case 'p':
            paused = !paused;
            break;
        default:
            ;
        }
    }

    return 0;
}