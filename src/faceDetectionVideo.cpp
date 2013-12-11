//============================================================================
// Name        : FaceDetectionVideo.cpp

// Author      : Sabrina Ecca
// Description : C++, OpenCV 2.4.3
//============================================================================

#include "opencv2/core/core.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "DataSharing.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <time.h>
#include <string>
using namespace std;
using namespace cv;

//Global variables
//Cascade Classifier file, used for face detection
const char* faceCascadeName = "./Parameters/haar/haarcascade_frontalface_alt.xml";
const char* eyesCascadeName = "./Parameters/haar/haarcascade_eye_tree_eyeglasses.xml";

const char* windowName = "Face Detection";     //Name shown in the GUI window
CvCapture* capture = NULL;
Mat grayFrame;
Mat frame;
IplImage* result;
clock_t start,end;
int nrFrames = 0;
int posFrame;
int posFrameBefore;
int msecFrame;
int msecFrameBefore;
int widthFrame;
int heightFrame;
int x, y, h, w;
int width, height;
int bbNr = 0;
double fps;
double detectTime;
bool paused = false;
string video;
string output;
string filename;
string filename1;

CascadeClassifier faceCascade;
CascadeClassifier eyesCascade;

RNG rng(12345);

/*Function Headers*/
void detectAndDisplay(Mat frame, FileStorage& fs, int bbNr);

/*Function main */
int main(int argc,  char **argv){
	//Variables to compute the detection time
	int time;
	start=clock();

   //Call Socket
   bool connectionToSocket = false;
   DataSharing socket;
	if (socket.connectToSoket("127.0.0.1")){
		connectionToSocket = true;
		cout << endl << "connectedDetection";
	}
	else cout << endl << "notConnectedDetection";
	if (argc > 1)
	 { cout << endl << argv[0]<< endl;

	    video =  "./" + (string) argv[1];
	    filename = "./Data/" + (string) argv[1] +".xml";
	    filename1 = "./Data/" + (string) argv[1] +".yml";
	    output =   "./VideoOut/" + (string) argv[1];
	    cout << endl << video << endl<< filename << endl << output;
	    };

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
	capture = cvCaptureFromFile(video.c_str());
	if (capture) {
		//Get frame properties
		nrFrames = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_COUNT);
		widthFrame = cvGetCaptureProperty(capture,             //Dimension of the individual frames of the video to be read o captured.
				CV_CAP_PROP_FRAME_WIDTH);
		heightFrame = cvGetCaptureProperty(capture,            //Dimension of the individual frames of the video to be read o captured.
				CV_CAP_PROP_FRAME_HEIGHT);
		fps = cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);  //Frame rate

		//Save frame data in xml file storage
		FileStorage fs(filename, FileStorage::WRITE);

		//Save frame data in yml file storage
		FileStorage fs1(filename1, FileStorage::WRITE);

		if (!fs.isOpened()) {
			cout << "Input video could not be opened !!" << endl;
		}
		if (!fs1.isOpened()) {
			cout << "Input video could not be opened !!" << endl;
		}
		fs << "nrFrames" << nrFrames;
		fs << "Frames" << "[";

		VideoWriter writer(output,
		#ifdef PROBLEM
				CV_FOURCC('I','4','2','0') ,
		#else
				CV_FOURCC('M', 'J', 'P', 'G'),
		#endif
				fps, Size(widthFrame, heightFrame), 1);

		for (;;) {

			if (!paused) {
				frame = cvQueryFrame(capture);
				//Apply the classifier to the frame
				if (!frame.empty()) {
					detectAndDisplay(frame, fs, bbNr);
					bbNr++;
					printf("%d Detected Faces Nr.\n", bbNr);

					if(connectionToSocket)
						socket.writeToSoket(((float) (posFrame + 1)/nrFrames)*100);

					//Get frame properties
					posFrameBefore = ((int) cvGetCaptureProperty(capture, //POS_FRAME is the current position in frame number. It retrieves the current frame number.
							CV_CAP_PROP_POS_FRAMES));
					msecFrameBefore = (int) cvGetCaptureProperty(capture, //POS_MSEC is the current position in a video file, measured in milliseconds.
							CV_CAP_PROP_POS_MSEC);
					printf(
							"widthFrame: %d heightFrame: %d nrFrames: %d posFrameBefore: %d msecFrameBefore: %d ms fps: %g\n",
							widthFrame, heightFrame, nrFrames, posFrameBefore,
							msecFrameBefore, fps);

					writer << frame;

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

		double t1 = (double) cvGetTickCount() - t1;
		double detectTime1 = (int) (t1 / ((double) cvGetTickFrequency() * 1000.));

		end = clock();
		time = ((int)(end-start))/CLOCKS_PER_SEC ;

		fs1 << "Frames Nr" << nrFrames;
		fs1 << "Detected Faces Nr" << bbNr;
		fs1 << "detectionTime" << detectTime1;
		fs1 << "detectionTime1" << time;

		fs << "]";
		fs.release();
		fs1.release();
		if(connectionToSocket)
		socket.release();
		return 0;
	}
}
/*
 * Function DetectAndDisplay
 */
void detectAndDisplay(Mat frame, FileStorage& fs, int bbNr) {
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

	//cvtColor(frame, grayFrame, CV_BGR2GRAY );
	resize(grayFrame, grayFrame, grayFrame.size(), 0, 0, INTER_LINEAR);

	//Standardize the brightness and contrast to improve dark images.
	equalizeHist(grayFrame, grayFrame);

	//Compute the detection time
	double t = (double) cvGetTickCount();
	//Detects objects of different sizes in the gray scale image. Then, the detected objects are returned as a list of rectangles.
	faceCascade.detectMultiScale(grayFrame, faces, 1.2, 3,
			0 | CV_HAAR_SCALE_IMAGE, Size(10, 60));
	t = (double) cvGetTickCount() - t;
	detectTime = (int) (t / ((double) cvGetTickFrequency() * 1000.));
	printf("detection time = %g ms\n", detectTime);
	//Get frame properties
	posFrame = ((int) cvGetCaptureProperty(capture, //POS_FRAME is the current position in frame number. It retrieves the current frame number.
			CV_CAP_PROP_POS_FRAMES));
	msecFrame = (int) cvGetCaptureProperty(capture, //POS_MSEC is the current position in a video file, measured in milliseconds.
			CV_CAP_PROP_POS_MSEC);
	printf("posFrame: %d msecFrame: %d ms\n", posFrame, msecFrame);

	fs << "{" << "Frame" << "{";
	fs << "frameId" << posFrame - 1;
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

		string boxText1 = format("posFrame= %d msecFrame= %d ms", posFrame,
				msecFrame);
		putText(frame, boxText1, Point(x + height, y + width),
				FONT_HERSHEY_PLAIN, 1.0, CV_RGB(0,255,0), 2.0);

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
		}

	//Show what you got
	imshow(windowName, frame);

	fs << "]";
	fs << "}" << "}";

}

