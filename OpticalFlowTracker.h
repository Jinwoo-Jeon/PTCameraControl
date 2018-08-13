#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <string>

#define MAX_ITERATION_FOR_VELOCITY	5

using namespace cv;
using namespace std;

class OpticalFlowTracker
{
public:
	OpticalFlowTracker();
	~OpticalFlowTracker();
	void initTracker();
	void initialize();
	void updateTracker(Mat m);
	float GetDistance(float x1, float x2, float y1, float y2);
	void generateDragVal(cv::Rect2d &rect);

	Rect getTrackingBBox();

private:
	Mat prevGray;
	TermCriteria termcrit;
	Size winSize;

	//void drawOFdirection(int part, Point2f points0, Point2f points1);
	//void initialize();

	vector<float> distance[1];
	Point2f mean_prev;
	float velocity[MAX_ITERATION_FOR_VELOCITY] = { 0 };
	vector<Point2f> refine_point[1];

	float allsum;
	float average;
	vector<int> point_part[1];
	int deletecount;
	int ROIcycle;
	float maxdistance_x;
	float maxdistance_y;
	int partnum[9];
	int maxpart;
	float roi_width;
	float roi_height;
	float roi_size;
	vector<float> speed;
	bool firstframe_decoded;
	bool av_velocity_calculated;
	bool status_hovering;
	int tracked_frame_count;
	int erased_features_count;
	int numberofintialfeatures;
	bool needtoerase;
	int erased_for_new;
	int numberofintialfeatures_for_new;
	bool needtoerase_for_new;
	bool refined;
	bool afterrefine_addRemovePt;
	float fps;
	int windowsize;
	float average_distance;
	int acceleration_frame_count;
	DWORD first;
	DWORD first_first;
	DWORD last;
	vector<Point2f> points[2];
	float av_velocity;
	int MAX_COUNT;
	bool needToInit;
	int total_frame_count;
	int frame_count;
#define PI 3.14159265358979323846
#define Drag_Subsampling 5
	Mat Org_btn;
	typedef struct drag_point_and_flag {

		Point2f *point;
		bool *addRemovePt;
		cv::Size size;

	} Drag_pt_flag;

	Drag_pt_flag Drg_val;
	Point2f mean;
	bool btn_flag;
	bool point_cal_confirm; // _MS_ 버튼 클릭후 뗐을 시 true로 올라가고, 계산이 마치면 false로 변함
	Point2f btn_point[2] = { Point2f(-1,-1), Point2f(-1,-1) };

};

