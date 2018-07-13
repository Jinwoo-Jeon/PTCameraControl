#include "stdafx.h"
#include "ImageProcController.h"

//#include <opencv2/opencv.hpp>
//#include <opencv2/core.hpp>
#include <opencv2/tracking.hpp>
//#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui.hpp>

using namespace cv;

ImageProcController::ImageProcController()
{
}


ImageProcController::~ImageProcController()
{
}

bool ImageProcController::initTracker(TrackerMethod::Enum method)
{
	switch (method) {
	case TrackerMethod::Boosting:
		tracker = cv::TrackerBoosting::create();
		break;
	case TrackerMethod::CSRT:
		tracker = cv::TrackerCSRT::create();
		break;
	case TrackerMethod::GOTURN:
		tracker = cv::TrackerGOTURN::create();
		break;
	case TrackerMethod::KCF:
		tracker = cv::TrackerKCF::create();
		break;
	case TrackerMethod::MedianFlow:
		tracker = cv::TrackerMedianFlow::create();
		break;
	case TrackerMethod::MIL:
		tracker = cv::TrackerMIL::create();
		break;
	case TrackerMethod::MOSSE:
		tracker = cv::TrackerMOSSE::create();
		break;
	case TrackerMethod::TLD:
		tracker = cv::TrackerTLD::create();
		break;
	default:
		return false;
	}
	return true;
}

bool ImageProcController::initFaceDetector(cv::String classfierFilename)
{
	if (!face_cascade.load(classfierFilename))
	{
		return false;
	}
	return true;

}

cv::Rect ImageProcController::detectFace(cv::Mat image)
{
	Point faceCntrPoint;
	std::vector<Rect> faces;
	face_cascade.detectMultiScale(image, faces, 1.1, 2);
	if (faces.size() > 0)
	{
		return faces[0];
	}
	else 
	{
		return cv::Rect(0,0,0,0);
	}
}
