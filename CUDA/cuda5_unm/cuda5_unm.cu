// ***********************************************************************
//
// Demo program for education in subject
// Computer Architectures and Parallel Systems.
// Petr Olivka, dep. of Computer Science, FEI, VSB-TU Ostrava
// email:petr.olivka@vsb.cz
//
// Example of CUDA Technology Usage wit unified memory.
// Image transformation from RGB to BW schema. 
//
// ***********************************************************************

#include <stdio.h>
#include <cuda_device_runtime_api.h>
#include <cuda_runtime.h>

#include "pic_type.h"

// Demo kernel to create chess board
__global__ void kernel_target( CudaPic picture )
{
	// X,Y coordinates and check image dimensions
	int y = blockDim.y * blockIdx.y + threadIdx.y;
	int x = blockDim.x * blockIdx.x + threadIdx.x;
	if ( y >= picture.m_size.y ) return;
	if ( x >= picture.m_size.x ) return;

	int centerx = picture.m_size.x / 2;
	int centery = picture.m_size.y / 2;
	//picture.at3(centerx, centery) = { 0, 0, 0 };

	int distance = sqrt(pow(x - centerx, 2) + pow(y - centery, 2));

	//blue green red
	if(distance <= 50)
		picture.at3(x, y) = { 0, 255, 255 };

	if(distance > 50)
		picture.at3(x, y) = { 0, 0, 255 };

	if(distance > 100)
		picture.at3(x, y) = { 255, 0, 0 };

	if(distance > 150)
		picture.at3(x, y) = { 0, 0, 0 };

	if(distance > 200)
		picture.at3(x, y) = { 255, 255, 255 };
}

void cu_create_target(CudaPic t_color_pic)
{
	cudaError_t l_cerr;

	// Grid creation, size of grid must be equal or greater than images
	int l_block_size = 32;
	dim3 l_blocks( ( t_color_pic.m_size.x + l_block_size - 1 ) / l_block_size,
				   ( t_color_pic.m_size.y + l_block_size - 1 ) / l_block_size );
	dim3 l_threads( l_block_size, l_block_size );
	kernel_target<<< l_blocks, l_threads >>>( t_color_pic );

	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );

	cudaDeviceSynchronize();
}

// -----------------------------------------------------------------------------------------------

// Demo kernel to create chess board
__global__ void kernel_chessboard( CudaPic t_color_pic )
{
	// X,Y coordinates and check image dimensions
	int l_y = blockDim.y * blockIdx.y + threadIdx.y;
	int l_x = blockDim.x * blockIdx.x + threadIdx.x;
	if ( l_y >= t_color_pic.m_size.y ) return;
	if ( l_x >= t_color_pic.m_size.x ) return;

	unsigned char b_or_w = 255 * ( ( blockIdx.x + blockIdx.y ) & 1 );

	// Store point into image
	t_color_pic.m_p_uchar3[ l_y * t_color_pic.m_size.x + l_x ] = { b_or_w, b_or_w, b_or_w };
}

void cu_create_chessboard( CudaPic t_color_pic, int t_square_size )
{
	cudaError_t l_cerr;

	// Grid creation, size of grid must be equal or greater than images
	dim3 l_blocks( ( t_color_pic.m_size.x + t_square_size - 1 ) / t_square_size,
			       ( t_color_pic.m_size.y + t_square_size - 1 ) / t_square_size );
	dim3 l_threads( t_square_size, t_square_size );
	kernel_chessboard<<< l_blocks, l_threads >>>( t_color_pic );

	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );

	cudaDeviceSynchronize();
}

// -----------------------------------------------------------------------------------------------

