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


__global__ void kernel_flag (CudaPic t_color_pic )
{
	// X,Y coordinates and check image dimensions
	int l_y = blockDim.y * blockIdx.y + threadIdx.y;
	int l_x = blockDim.x * blockIdx.x + threadIdx.x;
	if ( l_y >= t_color_pic.m_size.y ) return;
	if ( l_x >= t_color_pic.m_size.x ) return;
	/*
	int r = 50;
	int sx = 150 - l_x;
	int sy = 75 - l_y;

	uchar3 l_bgr = { 255, 255, 255 };

	if (sx * sx + sy*sy >= r*r -70 && sx * sx + sy*sy <= r*r + 70)
		l_bgr = { 0, 0, 0 };
	*/
	uchar3 l_bgr = { 255, 255, 255 };
	if (l_x < 125){
		l_bgr = { 255, 0, 0 };

	}
	if (l_x > 250)
		l_bgr = { 0, 0, 255 };

	// Store point into image
	t_color_pic.m_p_uchar3[ l_y * t_color_pic.m_size.x + l_x ] = l_bgr;
}

void cu_create_flag(CudaPic t_color_pic)
{
	cudaError_t l_cerr;
	dim3 blocks (35,20);
	dim3 threads (10,10);
	kernel_flag<<<blocks, threads>>>( t_color_pic );
	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );

	cudaDeviceSynchronize();
}

__global__ void kernel_rotate(CudaPic t_color_pic, CudaPic t_color_pic_rotated)
{
	int l_y = blockDim.y * blockIdx.y + threadIdx.y;
	int l_x = blockDim.x * blockIdx.x + threadIdx.x;
	if ( l_y >= t_color_pic.m_size.y ) return;
	if ( l_x >= t_color_pic.m_size.x ) return;

	int newX = t_color_pic_rotated.m_size.x - l_y;
	int newY = l_x;

	t_color_pic_rotated.m_p_uchar3[ newY * t_color_pic_rotated.m_size.x +  newX] = t_color_pic.m_p_uchar3[ l_y * t_color_pic.m_size.x + l_x ];
}

void cu_rotate(CudaPic t_color_pic, CudaPic t_color_pic_rotated)
{
	cudaError_t l_cerr;
	dim3 blocks (35,20);
	dim3 threads (10,10);
	kernel_rotate<<<blocks, threads>>>( t_color_pic, t_color_pic_rotated );
	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );

	cudaDeviceSynchronize();
}

// -----------------------------------------------------------------------------------------------


__global__ void kernel_insert(CudaPic t_big_pic, CudaPic t_small_pic, int2 t_position)
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

	t_big_pic.m_p_uchar3[ l_by * t_big_pic.m_size.x + l_bx ] = t_small_pic.m_p_uchar3[ l_y * t_small_pic.m_size.x + l_x ];
}

void cu_insert( CudaPic t_big_pic, CudaPic t_small_pic, int2 t_position )
{
	cudaError_t l_cerr;
	dim3 blocks (t_small_pic.m_size.x/10, t_small_pic.m_size.y/10);
	dim3 threads (10,10);
	kernel_insert<<<blocks, threads>>>( t_big_pic, t_small_pic, t_position );
	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );

	cudaDeviceSynchronize();
}

__global__ void kernel_wave(CudaPic old_pic, CudaPic new_pic)
{
	int l_y = blockDim.y * blockIdx.y + threadIdx.y;
	int l_x = blockDim.x * blockIdx.x + threadIdx.x;
	if ( l_y >= old_pic.m_size.y ) return;
	if ( l_x >= old_pic.m_size.x ) return;

	float a = l_x;
	int l_ny = l_y +50 - sin(a/20)*20;
	int l_nx = l_x;
	int colourscale = (1 - sin(a/20))*70;
	if ( l_ny >= new_pic.m_size.y || l_ny < 0 ) return;
	if ( l_nx >= new_pic.m_size.x || l_nx < 0 ) return;

	new_pic.m_p_uchar3[ l_ny * new_pic.m_size.x + l_nx ] = old_pic.m_p_uchar3[ l_y * old_pic.m_size.x + l_x ];
	if(new_pic.m_p_uchar3[ l_ny * new_pic.m_size.x + l_nx ].x > colourscale)
		new_pic.m_p_uchar3[ l_ny * new_pic.m_size.x + l_nx ].x -= colourscale;
	if(new_pic.m_p_uchar3[ l_ny * new_pic.m_size.x + l_nx ].y > colourscale)
		new_pic.m_p_uchar3[ l_ny * new_pic.m_size.x + l_nx ].y -= colourscale;
	if(new_pic.m_p_uchar3[ l_ny * new_pic.m_size.x + l_nx ].z > colourscale)
		new_pic.m_p_uchar3[ l_ny * new_pic.m_size.x + l_nx ].z -= colourscale;

}

void cu_wave(CudaPic old_pic, CudaPic new_pic)
{
	cudaError_t l_cerr;
	dim3 blocks (old_pic.m_size.x/10, old_pic.m_size.y/10);
	dim3 threads (10,10);
	kernel_wave<<<blocks, threads>>>( old_pic, new_pic);
	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );
	cudaDeviceSynchronize();
}

__global__ void kernel_cut(CudaPic t_orig_pic, CudaPic t_cut_pic, int2 t_position)
{
	int l_y = blockDim.y * blockIdx.y + threadIdx.y;
	int l_x = blockDim.x * blockIdx.x + threadIdx.x;
	if ( l_y >= t_cut_pic.m_size.y ) return;
	if ( l_x >= t_cut_pic.m_size.x ) return;
	int l_oy = l_y + t_position.y;
	int l_ox = l_x + t_position.x;
	if ( l_oy >= t_orig_pic.m_size.y || l_oy < 0 ) return;
	if ( l_ox >= t_orig_pic.m_size.x || l_ox < 0 ) return;

	t_cut_pic.m_p_uchar3[ l_y * t_cut_pic.m_size.x + l_x ] = t_orig_pic.m_p_uchar3[ l_oy * t_orig_pic.m_size.x + l_ox ];
}


void cu_cut(CudaPic t_orig_pic, CudaPic t_cut_pic, int2 t_position)
{
	cudaError_t l_cerr;
	dim3 blocks (t_cut_pic.m_size.x/10, t_cut_pic.m_size.y/10);
	dim3 threads (10,10);
	kernel_cut<<<blocks, threads>>>( t_orig_pic, t_cut_pic, t_position );
	if ( ( l_cerr = cudaGetLastError() ) != cudaSuccess )
		printf( "CUDA Error [%d] - '%s'\n", __LINE__, cudaGetErrorString( l_cerr ) );

	cudaDeviceSynchronize();
}

__global__ void resize(CudaPic t_orig_pic, CudaPic t_resized_pic)
{

}

void cu_resize(CudaPic t_orig_pic, CudaPic t_resized_pic)
{

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
