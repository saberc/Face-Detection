//============================================================================
// Name        : FaceDectionVideo.cpp

// Author      : Sabrina Ecca
// Version     :
// Copyright   : Your copyright notice
// Description : C++, Ansi-style
//============================================================================

#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include <string>
using namespace std;
using namespace cv;

//Function Headers
void detectAndDisplay(Mat frame);

// Global variables
string faceCascadeName = "./haarcascades/haarcascade_frontalface_alt.xml";
//string eyesCascadeName = "./haarcascades/haarcascade_eye_tree_eyeglasses.xml";
string eyesCascadeName = "./haarcascades/haarcascade_eye.xml";
string windowName = "Face Detection";
string windowTest = "Detection Test";
string equalized_window = "Equalized Image";
const char* video = "/Users/sabriecca/Documents/Video/REC1/videodemo.mov";
const string filename = "/Users/sabriecca/Documents/Video/REC1/prova.xml";

CvCapture* capture = 0;
Mat grayFrame;
Mat frameCopy;
Mat frame;
bool paused = false;
double millsecFrame = 0;
int totalFrames = 0;
int posFrame = 0;
double detectTime = 0;
int x, y, h, w;
int width;
int height;

CascadeClassifier faceCascade;
CascadeClassifier eyesCascade;
RNG rng(12345);

/*Function main */
int main() {
	//Load the cascades
	if (!faceCascade.load(faceCascadeName)) {
		printf("Error loading\n");
		return -1;
	};
	if (!eyesCascade.load(eyesCascadeName)) {
		printf("Error loading\n");
		return -1;
	};

//Read the video stream
//capture = cvCaptureFromCAM(-1);
	capture = cvCaptureFromFile(video);
	if (capture) {
		for (;;) {
			if (!paused) {
				frame = cvQueryFrame(capture);
				//Resize video dimension
				//resize(frame,frame,Size(),0.5,0.5);
				//Apply the classifier to the frame
				if (!frame.empty()) {
					detectAndDisplay(frame);
                    //Get frame properties
					int widthFrame = cvGetCaptureProperty(capture,
							CV_CAP_PROP_FRAME_WIDTH);
					int heightFrame = cvGetCaptureProperty(capture,
							CV_CAP_PROP_FRAME_HEIGHT);
					totalFrames = (int) cvGetCaptureProperty(capture,
							CV_CAP_PROP_FRAME_COUNT);
					posFrame = ((int) cvGetCaptureProperty(capture,
							CV_CAP_PROP_POS_FRAMES));
					millsecFrame = cvGetCaptureProperty(capture,
							CV_CAP_PROP_POS_MSEC);
					printf(
							"widthFrame: %d heightFrame: %d totalFrames: %d posFrame: %d millsecFrame: %g ms\n",
							widthFrame, heightFrame, totalFrames, posFrame, millsecFrame);
				} else {
					printf(" No captured frame!");
					break;
				}
			}
			//Get pause or stop the video stream
			char c = (char) waitKey(10);
			if (c == 'c') {
				break;
			} else {
				if (c == 'p') {
					paused = !paused;
				} else if (c == 'q') {
					paused = !paused;
				}
			}
		}
		return 0;
	}
}

/**
 * Function DetectAndDisplay
 */

void detectAndDisplay(Mat frame) {
	vector<Rect> faces;
	static CvScalar colors[] =
			{ { { 0, 0, 255 } }, { { 0, 128, 255 } }, { { 0, 255, 255 } }, { {
					0, 255, 0 } }, { { 255, 128, 0 } }, { { 255, 255, 0 } }, { {
					255, 0, 0 } }, { { 255, 0, 255 } }, { { 255, 255, 255 } } };
	cvtColor(frame, grayFrame, CV_BGR2GRAY);
	resize(grayFrame, grayFrame, grayFrame.size(), 0, 0, INTER_LINEAR);
	equalizeHist(grayFrame, grayFrame);

	//Compute the detection time
	double t = (double) cvGetTickCount();
	//Detects objects of different sizes in the input image. The detected objects are returned as a list of rectangles.
	faceCascade.detectMultiScale(grayFrame, faces, 1.2, 3,
			0 | CV_HAAR_SCALE_IMAGE, Size(10, 60));
	t = (double) cvGetTickCount() - t;
	detectTime = (t / ((double) cvGetTickFrequency() * 1000.));
	printf("detection time = %g ms\n", detectTime);

	/*	for (size_t i = 0; i < faces.size() ; i++) {
	 Point center(faces[i].x + faces[i].width / 2, faces[i].y + faces[i].height / 2);
	 ellipse(frame, center, Size(faces[i].width / 2, faces[i].height / 2), 0, 0, 360, colors[i%8], 2, 8, 0);
	 */

	//Draw the the Bounding Box for each detected face
	for (size_t i = 0; i < faces.size(); i++) {
		Mat faceROI = grayFrame(faces[i]);
		vector<Rect> eyes;
		Scalar eyeColor = CV_RGB(255,255,255);
		x = faces[i].x;
		y = faces[i].y;
		w = x + faces[i].width;
		h = y + faces[i].height;
		rectangle(frame, Point(x, y), Point(w, h), colors[i % 8], 2, 8, 0);
		width = faces[i].width;
		height = faces[i].height;
		printf("x= %d " "y= %d " "w= %d " "h= %d\n ", x, y, width, height);
		//Create the text we will annotate the bounding box coordinates
		string boxText = format("x= %d " "y= %d " "w= %d " "h= %d ", x, y,
				width, height);
		//Put the coordinates in the image at the position of Point(x,y)
		putText(frame, boxText, Point(x, y), FONT_HERSHEY_PLAIN, 1.0,
				CV_RGB(0,255,0), 2.0);
		//In each face, detect eyes
		eyesCascade.detectMultiScale(faceROI, eyes, 1.2, 1, 1 | CV_HAAR_SCALE_IMAGE, Size(20, 20));
		for (size_t j = 0; j < eyes.size(); j++) {

		Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2, faces[i].y + eyes[j].y + eyes[j].height / 2);
		int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
		circle(frame, eye_center, radius, eyeColor, 1, 8, 0);

		}
	}
		//Show what you got
		imshow(windowName, frame);

		//Save frame data in xml file
		FileStorage fs(filename, FileStorage::APPEND);
		if (!fs.isOpened()) {
			cout << "Unable to open file storage!" << endl;
		}
		/*
		 *First format of xml file
		fs << "Frame";
		fs << "{" << "id" << posFrame;
		fs << "millsecFrame" << millsecFrame;
		fs << "detectionTime" << detectTime;
		fs << "BBox" << "[";
		fs << "x=" << x << "y=" << y << "w=" << width << "h=" << height;
		fs << "]";
		"}";
        */

		//Second format of xml file
		fs << "Frame";
		fs << "{" << "id" << posFrame;
		fs << "millsecFrame" << millsecFrame;
		fs << "detectionTime" << detectTime;
		fs << "BBox";
			fs << "{" << "x" << x;
				fs << "y" << y;
				fs << "w" << width;
				fs << "h" << height << "}"<<"}";




		fs.release();

}