// Demo kernel to create picture with alpha channel gradient
__global__ void kernel_alphaimg( CudaPic t_color_pic, uchar3 t_color )
{
	// X,Y coordinates and check image dimensions
	int l_y = blockDim.y * blockIdx.y + threadIdx.y;
	int l_x = blockDim.x * blockIdx.x + threadIdx.x;
	if ( l_y >= t_color_pic.m_size.y ) return;
	if ( l_x >= t_color_pic.m_size.x ) return;

	int l_diagonal = sqrtf( t_color_pic.m_size.x * t_color_pic.m_size.x + t_color_pic.m_size.y * t_color_pic.m_size.y );
	int l_dx = l_x - t_color_pic.m_size.x / 2;
	int l_dy = l_y - t_color_pic.m_size.y / 2;
	int l_dxy = sqrtf( l_dx * l_dx + l_dy * l_dy ) - l_diagonal / 2;

	// Store point into image
	t_color_pic.m_p_uchar4[ l_y * t_color_pic.m_size.x + l_x ] =
		{ t_color.x, t_color.y, t_color.z, ( unsigned char ) ( 255 - 255 * l_dxy / ( l_diagonal / 2 ) ) };
}

void cu_create_alphaimg( CudaPic t_color_pic, uchar3 t_color )
{
	cudaError_t l_cerr;

	// Grid creation, size of grid must be equal or greater than images
	int l_block_size = 32;
	dim3 l_blocks( ( t_color_pic.m_size.x + l_block_size - 1 ) / l_block_size,
			       ( t_color_pic.m_size.y + l_block_size - 1 ) / l_block_size );
	dim3 l_threads( l_block_size, l_block_size );
	kernel_alphaimg<<< l_blocks, l_threads >>>( t_color_pic, t_color );

	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );

	cudaDeviceSynchronize();
}

// -----------------------------------------------------------------------------------------------

// Demo kernel to create picture with alpha channel gradient
__global__ void kernel_insertimage( CudaPic t_big_pic, CudaPic t_small_pic, int2 t_position )
{
	// X,Y coordinates and check image dimensions
	int l_y = blockDim.y * blockIdx.y + threadIdx.y;
	int l_x = blockDim.x * blockIdx.x + threadIdx.x;
	if ( l_y >= t_small_pic.m_size.y ) return;
	if ( l_x >= t_small_pic.m_size.x ) return;
	int l_by = l_y + t_position.y;
	int l_bx = l_x + t_position.x;
	if ( l_by >= t_big_pic.m_size.y || l_by < 0 ) return;
	if ( l_bx >= t_big_pic.m_size.x || l_bx < 0 ) return;

	// Get point from small image
	uchar4 l_fg_bgra = t_small_pic.m_p_uchar4[ l_y * t_small_pic.m_size.x + l_x ];
	uchar3 l_bg_bgr = t_big_pic.m_p_uchar3[ l_by * t_big_pic.m_size.x + l_bx ];
	uchar3 l_bgr = { 0, 0, 0 };

	// compose point from small and big image according alpha channel
	l_bgr.x = l_fg_bgra.x * l_fg_bgra.w / 255 + l_bg_bgr.x * ( 255 - l_fg_bgra.w ) / 255;
	l_bgr.y = l_fg_bgra.y * l_fg_bgra.w / 255 + l_bg_bgr.y * ( 255 - l_fg_bgra.w ) / 255;
	l_bgr.z = l_fg_bgra.z * l_fg_bgra.w / 255 + l_bg_bgr.z * ( 255 - l_fg_bgra.w ) / 255;

	// Store point into image
	t_big_pic.m_p_uchar3[ l_by * t_big_pic.m_size.x + l_bx ] = l_bgr;
}

void cu_insertimage( CudaPic t_big_pic, CudaPic t_small_pic, int2 t_position )
{
	cudaError_t l_cerr;

	// Grid creation, size of grid must be equal or greater than images
	int l_block_size = 32;
	dim3 l_blocks( ( t_small_pic.m_size.x + l_block_size - 1 ) / l_block_size,
			       ( t_small_pic.m_size.y + l_block_size - 1 ) / l_block_size );
	dim3 l_threads( l_block_size, l_block_size );
	kernel_insertimage<<< l_blocks, l_threads >>>( t_big_pic, t_small_pic, t_position );

	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );

	cudaDeviceSynchronize();
}
