#include "stdafx.h"
#include "ImageProcController.h"

#include <opencv2/opencv.hpp>
//#include <opencv2/core.hpp>
#include <opencv2/tracking.hpp>
//#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui.hpp>
#define MAX_COUNT 10
using namespace cv;
using namespace std;

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
	case TrackerMethod::OFbased_SP:
		termcrit = TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, 20, 0.03);
		winSize = Size(15, 15);
		prevFrame = Mat(Size(0,0), CV_8U);
		LKpoints.clear();
		break;
	default:
		return false;
	}
	trackerInitPointSet = false;
	trackingMethod = method;
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

//OpenCV tracker
void ImageProcController::updateTracker(cv::Mat m, cv::Rect2d &rect) {
	cvtColor(m, m, CV_RGB2GRAY);
	if (trackerInitPointSet)
	{
		tracker->update(m, rect);
	}
	else
	{
		if (!tracker->init(m, rect))
		{
			ATLTRACE("***Could not initialize tracker...***\r\n");
		}
		else
		{
			trackerInitPointSet = true;
		}
	}
}


void ImageProcController::updateLKTracker(cv::Mat m, cv::Rect2d &rect) {
	cvtColor(m, m, CV_RGB2GRAY);
	vector<Point2f> newLKpoints;
	if (prevFrame.size().width == 0)
	{
		double qualityLevel = 0.01;
		double minDistance = 10;
		int blockSize = 3;
		int gradSize = 3;
		cv::Mat mask(m.size(), CV_8U);
		mask = 0;
		mask(rect) = 1;

		prevFrame = m;
		goodFeaturesToTrack(m, newLKpoints, MAX_COUNT, qualityLevel, minDistance, mask, blockSize, gradSize);
		cornerSubPix(m, newLKpoints, winSize, Size(-1, -1), termcrit);
		ATLTRACE("feature init\n");
		ATLTRACE("%d\n", newLKpoints.size());
	}
	else
	{
		vector<uchar> status;
		vector<float> err;

		calcOpticalFlowPyrLK(prevFrame, m, LKpoints, newLKpoints, status, err, winSize, 3, termcrit, 0, 0.001);
		prevFrame = m;
	}
	std::swap(newLKpoints, LKpoints);
}