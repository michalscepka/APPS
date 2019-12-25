// ***********************************************************************
//
// Demo program for education in subject
// Computer Architectures and Parallel Systems.
// Petr Olivka, dep. of Computer Science, FEI, VSB-TU Ostrava
// email:petr.olivka@vsb.cz
//
// Example of CUDA Technology Usage.
//
// Image interface for CUDA
//
// ***********************************************************************

#pragma once

#include <cuda_device_runtime_api.h>
#include <cuda_runtime.h>

#include "uni_mem_allocator.h"

// Structure definition for exchanging data between Host and Device
struct CudaPic
{
	uint3 m_size;				// size of picture
	union {
		void   *m_p_void;		// data of picture
		uchar1 *m_p_uchar1;	// data of picture
		uchar3 *m_p_uchar3;	// data of picture
		uchar4 *m_p_uchar4;	// data of picture
	};

	__host__ CudaPic() {	}

	__host__ CudaPic(cv::Mat *image)
	{
		m_size.x = image->cols;
		m_size.y = image->rows;
		m_p_void = image->data;
	}

	__device__ __host__  uchar1 &at1(uint32_t y,uint32_t x)
	{
		return m_p_uchar1[y*m_size.x + x];
	}

	__device__ __host__ uchar3 &at3(uint32_t y,uint32_t x)
	{
		return m_p_uchar3[y*m_size.x + x];
	}

	__device__ __host__  uchar4 &at4(uint32_t y,uint32_t x)
	{
		return m_p_uchar4[y*m_size.x + x];
	}
};
