/*
 * image_test.cpp
 *
 *  Created on: Apr 19, 2017
 *      Author: lkang
 */

#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "headers.h"
#include "lane_marker_detector.h"
#include "receiver_socket.h"
#include "data_pool.h"
#include "udp_socket.h"
#include "car_control.h"







void thresh_callback(int, void* );

Mat src, src_gray;
int thresh = 100;

int adjustTest()
{
	string imagepath = "/home/lkang/Desktop/caltech-lanes/cordova1/";
	cv::String in = string(imagepath + string("f00185.png"));
	src = imread(in, IMREAD_COLOR);
	if (src.empty()) {
		cerr << "No image supplied ..." << endl;
		return -1;
	}
	cvtColor( src, src_gray, COLOR_BGR2GRAY );
	const char* source_window = "Source";
	namedWindow( source_window, WINDOW_AUTOSIZE );
	imshow( source_window, src );
	createTrackbar("Canny thresh:", "Source", &thresh, 255, thresh_callback );
	thresh_callback( 0, 0 );
	waitKey(0);
	return(0);
}
void thresh_callback(int, void* )
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Canny( src_gray, canny_output, thresh, thresh*2);
	findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
	Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );

	for( size_t i = 0; i< contours.size(); i++ ) {
		//Rect _boundingRect = boundingRect( contours[i] );
		//Scalar color = mean( src( _boundingRect ) );
		drawContours( drawing, contours, (int)i, Scalar(0,0,255), FILLED);

	}
	namedWindow( "Contours", WINDOW_AUTOSIZE );
	imshow( "Contours", drawing );
}


