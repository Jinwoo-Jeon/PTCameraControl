//#include <opencv2/opencv.hpp>
//#include <opencv2/core.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui.hpp>
#include <string>

namespace TrackerMethod {
	enum Enum {
		Boosting,
		CSRT,
		GOTURN,
		KCF,
		MedianFlow,
		MIL,
		MOSSE,
		TLD
	};
}
class ImageProcController
{
public:
	uchar* imageBufferPtr;
	int imageWidth;
	int imageHeight;

	cv::Ptr<cv::Tracker> tracker;
	cv::CascadeClassifier face_cascade;

	ImageProcController();
	~ImageProcController();
	bool initTracker(TrackerMethod::Enum method);
	bool initFaceDetector(cv::String classfierFilename);
	void setImageInfo();
	cv::Mat getRGBImageMat();

};

