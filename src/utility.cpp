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


}



