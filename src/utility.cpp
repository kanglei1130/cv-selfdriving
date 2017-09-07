/*
 * utility.cpp
 *
 *  Created on: Sep 6, 2017
 *      Author: lkang
 */
#include "utility.h"

namespace utility {

double blurDetection(cv::Mat& src) {
	cv::Mat gray;
	cvtColor(src, gray, CV_BGR2GRAY);
	cv::Mat lap;
	cv::Laplacian(gray, lap, CV_64F);
	cv::Scalar mu, sigma;
	cv::meanStdDev(lap, mu, sigma);
	double focusMeasure = sigma.val[0]*sigma.val[0];
	return focusMeasure;
}


int adjustTest(Mat& src)
{
	assert (!src.empty());
	Mat src_gray;
	int thresh = 100;
	cvtColor( src, src_gray, COLOR_BGR2GRAY );
	const char* source_window = "Source";
	namedWindow( source_window, WINDOW_AUTOSIZE );
	imshow( source_window, src );
	createTrackbar("Canny thresh:", "Source", &thresh, 255, thresh_callback, &src_gray);
	thresh_callback(thresh, &src_gray);
	waitKey(0);
	return(0);
}
void thresh_callback(int thresh, void* gray)
{
	Mat* src_gray = (Mat*)gray;
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Canny(*src_gray, canny_output, thresh, thresh*2);
	findContours( canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
	Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
	for( size_t i = 0; i< contours.size(); i++ ) {
		drawContours( drawing, contours, (int)i, Scalar(0,0,255), FILLED);
	}
	namedWindow( "Contours", WINDOW_AUTOSIZE );
	imshow( "Contours", drawing );
}



}



