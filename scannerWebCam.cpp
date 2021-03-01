#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;


/////////////// Scanner //////////////////////
//variables
Mat imgOriginal, imgGray, imgCanny, imgThre, imgBlur, imgDil, imgErode, imgWarp, imgCrop;
vector<Point>initialPoints, reoPoints;
float w = 600, h = 500;

//Pre-processing img
Mat preProcessing(Mat img) {
	cvtColor(img, imgGray, COLOR_BGR2GRAY);
	GaussianBlur(img, imgBlur, Size(3,3), 0); 
	Canny(img, imgCanny, 25, 50);
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(imgCanny, imgDil, kernel);
	erode(imgDil, imgErode, kernel);
	return imgErode;
}

//Get contours
vector<Point> getContours(Mat img) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());

	vector<Point>biggest;
	int maxArea =0;

	for (int i = 0; i < contours.size(); i++)
	{
		int area = contourArea(contours[i]);
		//cout << area << endl;
		if (area > 1000)
		{
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);
			if (area > maxArea && conPoly[i].size()==4) {
				biggest = {conPoly[i][0],conPoly[i][1], conPoly[i][2], conPoly[i][3]};
				maxArea = area;
			}
		}
	}
	return biggest;
}

vector<Point>reorder(vector<Point> points)
{
	vector<Point> newPoints;
	vector<int> sumPoints, subPoints;

	for (int i = 0; i < 4; i++) {
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}

	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //0
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //1
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); //2
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); //3

	return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float w, float h)
{
	Point2f src[4] = { points[0], points[1],points[2], points[3]};
	Point2f dst[4] = { {0.0f,0.0f}, {w,0.0f}, {0.0f,h},{w,h} };

	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(w, h));
	return imgWarp;
}

void main() {

	////VideoCapture////
	VideoCapture cap(1); //0 for integrated cam 1(id) for external cam
	Mat imgWebCam,img;
	
	while (true) {

		cap.read(img);
		imshow("Image", img);
		cap >> imgWebCam;
		if (waitKey(1) == 27) {
			break;
		}
		
	}
	string path = "Resources/imgWebCam.jpg";
	imwrite(path, imgWebCam); 
	imgOriginal = imread(path);
	////Image Capture////

	//string path = "Resources/paper.jpg";
	//imgOriginal = imread(path);

	//resize(imgOriginal, imgOriginal, Size(), 0.5, 0.5);

	//Preprocessing
	imgThre = preProcessing(imgOriginal);
	// Get Contours return biggest shape on the image
	initialPoints = getContours(imgThre);
	// Reorder inital rect points from Countour x,x ; x+w,y ; x,y+h ; x+w,y+h
	reoPoints = reorder(initialPoints);
	//Warp image
	imgWarp = getWarp(imgOriginal, reoPoints, w, h);

	//Crop
	int cropVal = 7;
	Rect roi(cropVal, cropVal, w - (2 * cropVal), h - (2 * cropVal));
	imgCrop = imgWarp(roi);
	//Save Scan image 
	string path1 = "Resources/Scan_01.jpg";
	imwrite(path1, imgCrop);

	imshow("Image", imgOriginal);
	imshow("Image Dilation", imgThre);
	imshow("Image Warp", imgWarp);
	imshow("Scan 1", imgCrop);
	waitKey(0);
}