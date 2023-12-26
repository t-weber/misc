/**
 * opencv test 2: mask generation
 * @author Tobias Weber
 * @date 26-December-2023
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *	- https://docs.opencv.org/4.x/index.html
 *	- https://github.com/opencv/opencv
 *
 * g++ -std=c++20 -I/usr/include/opencv4 -o ocv2 ocv2.cpp -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui
 */

#include <array>
#include <tuple>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>



cv::Mat create_mask(const cv::Mat& mat)
{
	cv::Mat thres;
	cv::inRange(mat,
		std::array<std::uint8_t, 3>{0x20 /*h*/, 0x50 /*s*/, 0x10 /*v*/},
		std::array<std::uint8_t, 3>{0xaf /*h*/, 0xaf /*s*/, 0xcf /*v*/},
		thres);

	return thres;
}



cv::Mat clean_mask(const cv::Mat& mat, int size)
{
	cv::Mat cleaned;

	cv::Mat kern = cv::Mat::eye(cv::Size(size, size), CV_8U /*mat.type()*/);
	cv::morphologyEx(mat, cleaned, cv::MORPH_CLOSE, kern);
	cv::morphologyEx(cleaned, cleaned, cv::MORPH_OPEN, kern);

	return cleaned;
}



cv::Mat grey_img(const cv::Mat& mat)
{
	cv::Mat grey;
	cv::cvtColor(mat, grey, cv::COLOR_BGR2GRAY);

	return grey;
}



std::tuple<cv::Mat, std::array<cv::Mat, 3>> hsv_img(const cv::Mat& mat)
{
	cv::Mat hsv_all;
	cv::cvtColor(mat, hsv_all, cv::COLOR_BGR2HSV);

	std::array<cv::Mat, 3> hsv;
	cv::split(hsv_all, hsv);

	return std::make_tuple(hsv_all, std::move(hsv));
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

		// operations on the image
		auto [hsv, hsv_channels] = hsv_img(mat);
		const auto& [h, s, v] = hsv_channels;
		cv::Mat mask = create_mask(hsv);
		cv::Mat cleaned_mask = clean_mask(mask, 5);

		// show images
		cv::imshow("image", mat);
		//cv::imshow("h", h); cv::imshow("s", s); cv::imshow("v", v);
		cv::imshow("mask", mask);
		cv::imshow("cleaned mask", cleaned_mask);

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
