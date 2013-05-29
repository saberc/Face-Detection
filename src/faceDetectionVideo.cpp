//============================================================================
// Name        : FaceDetectionVideo.cpp
// Author      : Sabrina Ecca
// Version     :
// Copyright   : Your copyright notice
// Description : C++, OpenCV 2.4.2
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

// Global variables

//Cascade Classifier file, used for face detection
const char* faceCascadeName = "./haarcascades/haarcascade_frontalface_alt.xml";
const char* eyesCascadeName = "./haarcascades/haarcascade_eye_tree_eyeglasses.xml";
const char* video = "/Users/sabriecca/Documents/Video/REC1/videodemo.mov";
const char* filename = "./data/data.xml";
//const char* filename = "./data/data1.xml";
const char* windowName = "Face Detection"; //Name shown in the GUI window

CvCapture* capture = 0;
Mat grayFrame;
Mat frame;
int nrFrames = 0;
int posFrame;
int posFrameBefore;
int widthFrame;
int heightFrame;
int x, y, h, w;
int width, height;
int msecFrame;
int msecFrameBefore;
double detectTime;
bool paused = false;

CascadeClassifier faceCascade;
CascadeClassifier eyesCascade;

RNG rng(12345);

//Function Headers
void detectAndDisplay(Mat frame, FileStorage& fs);

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

/*Read the video stream*/
//capture = cvCaptureFromCAM(-1);
	capture = cvCaptureFromFile(video);
	if (capture) {
		nrFrames = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
		//Save frame data in xml file storage
		FileStorage fs(filename, FileStorage::WRITE);
		if (!fs.isOpened()) {
			cout << "Unable to open file storage!" << endl;
		}
		fs << "nrFrames" << nrFrames;
		fs << "Frames" << "[";

		for (;;) {
			if (!paused) {
				frame = cvQueryFrame(capture);
				//Apply the classifier to the frame
				if (!frame.empty()) {

					fs << "{" << "Frame" << "{";
					detectAndDisplay(frame, fs);
				    fs << "}" << "}";

					//Get frame properties
					widthFrame = cvGetCaptureProperty(capture,       //Dimension of the individual frames of the video to be read o captured.
							CV_CAP_PROP_FRAME_WIDTH);
					heightFrame = cvGetCaptureProperty(capture,      //Dimension of the individual frames of the video to be read o captured.
							CV_CAP_PROP_FRAME_HEIGHT);

					posFrameBefore = ((int) cvGetCaptureProperty(capture,   //POS_FRAME is the current position in frame number. It retrieves the current frame number.
							CV_CAP_PROP_POS_FRAMES));
					msecFrameBefore = (int) cvGetCaptureProperty(capture,   //POS_MSEC is the current position in a video file, measured in milliseconds.
							CV_CAP_PROP_POS_MSEC);
					printf(
							"widthFrame: %d heightFrame: %d nrFrames: %d posFrameBefore: %d msecFrameBefore: %d ms\n",
							widthFrame, heightFrame, nrFrames, posFrameBefore, msecFrameBefore);
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
		fs << "]";
		fs.release();

		return 0;
	}
}

/*
 * Function DetectAndDisplay
 */
void detectAndDisplay(Mat frame, FileStorage& fs) {
	vector<Rect> faces;
	static CvScalar colors[] =
			{ { { 0, 0, 255 } }, { { 0, 128, 255 } }, { { 0, 255, 255 } }, { {
					0, 255, 0 } }, { { 255, 128, 0 } }, { { 255, 255, 0 } }, { {
					255, 0, 0 } }, { { 255, 0, 255 } }, { { 255, 255, 255 } } };

	//If the input image is not gray scale, then convert the BGR or BGRA color image to gray scale.
	if (frame.channels() == 3) {
		cvtColor(frame, grayFrame, CV_BGR2GRAY);
	} else if (frame.channels() == 4) {
		cvtColor(frame, grayFrame, CV_BGRA2GRAY);
	} else {
		//Access the input image directly, since it is already gray scale.
		grayFrame = frame;
	}

	resize(grayFrame, grayFrame, grayFrame.size(), 0, 0, INTER_LINEAR);

	//Standardize the brightness and contrast to improve dark images.
	equalizeHist(grayFrame, grayFrame);

	//Compute the detection time
	double t = (double) cvGetTickCount();
	//Detects objects of different sizes in the gray scale image. Then, the detected objects are returned as a list of rectangles.
	faceCascade.detectMultiScale(grayFrame, faces, 1.2, 4,
			0 | CV_HAAR_SCALE_IMAGE | CV_HAAR_FIND_BIGGEST_OBJECT,
			Size(10, 60));
	t = (double) cvGetTickCount() - t;
	detectTime = (int)(t / ((double) cvGetTickFrequency() * 1000.));
	printf("detection time = %g ms\n", detectTime);

	posFrame = ((int) cvGetCaptureProperty(capture,   //POS_FRAME is the current position in frame number. It retrieves the current frame number.
			CV_CAP_PROP_POS_FRAMES));
	msecFrame = (int) cvGetCaptureProperty(capture,   //POS_MSEC is the current position in a video file, measured in milliseconds.
			CV_CAP_PROP_POS_MSEC);
	printf( "posFrame: %d msecFrame: %d ms\n",
			posFrame, msecFrame);

	fs << "frameId" << posFrame;
	fs << "msecFrame" << msecFrame;
	fs << "detectionTime" << detectTime;
	fs << "BBoxes" << "[";

	int bbId = 0;
	//Draw the Bounding Box for each detected face
	for (size_t i = 0; i < faces.size(); i++) {
		Mat faceROI = grayFrame(faces[i]);
		vector<Rect> eyes;
		Scalar eyeColor = CV_RGB(255,255,255);

		x = faces[i].x;
		y = faces[i].y;
		w = x + faces[i].width;
		h = y + faces[i].height;

		rectangle(frame, Point(x, y), Point(w, h), colors[i % 8], 2, 8, 0);

		//Width and height of the BBox
		width = faces[i].width;
		height = faces[i].height;

		printf("x= %d " "y= %d " "w= %d " "h= %d\n ", x, y, width, height);
		//Create the text we will annotate the bounding box coordinates
		string boxText = format("x= %d " "y= %d " "w= %d " "h= %d ", x, y,
				width, height);
		//Put the coordinates in the image at the position of Point(x,y)
		putText(frame, boxText, Point(x, y), FONT_HERSHEY_PLAIN, 1.0,
				CV_RGB(0,255,0), 2.0);

		//For each face, detect eyes
		eyesCascade.detectMultiScale(faceROI, eyes, 1.2, 1,
				1 | CV_HAAR_SCALE_IMAGE, Size(20, 20));
		for (size_t j = 0; j < eyes.size(); j++) {
			Point eye_center(faces[i].x + eyes[j].x + eyes[j].width / 2,
					faces[i].y + eyes[j].y + eyes[j].height / 2);
			int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
			circle(frame, eye_center, radius, eyeColor, 1, 8, 0);
		}

		bbId = (bbId + 1);
		//Load BBox data in xml file storage
		fs << "{" << "BBox" << "{";
		fs << "bboxId" << bbId;
		fs << "x" << x;
		fs << "y" << y;
		fs << "w" << width;
		fs << "h" << height;
		fs << "}" << "}";

		//Show what you got
		imshow(windowName, frame);
	}

	fs << "]";

}

