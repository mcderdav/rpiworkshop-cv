#include <opencv2\opencv.hpp>
#include <opencv2\video\tracking.hpp>
#include <iostream>

#include <math.h>

using namespace std;
using namespace cv;

#define WINDOW_NAME "Blob Detection"
#define SLIDER_WINDOW "Canny Thresholds"
#define POINT_INC 16
#define DELAY 1

int main(int argc, char** argv)
{

	cout << "Press esc to exit" << endl;

	//Create a video stream object
	VideoCapture videoCapture(1);

	//If there is no video stream, exit
	if (!videoCapture.isOpened())
		return -1;

	//Required Materials
	Mat flow, cflow, colorFrame, prevColorFrame, prevFrameCanny;
	UMat greyFrame, prevGreyFrame, uflow;

	//Create a new Window
	namedWindow(WINDOW_NAME);
	namedWindow(SLIDER_WINDOW);

	int cannyUpper = 30;
	int cannyLower = 0;

	createTrackbar("Upper Threshold", SLIDER_WINDOW, &cannyUpper, 255, nullptr);
	createTrackbar("Lower Threshold", SLIDER_WINDOW, &cannyLower, 255, nullptr);

	//Program Loop
	while (true)
	{

		//Get one Frame from the video
		videoCapture >> colorFrame;
		//Convert the frame to grey for processing
		cvtColor(colorFrame, greyFrame, COLOR_BGR2GRAY);

		//If there is a previous frame
		if (!prevGreyFrame.empty())
		{

			//Calculate the optical flow between the previous frame and current frame
			calcOpticalFlowFarneback(prevGreyFrame, greyFrame, uflow, 0.5, 3, 15, 3, 5, 1.2, 0);

			//Perform a Gaussian Blur to prep for canny
			GaussianBlur(prevGreyFrame, prevFrameCanny, Size(7, 7), 1.5, 1.5);
			//Perform the canny operator on the preped frame
			Canny(prevFrameCanny, prevFrameCanny, cannyLower, cannyUpper);
			//Convert Canny output to colour for drawing
			cvtColor(prevFrameCanny, prevFrameCanny, COLOR_GRAY2BGR);

			//copy flowmap uflow to flow for easy access to internal data
			uflow.copyTo(flow);

			//Iterate through each point to draw at
			for (int y = 0; y < colorFrame.rows; y += POINT_INC)
				for (int x = 0; x < colorFrame.cols; x += POINT_INC)
				{
					//Get the flow values at each point
					const Point2f & fxy = flow.at<Point2f>(y, x);

					//Get the scalar value of the motion at the point
					float scalarFlow = sqrt(fxy.x * fxy.x + fxy.y * fxy.y);

					unsigned __int8 red = 255.0 / (1.0 + exp(-scalarFlow / 4 + 2));
					unsigned __int8 blue = 255 - red;

					//Draw the line pointing in the direction of motion
					line(prevFrameCanny, Point(x, y), Point(cvRound(x + fxy.x), cvRound(y + fxy.y)), Scalar(blue, 0, red));

					//Draw the point that we're looking at
					circle(prevFrameCanny, Point(x, y), 2, Scalar(blue, 0, red), -1);
				}

			//Show the image
			imshow(WINDOW_NAME, prevFrameCanny);
		}

		//Wait 1ms and check for escape keypress
		if (waitKey(DELAY) == 27) break;

		//Push the curent frame back to the previous frame
		swap(prevGreyFrame, greyFrame);
		swap(prevColorFrame, colorFrame);
	}
	return 0;
}