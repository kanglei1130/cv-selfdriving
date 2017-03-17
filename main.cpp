#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


#include "lane_marker_detector.h"
#include "receiver_socket.h"
#include "headers.h"

string imagepath = "/home/lkang/Desktop/caltech-lanes/cordova1/";


void inversePerspectiveMapping(Mat& input, Mat& output);
void publish_points(Mat& img, Points& points, const Vec3b& color);
int processImage(Mat src, Mat& gray);
void test(Mat src, Mat& gray);

void startUdpServer();

void thresh_callback(int, void* );
int adjustTest();
void processVideo();
void blurDetection(Mat& frame);

int main( int, char** argv )
{

	//processVideo();
	//adjustTest();

	//startUdpServer();

	/*
	Mat ipm = Mat::zeros(src.rows, src.cols, src.type());
	inversePerspectiveMapping(canny_output, ipm);
	imshow( "ipm", ipm);
	*/


	string in = string("/home/lkang/Desktop/blur.jpg");
	Mat src = imread(in, IMREAD_COLOR);
	Mat gray;
	cvtColor(src, gray, CV_BGR2GRAY);
	blurDetection(gray);

	//processVideo();
	return(0);
}


void blurDetection(Mat& frame) {
	Mat laplace;
	Laplacian(frame, laplace, CV_16S, 3);
	Scalar     mean;
	Scalar     stddev;
	meanStdDev (laplace, mean, stddev);
	cout<<"blurness:"<<stddev<<endl;
}


void startUdpServer() {
	const int kUdpPort = 55555;
	ReceiverSocket socket(kUdpPort);
	if (socket.BindSocketToListen()) {
		std::cout << "Listening on port " << kUdpPort << "." << std::endl;
	    while (true) {  // TODO: break out cleanly when done.
	    	std::vector<unsigned char> data = socket.GetPacket();
	    	cv::Mat frame = cv::imdecode(data, cv::IMREAD_COLOR);
	    	if (!frame.data) {
	    		std::cerr << "Could not decode image data." << std::endl;
	    		return;
	    	}
	    	cv::imshow("streaming video", frame);
	    	cv::waitKey(15);
	    }
	}
}


void processVideo() {
	VideoCapture cap("/home/lkang/Desktop/test.mp4"); // open the default camera
	if(!cap.isOpened()) { // check if we succeeded
		cout<<"not able to open"<<endl;
		return;
	}
	Mat gray;

	int counter = 0;
	for(;;) {
		Mat frame;
		cap >> frame; // get a new frame from camera
		if(frame.empty()) {
			break;
		}

		//cvtColor(frame, edges, CV_BGR2GRAY);
		//GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
		//Canny(edges, edges, 200, 400);

		int num = processImage(frame, gray);

		counter++;
		cout<<counter<<","<<num<<endl;
		/*
		if(num==0) {
			imshow("frame", frame);
			imshow("gray", gray);
			waitKey(0);
			break;
		}
		*/
		cvtColor(frame, gray, CV_BGR2GRAY);
		blurDetection(gray);
		sleep(1);

	}
	cout<<counter<<endl;
}


void test(Mat src, Mat& gray) {
	cvtColor( src, gray, COLOR_BGR2GRAY );
	Mat canny_output;
	Canny(gray, canny_output, 100, 200);
	LaneMarkerDetector detector(src);
	Mat temp = Mat::zeros(src.rows, src.cols, src.type());

	detector.laneMarkerDetector(canny_output, src, temp);


	Point center(src.cols/2, src.rows*4/5);
	temp.at<Vec3b>(center.y, center.x) = kLaneRed;

	cvtColor(canny_output, gray, COLOR_GRAY2BGR);

	for(int i = 0; i < detector.left_lanes_.size(); ++i) {
		Points left = detector.left_lanes_.at(i);
		cout<<left.size()<<endl;
		publish_points(gray, left, kLaneRed);
	}
	for(int i = 0; i < detector.right_lanes_.size(); ++i) {
		Points right = detector.right_lanes_.at(i);
		cout<<right.size();
		publish_points(gray, right, kLaneRed);
	}


	imshow( "original", src);
	imshow( "canny", gray);
	waitKey(0);

}

const int kCannyThreshold = 200;
int processImage(Mat src, Mat& gray) {
	cvtColor( src, gray, COLOR_BGR2GRAY );
	Mat canny_output;
	Canny(gray, canny_output, kCannyThreshold, kCannyThreshold * 2);
	LaneMarkerDetector detector(src);

	Mat temp = Mat::zeros(src.rows, src.cols, src.type());

	detector.laneMarkerDetector(canny_output, src, temp);


	Point center(src.cols/2, src.rows*4/5);
	temp.at<Vec3b>(center.y, center.x) = kLaneRed;


	Points left = detector.getLeftLane(center);
	Points right = detector.getRightLane(center);

	cout<<"left:"<<left.size()<<";right:"<<right.size()<<endl;

	Mat test = Mat::zeros(src.rows, src.cols, src.type());
	cvtColor( canny_output, test, COLOR_GRAY2BGR);

	publish_points(test, left, kLaneRed);
	publish_points(test, right, kLaneRed);

	//Point
	Points cline = detector.getDirectionLine();
	//cout<<cline.size()<<endl;
	publish_points(test, cline, kLaneWhite);

	gray = test;


	return cline.size();
}


void publish_points(Mat& img, Points& points, const Vec3b& icolor) {
	//Point x => row   y => column
	for(int i = 0; i < points.size(); ++i) {
		Point point = points.at(i);
		img.at<Vec3b>(point.y, point.x) = icolor;
	}
}


void inversePerspectiveMapping(Mat& input, Mat& output) {

	// Input Quadilateral or Image plane coordinates
	Point2f inputQuad[4];
	// Output Quadilateral or World plane coordinates
	Point2f outputQuad[4];

	// Lambda Matrix
	Mat lambda( 2, 4, CV_32FC1 );

	// Set the lambda matrix the same type and size as input
	lambda = Mat::zeros( input.rows, input.cols, input.type() );

	// The 4 points that select quadilateral on the input , from top-left in clockwise order
	// These four pts are the sides of the rect box used as input
	inputQuad[0] = Point2f( input.cols/2 - 500, -100);
	inputQuad[1] = Point2f( input.cols/2 + 500, -100);
	inputQuad[2] = Point2f( input.cols - 1, input.rows - 1);
	inputQuad[3] = Point2f( 0, input.rows - 1);
	// The 4 points where the mapping is to be done , from top-left in clockwise order
	outputQuad[0] = Point2f(0, 0);
	outputQuad[1] = Point2f(input.cols-1, 0);
	outputQuad[2] = Point2f(input.cols-1, input.rows-1);
	outputQuad[3] = Point2f(0, input.rows-1);

	// Get the Perspective Transform Matrix i.e. lambda
	lambda = getPerspectiveTransform( inputQuad, outputQuad );
	// Apply the Perspective Transform just found to the src image
	warpPerspective(input, output, lambda, output.size());
}







Mat src, src_gray;
int thresh = 100;

int adjustTest()
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

