#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <math.h>
#include <cmath>
#include <iostream>
#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <time.h>


using namespace cv;
using namespace std;

string imagepath = "/home/lkang/Desktop/caltech-lanes/cordova1/";


void laneMarkerDetector(Mat& input, Mat& src, Mat& temp);
const Vec3b kLaneWhite = Vec3b(255, 255, 255);
const Vec3b kLaneYellow = Vec3b(150, 200, 255);


int main( int, char** argv )
{
	string in = string(imagepath + string("f00185.png"));
	//string in = string(imagepath + string("f00050.png"));

	Mat src = imread(in, IMREAD_COLOR);
	Mat src_gray;
	cvtColor( src, src_gray, COLOR_BGR2GRAY );
	Mat canny_output;
	Canny( src_gray, canny_output, 100, 200);
	namedWindow( "canny", WINDOW_AUTOSIZE );

	Mat temp = Mat::zeros(src.rows, src.cols, src.type());
	laneMarkerDetector(canny_output, src, temp);

	imshow( "canny", canny_output);
	imshow( "original", src);
	imshow( "test", temp);
	waitKey(0);
	return(0);
}


double colorDiff(const Vec3b& color, const Vec3b& another) {
	double sum = 0;
	for(int i = 0; i < 3; ++i) {
		sum += pow((int)color.val[i] - (int)another.val[i], 2.0);
	}
	double diff = sqrt(sum);
	return diff;
}

void laneMarkerDetector(Mat& img, Mat& src, Mat& temp)
{
	int cols = img.cols;
	int rows = img.rows;
	int start = rows/3;
	int end = rows * 7/10;

	cout<<start<<","<<end<<endl;
	//
	for(int x = 0; x < rows; ++x) {
		for(int y = 0; y < cols; ++y) {
			//black outside the are of interest
			if(x < start || x > end) {
				img.at<unsigned char>(x, y) = 0;
			} else {
				//draw a white line in the middle, as the forwarding direction of the car
				if(y == cols/2) {
					img.at<unsigned char>(x, y) = 255;
				}
			}
		}
	}

	//identify the lane markers, rows from bottom to top
	//columns from the middle to sides
	for(int x = end; x >= start; --x) {
		//left marker
		for(int i = cols/2 - 1; i >=0; --i) {
			unsigned char color = img.at<unsigned char>(x, i);
			if(color == 255) {
				double minDiff = 10000;
				for(int j = 0; j < 3; ++j) {
					Vec3b color = src.at<Vec3b>(x, i - j - 1);
					minDiff = min(minDiff, colorDiff(color, kLaneYellow));
					cout<<color<<endl;
				}
				if(minDiff < 100) {
					temp.at<Vec3b>(x, i) = src.at<Vec3b>(x, i);
				}
				cout<<x<<","<<minDiff<<endl;
				break;
			}
		}
		//right marker
		for(int i = cols/2 + 1; i < cols; ++i) {
			unsigned char color = img.at<unsigned char>(x, i);
			if(color == 255) {
				double minDiff = 10000;
				for(int j = 0; j < 3; ++j) {
					Vec3b color = src.at<Vec3b>(x, i + j + 1);
					minDiff = min(minDiff, colorDiff(color, kLaneWhite));
				}
				if(minDiff < 100) {
					temp.at<Vec3b>(x, i) = src.at<Vec3b>(x, i);
				}
				break;
			}
		}
	}
}

/*

Mat src, src_gray;
int thresh = 100;
void thresh_callback(int, void* );
int main( int, char** argv )
{
	string in = string(imagepath + string("f00185.png"));
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
*/
