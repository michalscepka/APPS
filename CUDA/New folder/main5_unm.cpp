// ***********************************************************************
//
// Demo program for education in subject
// Computer Architectures and Parallel Systems.
// Petr Olivka, dep. of Computer Science, FEI, VSB-TU Ostrava
// email:petr.olivka@vsb.cz
//
// Example of CUDA Technology Usage with unified memory.
//
// Image transformation from RGB to BW schema. 
// Image manipulation is performed by OpenCV library. 
//
// ***********************************************************************

#include <stdio.h>
#include <cuda_device_runtime_api.h>
#include <cuda_runtime.h>
#include <opencv2/opencv.hpp>

#include "uni_mem_allocator.h"
#include "pic_type.h"

// Function prototype from .cu file
void cu_create_chessboard( CudaPic t_pic, int t_square_size );
void cu_create_alphaimg( CudaPic t_pic, uchar3 t_color );
void cu_insertimage( CudaPic t_big_pic, CudaPic t_small_pic, int2 t_position );
void cu_create_flag(CudaPic t_color_pic);
void cu_rotate(CudaPic t_color_pic, CudaPic t_color_pic_rotated);
void cu_insert(CudaPic t_big_pic, CudaPic t_small_pic, int2 t_position);
void cu_cut(CudaPic t_orig_pic, CudaPic t_cut_pic, int2 t_position);
void cu_wave(CudaPic old_pic, CudaPic new_pic);

void test();

int main( int t_numarg, char **t_arg )
{
	// Uniform Memory allocator for Mat
	UniformAllocator allocator;
	cv::Mat::setDefaultAllocator( &allocator );
	printf("Hello world ");

	cv::Mat bgr_img = cv::imread( "french.png", CV_LOAD_IMAGE_COLOR );
	cv::imshow("french", bgr_img);




	cv::Mat img( 200, 350, CV_8UC3 );
	CudaPic l_pic_sign;
	l_pic_sign.m_size.x = img.cols;
	l_pic_sign.m_size.y = img.rows;
	l_pic_sign.m_p_uchar3 = ( uchar3 * ) img.data;

	cu_create_flag(l_pic_sign);

	cv::imshow("Flag", img);

	cv::Mat img_rotated( 350, 250, CV_8UC3 );
	CudaPic l_pic_rotated;
	l_pic_rotated.m_size.x = img_rotated.cols;
	l_pic_rotated.m_size.y = img_rotated.rows;
	l_pic_rotated.m_p_uchar3 = ( uchar3 * ) img_rotated.data;
	cu_rotate(l_pic_sign, l_pic_rotated);
	//cv::imshow("rotated", img_rotated);


	cv::Mat img_big(500,500, CV_8UC3);
	CudaPic l_big_pic;
	l_big_pic.m_size.x = img_big.cols;
	l_big_pic.m_size.y = img_big.rows;
	l_big_pic.m_p_uchar3 = (uchar3*) img_big.data;

	//cu_insert(l_big_pic, l_pic_sign, {10,10});
	//cv::imshow("big pic", img_big);

	cv::Mat img_cut(100,100,CV_8UC3);
	CudaPic l_cut_pic;
	l_cut_pic.m_size.x = img_cut.cols;
	l_cut_pic.m_size.y = img_cut.rows;
	l_cut_pic.m_p_uchar3 = (uchar3*) img_cut.data;
	cu_cut(l_pic_rotated, l_cut_pic, {50,50});
	cv::imshow("cut", img_cut);

	cu_insert(l_big_pic, l_cut_pic, {10,10});
	//cv::imshow("big pic", img_big);

	cv::Mat img_wave(300,350,CV_8UC3);
	CudaPic l_wave_pic;
	l_wave_pic.m_size.x = img_wave.cols;
	l_wave_pic.m_size.y = img_wave.rows;
	l_wave_pic.m_p_uchar3 = (uchar3*) img_wave.data;
	cu_wave(l_pic_sign, l_wave_pic);
	cv::imshow("cut", img_wave);



	/*
	cv::Mat::setDefaultAllocator( &allocator );

	cv::Mat l_cv_chessboard( 511, 515, CV_8UC3 );

	CudaPic l_pic_chessboard;
	l_pic_chessboard.m_size.x = l_cv_chessboard.cols;
	l_pic_chessboard.m_size.y = l_cv_chessboard.rows;
	l_pic_chessboard.m_p_uchar3 = ( uchar3 * ) l_cv_chessboard.data;

	cu_create_chessboard( l_pic_chessboard, 21 );

	cv::imshow( "Chess Board", l_cv_chessboard );

	cv::Mat l_cv_alphaimg( 211, 191, CV_8UC4 );

	CudaPic l_pic_alphaimg;
	l_pic_alphaimg.m_size.x = l_cv_alphaimg.cols;
	l_pic_alphaimg.m_size.y = l_cv_alphaimg.rows;
	l_pic_alphaimg.m_p_uchar4 = ( uchar4 * ) l_cv_alphaimg.data;

	cu_create_alphaimg( l_pic_alphaimg, { 0, 0, 255 } );

	cv::imshow( "Alpha channel", l_cv_alphaimg );

	cu_insertimage( l_pic_chessboard, l_pic_alphaimg, { 11, 23 } );

	cv::imshow( "Result I", l_cv_chessboard );

	// some argument?
	if ( t_numarg > 1 )
	{
		// Load image
		cv::Mat l_cv_img = cv::imread( t_arg[ 1 ], CV_LOAD_IMAGE_UNCHANGED );

		if ( !l_cv_img.data )
			printf( "Unable to read file '%s'\n", t_arg[ 1 ] );

		else if ( l_cv_img.channels() != 4 )
			printf( "Image does not contain alpha channel!\n" );

		else
		{
			// insert loaded image
			CudaPic l_pic_img;
			l_pic_img.m_size.x = l_cv_img.cols;
			l_pic_img.m_size.y = l_cv_img.rows;
			l_pic_img.m_p_uchar4 = ( uchar4 * ) l_cv_img.data;

			cu_insertimage( l_pic_chessboard, l_pic_img, { ( int ) l_pic_chessboard.m_size.x / 2, ( int ) l_pic_chessboard.m_size.y / 2 } );

			cv::imshow( "Result II", l_cv_chessboard );
		}
	}
	*/
	cv::waitKey( 0 );
}

