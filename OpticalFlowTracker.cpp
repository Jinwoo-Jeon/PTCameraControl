#include "stdafx.h"
#include "OpticalFlowTracker.h"

#include <opencv2/opencv.hpp>
//#include <opencv2/core.hpp>
#include <opencv2/tracking.hpp>
//#include <opencv2/objdetect/objdetect.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

OpticalFlowTracker::OpticalFlowTracker()
{
}


OpticalFlowTracker::~OpticalFlowTracker()
{
}

void OpticalFlowTracker::initTracker()
{
	initialize();
	termcrit = TermCriteria(TermCriteria::COUNT | TermCriteria::EPS, 20, 0.03);
	winSize = Size(15, 15);
	mean_prev.x = 0; mean_prev.y = 0;
}

Rect OpticalFlowTracker::getTrackingBBox()
{
	ATLTRACE("point length %d\n", points[0].size());
	if (!points[0].empty()) 
	{
		if (points[0].size() == 1) {
			return Rect(points[0][0].x - 15, points[0][0].y - 15, 30, 30);
		}
		else {
			return Rect(mean.x - maxdistance_x, mean.y - maxdistance_y, maxdistance_x * 2, maxdistance_y * 2);
		}
	}
	return Rect();
}

float OpticalFlowTracker::GetDistance(float x1, float x2, float y1, float y2) {
	float dis = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
	//dis = dis > 0 ? dis : dis*(-1);
	return dis;
}


void OpticalFlowTracker::generateDragVal(cv::Rect2d &rect)
{

	// 드래그 포인트가 가로 세로 5 포인트 미만일 때, 포인트로 받아옴
	Point2f btn_point[2] = { Point2f(rect.x,rect.y), Point2f(rect.x+rect.width, rect.y+rect.height) };

	if ((btn_point[1].x - btn_point[0].x) <= 5 && (btn_point[1].y - btn_point[0].y) <= 5) {
		Drg_val.point = new Point2f[1];
		Drg_val.addRemovePt = new bool[1];
		Drg_val.size = Size(1, 1);
		Drg_val.point[0] = btn_point[0]; // _MS_ 클릭 시작한 지점 + 5배씩 늘어남
		Drg_val.addRemovePt[0] = true;		// _MS_ true로 초기화
		cout << "Clicked Point " << "(" << btn_point[0].x << ", " << btn_point[0].y << ") to (" << btn_point[1].x << ", " << btn_point[1].y << ")" << endl;
		btn_flag = false;
		point_cal_confirm = true;
		return;
	}

	Drg_val.point = new Point2f[(btn_point[1].y - btn_point[0].y)*(btn_point[1].x - btn_point[0].x) / Drag_Subsampling / Drag_Subsampling];
	Drg_val.addRemovePt = new bool[(btn_point[1].y - btn_point[0].y)*(btn_point[1].x - btn_point[0].x) / Drag_Subsampling / Drag_Subsampling];
	Drg_val.size = Size((int)(btn_point[1].x - btn_point[0].x) / Drag_Subsampling, (int)(btn_point[1].y - btn_point[0].y) / Drag_Subsampling);

	for (int h = 0; h < Drg_val.size.height; h++) {
		for (int w = 0; w < Drg_val.size.width; w++) {

			Drg_val.point[Drg_val.size.width*h + w] = Point2f(btn_point[0].x + w*Drag_Subsampling, btn_point[0].y + h*Drag_Subsampling); // _MS_ 클릭 시작한 지점 + 5배씩 늘어남
			Drg_val.addRemovePt[Drg_val.size.width*h + w] = true;		// _MS_ true로 초기화

		}
	}

}


