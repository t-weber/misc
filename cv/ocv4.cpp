/**
 * opencv test 4: object detection
 * @author Tobias Weber
 * @date 27-December-2023
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *	- https://docs.opencv.org/4.x/index.html
 *	- https://github.com/opencv/opencv
 *
 * g++ -std=c++20 -I/usr/include/opencv4 -o ocv4 ocv4.cpp -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_objdetect -lopencv_highgui
 */

#include <vector>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>



cv::Mat grey_img(const cv::Mat& mat)
{
	cv::Mat grey;
	cv::cvtColor(mat, grey, cv::COLOR_BGR2GRAY);

	return grey;
}



int main(int argc, char** argv)
{
	if(argc < 2)
	{
		std::cerr << "Please specify an image." << std::endl;
		return -1;
	}

	try
	{
		// load cascade classifier
		const char *casc_file = "/usr/share/opencv4/haarcascades/haarcascade_eye.xml";
		//const char *casc_file = "/usr/share/opencv4/haarcascades/haarcascade_fullbody.xml";
		cv::CascadeClassifier casc{casc_file};
		if(casc.empty())
		{
			std::cerr << "Error: Could not load cascade classifier." << std::endl;
			return -1;
		}

		// load image
		cv::Mat mat = cv::imread(argv[1], cv::IMREAD_COLOR /*cv::IMREAD_GRAYSCALE*/);
		cv::Mat grey = grey_img(mat);
		std::cout << "Image infos:\n";
		std::cout << "\tElement data type: " << mat.type() << std::endl;
		std::cout << "\tTotal elements: " << mat.total() << std::endl;
		std::cout << "\tChannels: " << mat.channels() << std::endl;
		std::cout << "\tDimensions: " << mat.dims << std::endl;
		std::cout << "\tSizes: " << mat.cols << " x " << mat.rows << std::endl;

		// run cascade classifier
		std::vector<cv::Rect> all_rects;
		casc.detectMultiScale(grey, all_rects, 1.25 /*scale*/, 3 /*neighbours*/);

		std::cout << "\nDetected: " << all_rects.size() << std::endl;
		for(const cv::Rect& rect : all_rects)
			cv::rectangle(mat, rect, cv::Scalar{0xff, 0x00, 0x00});

		// show image
		cv::imshow("image", mat);

		// close windows
		cv::waitKey(0);
		cv::destroyAllWindows();
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return -1;
	}

	return 0;
}
