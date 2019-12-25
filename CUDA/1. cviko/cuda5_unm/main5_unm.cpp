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
void cu_create_target(CudaPic t_pic);
void cu_create_chessboard( CudaPic t_pic, int t_square_size );
void cu_create_alphaimg( CudaPic t_pic, uchar3 t_color );
void cu_insertimage( CudaPic t_big_pic, CudaPic t_small_pic, int2 t_position );

int main( int t_numarg, char **t_arg )
{
	// Uniform Memory allocator for Mat
	UniformAllocator allocator;
	cv::Mat::setDefaultAllocator( &allocator );

	/*cv::Mat l_cv_chessboard( 511, 515, CV_8UC3 );

	CudaPic l_pic_chessboard;
	l_pic_chessboard.m_size.x = l_cv_chessboard.cols;
	l_pic_chessboard.m_size.y = l_cv_chessboard.rows;
	l_pic_chessboard.m_p_uchar3 = ( uchar3 * ) l_cv_chessboard.data;

	cu_create_chessboard( l_pic_chessboard, 21 );

	cv::imshow( "Chess Board", l_cv_chessboard );*/




	cv::Mat cv_platno( 511, 515, CV_8UC3 );

	CudaPic platno;
	platno.m_size.x = cv_platno.cols;
	platno.m_size.y = cv_platno.rows;
	platno.m_p_uchar3 = ( uchar3 * ) cv_platno.data;

	cu_create_target(platno);

	cv::imshow( "Platno", cv_platno );




	/*cv::Mat l_cv_alphaimg( 211, 191, CV_8UC4 );

	CudaPic l_pic_alphaimg;
	l_pic_alphaimg.m_size.x = l_cv_alphaimg.cols;
	l_pic_alphaimg.m_size.y = l_cv_alphaimg.rows;
	l_pic_alphaimg.m_p_uchar4 = ( uchar4 * ) l_cv_alphaimg.data;

	cu_create_alphaimg( l_pic_alphaimg, { 0, 0, 255 } );

	//cv::imshow( "Alpha channel", l_cv_alphaimg );

	cu_insertimage( l_pic_chessboard, l_pic_alphaimg, { 11, 23 } );

	//cv::imshow( "Result I", l_cv_chessboard );

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

			//cv::imshow( "Result II", l_cv_chessboard );
		}
	}*/

	cv::waitKey( 0 );
}

