/*
 * utility
 *
 *  Created on: Sep 6, 2017
 *      Author: lkang
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "udp_socket.h"
#include "headers.h"

using namespace cv;
using namespace std;

namespace utility {

static std::string FrameDataFromCar = "frame_data_from_car";
static std::string FrameDataFromServer = "frame_data_from_server";

static std::string ControlMessageFromController = "control_message_from_controller";
static std::string ControlMessageFromServer = "control_message_from_server";


void convertFileToVideo(string file);
/**
 * @param src the input MAT image, with color
 * @return the blur value, use a threshold to detect blurness
 */
double blurDetection(cv::Mat& src);

/**
 * Canny threshold test
 */
void thresh_callback(int thresh, void* gray);
int adjustTest(Mat& src);

}



#endif /* UTILITY_H_ */