void OpticalFlowTracker::updateTracker(cv::Mat m)
{
	Mat gray;
	cvtColor(m, gray, CV_RGB2GRAY);

	if (prevGray.empty())
	{
		prevGray = m;
	}

	average_distance = 0;

	for (int t = 0; t < 9; t++)
		partnum[t] = 0;


	//A지점 부터  B지점까지
	if (!points[0].empty())
	{
		vector<uchar> status;
		vector<float> err;
		if (prevGray.empty())
			gray.copyTo(prevGray);

		//Optical Flow 계산
		calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize, 3, termcrit, 0, 0.001);	//Optical Flow 계산

		size_t i, k;
		mean.x = 0;  mean.y = 0;
		for (i = k = 0; i < points[1].size(); i++)
		{
			for (int btn_y = 0; btn_y < Drg_val.size.height; btn_y++) {
				for (int btn_x = 0; btn_x < Drg_val.size.width; btn_x++) {		// _MS_ 
																				//printf("\nfor문 루프");
					if (Drg_val.addRemovePt[Drg_val.size.width*btn_y + btn_x])	//이미 생성된 특징점 다시 클릭 할 경우
					{
						if (norm(Drg_val.point[Drg_val.size.width*btn_y + btn_x] - points[1][i]) <= 5)	//절대 배열 계산
						{
							Drg_val.addRemovePt[Drg_val.size.width*btn_y + btn_x] = false;
							continue;
						}
					}
				}
			}	//이미 생성된 특징점에 대한 중복성 제거 끝

			if (refined) {	//재정의 된 특징점에 대한 중복성 제거
				afterrefine_addRemovePt = false;
				refined = false;
			}
			if (!status[i])
				continue;

			points[1][k] = points[1][i];

			distance[0].push_back(GetDistance(points[0][i].x, points[1][k].x, points[0][i].y, points[1][k].y));

			//특징점들의 평균을 구하기 위한 mean point 값
			mean.x = mean.x + points[1][k].x;
			mean.y = mean.y + points[1][k].y;

			if (ROIcycle == 0) {

				//크기 정보   //features 방향 분류 하기 위한 계산
				float sx = 0;
				float sy = 0;
				float gradient = 0;
				bool gradientway = false;
				bool sizecheck = false;

				Point2f flowatxy;
				flowatxy.x = (points[1][k].x - points[0][i].x);
				flowatxy.y = (points[1][k].y - points[0][i].y);

				sx = flowatxy.x >= 0 ? flowatxy.x : flowatxy.x*(-1);
				sy = flowatxy.y >= 0 ? flowatxy.y : flowatxy.y*(-1);
				gradient = sy / sx;
				gradientway = gradient > 1 ? false : true;	//x가 크면 true, y가 크면 false 

				sizecheck = true;
				int part = 0;
				if (gradientway == true && sizecheck == true) {
					if (flowatxy.x >= 0 && flowatxy.y >= 0) {		//연분
						part = 8;
					}
					else if (flowatxy.x >= 0 && flowatxy.y < 0) {	//빨강
						part = 1;
					}
					else if (flowatxy.x < 0 && flowatxy.y >= 0) {		//연파
						part = 5;
					}
					else if (flowatxy.x < 0 && flowatxy.y < 0) {		//초
						part = 4;
					}
				}
				else if (gradientway == false && sizecheck == true) {
					if (flowatxy.x >= 0 && flowatxy.y >= 0) {			//보
						part = 7;
					}
					else if (flowatxy.x >= 0 && flowatxy.y < 0) {		//주
						part = 2;
					}
					else if (flowatxy.x < 0 && flowatxy.y >= 0) {		//파
						part = 6;
					}
					else if (flowatxy.x < 0 && flowatxy.y < 0) {		//노
						part = 3;
					}
				}
				else
					printf("");

				point_part[0].push_back(part);
				partnum[part]++;
			}
			k++;
		}////for 문 끝

		points[1].resize(k);
		point_part[0].resize(k);
		distance[0].resize(k);

		//크기 정보   //가장 많은 수를 가지는 분포값을 추출
		maxpart = 0;
		for (int j = 1; j < 9; j++) {
			if (partnum[j] > partnum[maxpart])
				maxpart = j;
		}
		if (maxpart == 0)
			printf("");

		//Adaptive ROI decision
		//ROI 그려주기
		int q = 1;
		if (!points[1].empty()) {

			if (!needtoerase) {
				numberofintialfeatures = points[1].size();
				needtoerase = true;
			}
			if (!needtoerase_for_new) {
				numberofintialfeatures_for_new = points[1].size();
				needtoerase_for_new = true;
			}
			mean.x = mean.x / k;
			mean.y = mean.y / k;

			if (erased_features_count >= numberofintialfeatures / 2) {
				maxdistance_x = maxdistance_x / 10 * 9;
				maxdistance_y = maxdistance_y / 10 * 9;
				erased_features_count = 0;
				needtoerase = false;
			}

			if (ROIcycle == 0) {
				for (i = 0; i < points[1].size(); i++) {
					//크기 정보 //90도 이상 바뀐 OF 체크
					int similarpart1, similarpart2;
					similarpart1 = maxpart - 1;
					similarpart2 = maxpart + 1;
					if (similarpart1 == 0)
						similarpart1 = 8;
					if (similarpart2 == 9)
						similarpart2 = 1;

					if (distance[0][i] < 0)
						printf("");
					average_distance = average_distance + distance[0][i];
					q++;
					if (point_part[0][i] == maxpart || point_part[0][i] == similarpart1 || point_part[0][i] == similarpart2) {
						//average_distance = average_distance + distance[0][i];
						//q++;
						float x = mean.x - points[1][i].x > 0 ? mean.x - points[1][i].x : (mean.x - points[1][i].x)*(-1);
						if (x > maxdistance_x && x < 100)
							maxdistance_x = x + 10;
						float y = mean.y - points[1][i].y > 0 ? mean.y - points[1][i].y : (mean.y - points[1][i].y)*(-1);
						if (y > maxdistance_y && y < 100)
							maxdistance_y = y + 10;
					}
					//printf(" 거리 %f \t", distance[0][i]);
				}	//for문 끝
				if (maxdistance_x < 15)
					maxdistance_x = 15;
				if (maxdistance_y < 15)
					maxdistance_y = 15;

			}
			average_distance = average_distance / q - 1;

			//circle(image, Point(mean.x, mean.y ), 15, Scalar(0, 255, 0), 2, 8);	//중심 분포 Circle 그리기 
			//if (points[1].size() == 1)
			//	rectangle(image, Point(points[1][0].x - 15, points[1][0].y - 15), Point(points[1][0].x + 15, points[1][0].y + 15), (0, 0, 255), 3);

			//else
			//	rectangle(image, Point(mean.x - maxdistance_x, mean.y - maxdistance_y), Point(mean.x + maxdistance_x, mean.y + maxdistance_y), (0, 0, 255), 3);


			if (!firstframe_decoded) {
				mean_prev = mean;
			}
			velocity[acceleration_frame_count] = GetDistance(mean_prev.x, mean.x, mean_prev.y, mean.y);

			av_velocity = 0;
			for (int k = 0; k < MAX_ITERATION_FOR_VELOCITY; k++) {
				av_velocity = av_velocity + velocity[k];
			}
			av_velocity = av_velocity / MAX_ITERATION_FOR_VELOCITY;

			if (!av_velocity_calculated && acceleration_frame_count >= MAX_ITERATION_FOR_VELOCITY - 1)
				av_velocity_calculated = true;

			if (GetDistance(mean_prev.x, mean.x, mean_prev.y, mean.y) < 1.3)
				status_hovering = true;
			else
				status_hovering = false;

			acceleration_frame_count++;
			if (acceleration_frame_count >= MAX_ITERATION_FOR_VELOCITY) {
				acceleration_frame_count = 0;
			}

			// 특징점 제거 부분
			roi_width = maxdistance_x * 2;
			roi_height = maxdistance_y * 2;
			for (i = 0; i < points[1].size(); i++) {
				//크기 정보 //90도 이상 바뀐 OF 체크
				int similarpart1, similarpart2;

				similarpart1 = maxpart - 1;
				similarpart2 = maxpart + 1;

				if (similarpart1 == 0)
					similarpart1 = 8;
				if (similarpart2 == 9)
					similarpart2 = 1;
				float x = 0, y = 0;
				similarpart1 = similarpart2 = maxpart;

				float volocity_check = 0;
				float threshold_volocity = 0;

				if (point_part[0][i] == maxpart || point_part[0][i] == similarpart1 || point_part[0][i] == similarpart2) {

				}
				else {

					if (av_velocity_calculated && !status_hovering) {
						volocity_check = distance[0][i] - av_velocity > 0 ? distance[0][i] - av_velocity : (distance[0][i] - av_velocity)*(-1);
						threshold_volocity = av_velocity / 3 * 2;

						if (volocity_check >= threshold_volocity) {
							refine_point[0].push_back(points[1][i]);
							//erased_features_count++;
							points[1].erase(points[1].begin() + i);
							continue;
						}
					}
					x = mean.x - points[1][i].x > 0 ? mean.x - points[1][i].x : (mean.x - points[1][i].x)*(-1);
					if (x > roi_width) {
						//refine_point[0].push_back(points[1][i]);
						points[1].erase(points[1].begin() + i);
						erased_features_count++;
						erased_for_new++;
						continue;
					}
					y = mean.y - points[1][i].y > 0 ? mean.y - points[1][i].y : (mean.y - points[1][i].y)*(-1);
					if (y > roi_height)
						//refine_point[0].push_back(points[1][i]);
						points[1].erase(points[1].begin() + i);
					erased_features_count++;
					erased_for_new++;
					continue;
				}

			}	//특징점 제거 끝
			for (i = 0; i < points[1].size(); i++) {
				printf("");
			}
			points[1].resize(i);
			//ROIcycle++;
			if (ROIcycle >= 1)
				ROIcycle = 0;
		} //ROI 그려주는 if문 끝나는 지점

		if (!firstframe_decoded)
			firstframe_decoded = true;

		mean_prev = mean;
		distance[0].clear();

		tracked_frame_count++;

	}// else if (!points[0].empty()) 끝나는 지점


	 //Resampling 부분
	if (!status_hovering && !refine_point[0].empty() && refine_point[0].size() < (size_t)MAX_COUNT) {
		for (int i = 0; i < refine_point[0].size(); i++) {
			vector<Point2f> tmp;
			tmp.push_back(refine_point[0][i]);
			//circle(image, refine_point[0][i], 10, Scalar(0, 0, 255), 2, 8);
			cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit); //코너 위치 수정
																	  //circle(image, tmp[0], 10, Scalar(0, 255, 0),1, 8);
			points[1].push_back(tmp[0]);
		}
		refine_point[0].clear();
		refined = true;
		afterrefine_addRemovePt = true;
	}

	if (!status_hovering&&tracked_frame_count >= 60 /*&& points[1].size() < 50*/ && !points[1].empty() && points[1].size() < (size_t)MAX_COUNT && erased_for_new >= numberofintialfeatures_for_new / 3 * 2) {
		for (int x = mean.x - maxdistance_x; x < mean.x + maxdistance_x; x += 20) {
			for (int y = mean.y - maxdistance_y; y < mean.y + maxdistance_y; y += 20) {
				vector<Point2f> tmp;
				Point2f corner;
				corner.x = x;
				corner.y = y;
				tmp.push_back(corner);
				cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit); //코너 위치 수정
				points[1].push_back(tmp[0]);
				//circle(image, tmp[0], 10, Scalar(0, 0, 255), 1, 8);
				//printf("\ncreate");
			}
		}
		needtoerase_for_new = false;
		tracked_frame_count = erased_for_new = 0;
		refined = true;
		afterrefine_addRemovePt = true;
	}
	//printf("포인트 사이즈 : %d", points[1].size());


	//FindConerFeature
	for (int btn_y = 0; btn_y < Drg_val.size.height; btn_y++) {
		for (int btn_x = 0; btn_x < Drg_val.size.width; btn_x++) {		// _MS_ 
			if (Drg_val.addRemovePt[Drg_val.size.width*btn_y + btn_x] && points[1].size() < (size_t)MAX_COUNT)	// __MS__ 새로운 관심 지점 클릭한 경우
			{
				vector<Point2f> tmp;
				tmp.push_back(Drg_val.point[Drg_val.size.width*btn_y + btn_x]);
				cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit); //코너 위치 수정
				points[1].push_back(tmp[0]);
				Drg_val.addRemovePt[Drg_val.size.width*btn_y + btn_x] = false;
			}
		}
	}

	//버튼 클릭이 끝나기 전에 메모리 초기화 시키는 상황을 방지하기 위함
	if (point_cal_confirm) {	// _MS_ 계산이 끝났으면 초기화
		delete Drg_val.addRemovePt;
		delete Drg_val.point;
		Drg_val.size = Size(0, 0);

		btn_point[0] = Point2f(-1, -1);
		btn_point[1] = Point2f(-1, -1);

		point_cal_confirm = !point_cal_confirm;
	}

	//needToInit = false;

	//true 일 경우 멈춰 있는 이미지에 Rectangle을 그려 마치 화면을 드래그 하는 것처럼 보이게 함
	//if (btn_flag == true) {
	//	rectangle(image, btn_point[0], btn_point[1], Scalar(0, 0, 255), 3);
	//}

	//if (frame_count == 30) {	// __MS__ 100 frame 마다 fps 측정

	//	last = GetTickCount();

	//	printf("FPS : %f\n", (float)30 / (last - first) * 1000);
	//	fps = (float)30 / (last - first) * 1000;
	//	frame_count = 0;
	//	first = GetTickCount();
	//}
	//// ......................................................................................
	//stringstream ss;
	//rectangle(image, cv::Point(10, 2), cv::Point(100, 20),				// 좌측 상단 흰색박스 생성 rectan 
	//	cv::Scalar(255, 255, 255), -1);
	////ss << cap.get(CAP_PROP_POS_FRAMES);
	//ss << fps;
	//if (fps != 0) {
	//	string frameNumberString = ss.str();
	//	putText(image, frameNumberString.c_str(), cv::Point(15, 15),
	//		FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
	//}

	//imshow("Lucas Kanade Tracker", image);	// 점표시된 이미지 보여줌
	//										//imshow("GRay", gray);	// 점표시된 이미지 보여줌


	//if (btn_flag == false) { // _MS_ btn_flag 즉, 누르고 있을 때에는 frame count를 올리지 않음
	//	frame_count++;	// __MS__
	//	total_frame_count++;
	//}

	//char c = (char)waitKey(10);
	//if (c == 27)
	//	break;
	//else if (c == 32)	// SPACE Key (pause)
	//{
	//	while ((c = waitKey(10)) != 32 && c != 27);
	//	if (c == 27) break;
	//}

	//switch (c)
	//{
	//case 'r':
	//	needToInit = true;
	//	break;
	//case 'c':
	//	points[0].clear();
	//	points[1].clear();
	//	break;
	//}
	std::swap(points[1], points[0]);
	cv::swap(prevGray, gray);
}


void OpticalFlowTracker::initialize() {
	allsum = 0;
	average = 0;
	deletecount = 0;
	ROIcycle = 0;
	maxdistance_x = 0;
	maxdistance_y = 0;
	partnum[9] = { 0 };
	maxpart = 0;
	roi_width = 0;
	roi_height = 0;
	roi_size = 0;
	firstframe_decoded = false;
	av_velocity_calculated = false;
	status_hovering = false;
	tracked_frame_count = 0;
	erased_features_count = 0;
	numberofintialfeatures = 0;
	needtoerase = false;
	erased_for_new = 0;
	numberofintialfeatures_for_new = 0;
	needtoerase_for_new = false;
	refined = false;
	afterrefine_addRemovePt = false;
	fps = 0;
	windowsize = 15;
	average_distance = 0;
	acceleration_frame_count = 0;
	av_velocity = 0;
	MAX_COUNT = 500;
	//needToInit = false;
	total_frame_count = 0;
	frame_count = 0;
	btn_flag = false;
	point_cal_confirm = false; // _MS_ 버튼 클릭후 뗐을 시 true로 올라가고, 계산이 마치면 false로 변함

	points[0].clear();
	points[1].clear();
	mean.x = 0; mean.y = 0;

}
