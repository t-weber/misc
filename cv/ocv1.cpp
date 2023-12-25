/**
 * opencv test 1
 * @author Tobias Weber
 * @date 25-December-2023
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *	- https://docs.opencv.org/4.x/index.html
 *	- https://github.com/opencv/opencv
 *
 * g++ -std=c++20 -I/usr/include/opencv4 -o ocv1 ocv1.cpp -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui
 */

#include <array>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>



cv::Mat blur(const cv::Mat& mat, int size)
{
	cv::Mat blurred;
	cv::GaussianBlur(mat, blurred, cv::Size(size, size), 0);

	return blurred;
}



cv::Mat grey(const cv::Mat& mat)
{
	cv::Mat grey;
	cv::cvtColor(mat, grey, cv::COLOR_BGR2GRAY);

	//std::cout << "Grey type: " << grey.type() << std::endl;
	return grey;
}



std::array<cv::Mat, 3> hsv(const cv::Mat& mat)
{
	cv::Mat hsv_all;
	cv::cvtColor(mat, hsv_all, cv::COLOR_BGR2HSV);

	std::array<cv::Mat, 3> hsv;
	cv::split(hsv_all, hsv);

	return hsv;
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
		// load image
		cv::Mat mat = cv::imread(argv[1], cv::IMREAD_COLOR /*cv::IMREAD_GRAYSCALE*/);
		std::cout << "Image infos:\n";
		std::cout << "\tElement data type: " << mat.type() << std::endl;
		std::cout << "\tTotal elements: " << mat.total() << std::endl;
		std::cout << "\tChannels: " << mat.channels() << std::endl;
		std::cout << "\tDimensions: " << mat.dims << std::endl;
		std::cout << "\tSizes: " << mat.rows << " x " << mat.cols << std::endl;
		/*std::cout << "\tSizes: ";
		for(int dim=0; dim<mat.size.dims(); ++dim)
			std::cout << mat.size[dim] << " ";
		std::cout << std::endl;*/

		// some test operations on the image
		cv::Mat blurred = blur(mat, 5);
		cv::Mat g(mat.rows, mat.cols, CV_8U); g = grey(mat);
		auto [h, s, v] = hsv(mat);

		// show images
		cv::imshow("image", mat);
		cv::imshow("blurred", blurred);
		cv::imshow("grey", g);
		cv::imshow("h", h);
		cv::imshow("s", s);
		cv::imshow("v", v);

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
