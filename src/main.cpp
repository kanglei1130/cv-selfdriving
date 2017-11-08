#include <opencv2/opencv.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/core/core.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


#include "headers.h"
#include "lane_marker_detector.h"
#include "remote_controller.h"
#include "udp_socket.h"
#include "utility.h"

string imagepath = "/home/wei/Pictures/1.jpg/";

void inversePerspectiveMapping(Mat& input, Mat& output);
void publish_points(Mat& img, Points& points, const Vec3b& color);
int processImage(Mat src, Mat& gray);
void test(Mat src, Mat& gray);


void processVideo();
void blurDetection(Mat& frame);

int changePixelColor();

void startThreads(int argc, char** argv);
void sendBackTime();


int main( int argc, char** argv )
{

  //processVideo();

  //cout<<"Hello Wolrd"<<endl;
  //startUdpServer();
  /*
  Mat ipm = Mat::zeros(src.rows, src.cols, src.type());
  inversePerspectiveMapping(canny_output, ipm);
  imshow( "ipm", ipm);*/



  //string in = string("/home/wei/Pictures/1.jpg/");
  //Mat src = imread(in, IMREAD_COLOR);
  //double clearness = utility::blurDetection(src);
  //cout<<clearness<<endl;



  //utility::adjustTest(src);

  // startThreads(argc, argv);


  //sendBackTime();

  //cout << currentTimeMillis() << endl;


  // string rawvideo = string("/home/lkang/Desktop/video_data/") + string("1510172761262.raw");
  // utility::convertFileToVideo(rawvideo, 1);
  // processVideo();

  return 0;

}


void startThreads(int argc, char** argv) {
  /*if there is an error about bind address or address already used
   *reconnect the tethering mode on the phone(turn off then turn on)
  */

  RemoteController* dataPool = new RemoteController(argc, argv);

  int num = 4;
  pthread_t threads[num];

  pthread_create (&(threads[0]), NULL, &RemoteController::GstreamerReceiver, dataPool);
  pthread_create (&(threads[1]), NULL, &RemoteController::UDPReceiverForCar, dataPool);
  pthread_create (&(threads[2]), NULL, &RemoteController::ControlPanel, dataPool);
  pthread_create (&(threads[3]), NULL, &RemoteController::VideoFrameProcesser, dataPool);

  for(int i = 0; i < num; ++i) {
    pthread_join(threads[i], NULL);
  }

  delete dataPool;
}





void processVideo() {
  VideoCapture cap("/home/lkang/Desktop/sample.h264"); // open the default camera

  if(!cap.isOpened()) { // check if we succeeded
    cout<<"not able to open"<<endl;
    return;
  }
  Mat gray;

  int counter = 0;
  for(;;) {
    counter ++;
    Mat frame;
    cap >> frame; // get a new frame from camera
    if(frame.empty()) {
      break;
    }

    // int num = processImage(frame, gray);
    imshow("gray", frame);
    waitKey(100);
    usleep(0);

    //break;
  }
  cout<<counter<<endl;
}



int changePixelColor() {
  cv::String in = string(imagepath + string("f00185.png"));
  Mat img = imread(in, IMREAD_COLOR);
  if (img.empty()) {
    cerr << "No image supplied ..." << endl;
    return -1;
  }
  int cols = img.cols;
  int rows = img.rows;

  cout<<rows<<","<<cols<<endl;
  for(int x = 0; x < rows; ++x) {
    for(int y = 0; y < cols; ++y) {
      if(y == cols/2) {
        img.at<Vec3b>(x, y) = Vec3b(255, 255, 255);;
      }
    }
  }
  imshow("Change Pixel Color", img);
  waitKey(0);
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

//adjust the edges/contours as image_test.cpp does
//const int kCannyThreshold = 200;
const int kCannyThreshold = 100;
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

  int leftsum = 0;
  int rightsum = 0;
  for(int i = 0; i < cline.size() && i < 20; ++i) {
    Point point = cline.at(i);
    if(point.x < center.x) {
      leftsum++;
    } else {
      rightsum++;
    }
  }
  int steering = 0;
  int sum = leftsum + rightsum;
  if(sum > 6) {
    double diff = double(leftsum - rightsum)/sum;
    if(diff > 0.3) {
      steering = -1;
    } else if(diff < -0.3) {
      steering = 1;
    } else {

    }
  }
  gray = test;
  return steering;
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






