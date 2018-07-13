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
		TLD,
		OFbased_SP,
		OFbased_MP
	};
}
class ImageProcController
{
public:
	cv::Ptr<cv::Tracker> tracker;
	cv::CascadeClassifier face_cascade;

	ImageProcController();
	~ImageProcController();
	bool initTracker(TrackerMethod::Enum method);
	bool initFaceDetector(cv::String classfierFilename);
	cv::Rect detectFace(cv::Mat image);

};

