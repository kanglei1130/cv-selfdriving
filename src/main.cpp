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
#include "packet_aggregator.h"

string imagepath = "/home/wei/Pictures/1.jpg/";

void inversePerspectiveMapping(Mat& input, Mat& output);
void publish_points(Mat& img, Points& points, const Vec3b& color);
int processImage(Mat src, Mat& gray);
void test(Mat src, Mat& gray);


void processVideo();

int changePixelColor();

void startThreads(int argc, char** argv);
void videoQuality(string rawVideo, string lossVideo);

void testPacketAggregator();

void downLoadImage(double latitude, double longitude, double heading, double pitch,  int i, int j);
void creatDir();
string GetCurrentWorkingDir();
void detectStopSign();

int main( int argc, char** argv )
{

  //processVideo();

  //cout<<"Hello Wolrd"<<endl;
  //startUdpServer();
  /*
  Mat ipm = Mat::zeros(src.rows, src.cols, src.type());
  inversePerspectiveMapping(canny_output, ipm);
  imshow( "ipm", ipm);*/



  //string in = string("/home/wei/Pictures/blur test image/worst_image.png");
  //Mat src = imread(in, IMREAD_COLOR);


  //utility::adjustTest(src);
  //utility::blurDetection_test(src);


  //startThreads(argc, argv);
  detectStopSign();

  //cout << currentTimeMillis() << endl;
/*
  string rawvideo = string("/home/lkang/Desktop/") + string("video.h264");
  string lossvideo = string("/home/lkang/Desktop/") + string("loss.h264");
  videoQuality(rawvideo, lossvideo);

*/

/*  string rawvideo = string("/home/wei/mobisysy/srcdat/video/turning_threshold/") + string("0_0.5.raw");
  //string testpath = string("/home/wei/mobisysy/compair_photo/video1/") + string("adapted_video.raw");

  utility::convertFileToVideo(rawvideo,0.0);*/

  /*
  string path = string("/home/lkang/Desktop/") + string("1511125613761.raw");
  utility::convertFileToVideoFEC(path, 0.0);
  */
  //processVideo();

/*  double pitch = 0.0;
  double latitude = 46.414382;
  double longitude = 10.013988;
  double heading = 151.78;
  int straightNum = 1;
  int rotationNum = 10;

  for(int i=0; i <straightNum;i++){
	for(int j=0; j <rotationNum;j++){
		downLoadImage(latitude, longitude,heading,pitch,i,j);
	}
  }*/

  return 0;
}


