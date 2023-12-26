/**
 * opencv test 3: contour tracing
 * @author Tobias Weber
 * @date 26-December-2023
 * @license see 'LICENSE.GPL' file
 *
 * References:
 *	- https://docs.opencv.org/4.x/index.html
 *	- https://github.com/opencv/opencv
 *
 * g++ -std=c++20 -I/usr/include/opencv4 -o ocv3 ocv3.cpp -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui
 */

#include <array>
#include <vector>
#include <tuple>
#include <functional>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>



cv::Mat create_mask(const cv::Mat& mat)
{
	cv::Mat thres;
	cv::inRange(mat,
		std::array<std::uint8_t, 1>{0x00},
		std::array<std::uint8_t, 1>{0xef},
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
		cv::Mat mat = cv::imread(argv[1], /*cv::IMREAD_COLOR*/ cv::IMREAD_GRAYSCALE);
		std::cout << "Image infos:\n";
		std::cout << "\tElement data type: " << mat.type() << std::endl;
		std::cout << "\tTotal elements: " << mat.total() << std::endl;
		std::cout << "\tChannels: " << mat.channels() << std::endl;
		std::cout << "\tDimensions: " << mat.dims << std::endl;
		std::cout << "\tSizes: " << mat.cols << " x " << mat.rows << std::endl;

		cv::Mat mask = create_mask(mat);
		cv::Mat cleaned_mask = clean_mask(mask, 5);

		// find external contours
		std::vector<std::vector<cv::Point>> contours_ext;
		std::vector<cv::Vec4i> hierarchy_ext;
		cv::findContours(cleaned_mask, contours_ext, hierarchy_ext,
			cv::RETR_EXTERNAL /*cv::RETR_LIST*/ /*cv::RETR_CCOMP*/ /*cv::RETR_TREE*/,
			cv::CHAIN_APPROX_SIMPLE /*cv::CHAIN_APPROX_NONE*/);

		cv::Mat cont_ext(mat.rows, mat.cols, CV_8UC3);
		for(int idx=0; idx<contours_ext.size(); ++idx)
			cv::drawContours(cont_ext, contours_ext, idx, cv::Scalar{0xff, 0x00, 0x00});

		// find contour hierarchy
		std::vector<std::vector<cv::Point>> contours_all;
		std::vector<cv::Vec4i> hierarchy_all; // [ next, prev, child, parent ]
		cv::findContours(cleaned_mask, contours_all, hierarchy_all,
			cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

		cv::Mat cont_all(mat.rows, mat.cols, CV_8UC3);
		std::function<void(int)> draw_hierarchy;
		draw_hierarchy = [&cont_all, &contours_all, &hierarchy_all, &draw_hierarchy](int idx)
		{
			if(idx < 0)
				return;

			// iterate the contours on the same level of the hierarchy
			int next_idx = idx;
			while(next_idx >= 0)
			{
				cv::drawContours(cont_all, contours_all, next_idx,
					cv::Scalar{0xff, 0x00, 0x00},
					1, cv::LINE_8, hierarchy_all);

				// draw child contours if available
				draw_hierarchy(hierarchy_all[next_idx][2]);

				next_idx = hierarchy_all[next_idx][0];
			}
		};

		draw_hierarchy(0);

		// show images
		cv::imshow("image", mat);
		//cv::imshow("mask", mask);
		//cv::imshow("cleaned mask", cleaned_mask);
		cv::imshow("external contours", cont_ext);
		cv::imshow("contour hierarchy", cont_all);

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
