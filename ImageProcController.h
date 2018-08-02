#include <opencv2/opencv.hpp>
//#include <opencv2/core.hpp>
#include <opencv2/tracking.hpp>
//#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui.hpp>
#include <string>
using namespace cv;
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
	Ptr<Tracker> tracker;
	CascadeClassifier face_cascade;
	TrackerMethod::Enum trackingMethod;
	std::vector<Point2f> LKpoints;

	ImageProcController();
	~ImageProcController();
	bool initTracker(TrackerMethod::Enum method);
	bool initFaceDetector(String classfierFilename);
	cv::Rect detectFace(Mat image);
	void updateTracker(Mat m, Rect2d &rect);
	void updateLKTracker(Mat m, Rect2d &rect);
private:
	bool trackerInitPointSet;
	Mat prevFrame;
	TermCriteria termcrit;
	Size winSize;
};