/* download the image */
void downLoadImage(double latitude, double longitude, double heading, double pitch, int i, int j) {
	CURL *image;
	CURLcode imgresult;
	FILE *fp;

	image = curl_easy_init();
	cout<<"initial"<<endl;
	if( image ){
		// Get local path, create folder
		creatDir();
		//Open file
		std::string outfilename = GetCurrentWorkingDir() + "/result/downloadedImage/straight_"
				+ to_string(i+1) + "_rotation_" + to_string(j+1) + ".jpg";
		cout<<GetCurrentWorkingDir()<<endl;
		fp = fopen(outfilename.c_str(), "wb");
		if( fp == NULL ) cout << "File cannot be opened";

		std::string url= "https://maps.googleapis.com/maps/api/streetview?size=10000x10000&\location=" + to_string(latitude+2*i)
					+ std::string(",") + to_string(longitude+2*i) + std::string("&\heading=") + to_string(heading+10*j)
					+ std::string("&\pitch=") + to_string(pitch);
		curl_easy_setopt(image, CURLOPT_URL, url.c_str());
		curl_easy_setopt(image, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(image, CURLOPT_WRITEDATA, fp);

		// Grab image
		imgresult = curl_easy_perform(image);
		if( imgresult ){
			cout << "Cannot grab the image!\n";
		}
	}
	// Clean up the resources
	curl_easy_cleanup(image);
	cout<<"cleanup"<<endl;
	// Close the file
	fclose(fp);
	cout<<"close fp"<<endl;
}

//get the local file path
std::string GetCurrentWorkingDir( void ) {
  char buff[FILENAME_MAX];
  GetCurrentDir( buff, FILENAME_MAX );
  std::string current_working_dir(buff);
  return current_working_dir;
}

//creat a new directory
void creatDir(){
	std::string dir = "mkdir -p " + GetCurrentWorkingDir() + "/result/downloadedImage";
	const int dir_err = system(dir.c_str());
	if (-1 == dir_err)
	{
	    printf("Error creating directory!n");
	    exit(1);
	}
}

// computes mean square error between two n-d matrices.
// lower -> more similar
static double meanSquareError(const Mat &img1, const Mat &img2) {
    Mat s1;
    absdiff(img1, img2, s1);   // |img1 - img2|
    s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits
    s1 = s1.mul(s1);           // |img1 - img2|^2
    Scalar s = sum(s1);        // sum elements per channel
    double sse = s.val[0] + s.val[1] + s.val[2];  // sum channels
    double mse = sse / (double)(img1.channels() * img1.total());
    return mse;
}


void detectStopSign() {
	// threshold of detection a stop sign
	int THRESHOLD = 7100;

    // read target image
	string in = string("/home/wei/Downloads/cv-selfdriving/StopSignDataset/11.jpg");
	string in2 = string("/home/wei/Downloads/cv-selfdriving/StopSignDataset/stopPrototype.png");
    Mat targetImage = imread(in, IMREAD_COLOR);;
    //cout << "Target Image: " << argv[1] << endl;

    // resize the image
    int width = 500;
    int height = width * targetImage.rows / targetImage.cols;
    resize(targetImage, targetImage, Size(width, height));
    // read prototype image
    //string in2 = string("/home/wei/Downloads/cv-selfdriving/StopSignDataset/stopPrototype.png");
    Mat prototypeImg = imread(in2, IMREAD_COLOR);

    //cout << "Prototype Image: " << argv[2] << endl;

    //int maxSim = 50000;
    int minMSE = INT_MAX;
    int location[4] = {0, 0, 0, 0};
    // start time
    int t0 = clock();
    //Mat tmpImg;
    Mat tmpImg = prototypeImg.clone();
    Mat window;
    for (int wsize = tmpImg.cols; wsize > 15; wsize /= 1.3) {
        if (tmpImg.rows < 15 || tmpImg.cols < 15)
        	break;
        if (tmpImg.rows > 900 || tmpImg.cols > 900) {
        	resize(tmpImg, tmpImg, Size(wsize, wsize));
        	continue;
        }
        cout << "Image pyramid width: " << wsize << " height: " << wsize << endl;
        for (int y = 0; y < targetImage.rows; y += 5) {
            for (int x = 0; x < targetImage.cols; x += 5) {
            	if (x + tmpImg.cols > targetImage.cols || y + tmpImg.cols > targetImage.rows)
            	    continue;
                Rect R(x, y, tmpImg.cols, tmpImg.cols); // create a rectangle
                window = targetImage(R); // crop the region of interest using above rectangle
                if (window.rows != tmpImg.rows || window.cols != tmpImg.cols)
                    continue;
                double tempSim = meanSquareError(tmpImg, window);
                if (tempSim < minMSE) {
                    minMSE = tempSim;
                    location[0] = x;
                    location[1] = y;
                    location[2] = tmpImg.rows;
                    location[3] = tmpImg.cols;
                }
            }
        }
        resize(tmpImg, tmpImg, Size(wsize, wsize));
    }

    // end time
    int t1 = clock();

    cout << "Execution time: " << (t1 - t0)/double(CLOCKS_PER_SEC)*1000 << " ms" << endl;
    cout << "Minimum MSE: " << minMSE << endl;
    if (minMSE < THRESHOLD) {
    	int buff1 = 50;
    	int x = location[0];
    	int y = location[1];
    	int w = location[2];
    	int h = location[3];
    	// draw the rectangle
    	rectangle(targetImage, Point(x-buff1/2,y-buff1/2), Point(x+w+buff1/2,y+h+buff1/2), Scalar(0,255,0), 2);
    	cout << "Stop sign found!" << endl;
    } else {
    	cout << "Stop sign not found!" << endl;
    }

    // show the image
    imshow("image", targetImage);
    waitKey(50000);
}

///////////////////////////////

void testPacketAggregator() {
  PacketAggregator packetAggregator;
  int frameLen = 38000;
  string payload(frameLen, 'x');

  FrameData frameData;
  frameData.compressedDataSize = frameLen;
  frameData.frameSendTime = 123;
  frameData.transmitSequence = 1;
  frameData.compressedDataSize = payload.size();


  vector<PacketAndData> packets = packetAggregator.deaggregatePackets(frameData, payload, 0.04);

  for (int i = 0; i < packets.size(); ++i) {
    PacketAndData cur = packets[i];
    packetAggregator.insertPacket(cur.first, cur.second);
  }
  for (int i = 0; i < packetAggregator.videoFrames.size(); ++i) {
    FrameAndData frameAndData = packetAggregator.videoFrames[i];
  }
}

void startThreads(int argc, char** argv) {
  /*if there is an error about bind address or address already used
   *reconnect the tethering mode on the phone(turn off then turn on)
  */

  RemoteController* dataPool = new RemoteController(argc, argv);

  int num = 4;
  pthread_t threads[num];

  pthread_create (&(threads[0]), NULL, &RemoteController::GstreamerReceiver, dataPool);
  pthread_create (&(threads[1]), NULL, &RemoteController::ControlPanel, dataPool);
  pthread_create (&(threads[2]), NULL, &RemoteController::VideoFrameProcesser, dataPool);
  if (dataPool->use_tcp_) {
    pthread_create (&(threads[3]), NULL, &RemoteController::TCPReceiverForCar, dataPool);
  } else {
    pthread_create (&(threads[3]), NULL, &RemoteController::UDPReceiverForCar, dataPool);
  }
  for(int i = 0; i < num; ++i) {
    pthread_join(threads[i], NULL);
  }

  delete dataPool;
}


void videoQuality(string rawVideo, string lossVideo) {
  VideoCapture raw(rawVideo.c_str());
  VideoCapture loss(lossVideo.c_str());

  if(!raw.isOpened() || !loss.isOpened()) { // check if we succeeded
    cout<<"not able to open"<<endl;
    return;
  }
  int counter = 0;
  double sum = 0.0;
  for(;;) {
    Mat raw_frame;
    Mat loss_frame;
    raw >> raw_frame; // get a new frame from camera
    loss >> loss_frame; // get a new frame from camera
    if(raw_frame.empty() || loss_frame.empty()) {
      break;
    }

    double snr = utility::getPSNR(raw_frame, loss_frame);
    if (snr == 0.0) snr = 40;
    // cout<<snr<<endl;
    counter ++;
    sum += snr;
  }
  cout<<sum / counter<<endl;
}



void processVideo() {
  VideoCapture cap("/home/wei/mobisysy/compair_photo/video/w_ovideo1.h264"); // open the default camera
  string output{"/home/wei/mobisysy/compair_photo/video2/1_/"};

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
    //imshow("gray", frame);
    //waitKey(100);
    //usleep(0);
    cv::imwrite(output + to_string(counter) + ".png", frame);

    if (counter > 1000) {
    	break;
    }
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






