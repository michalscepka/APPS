// ***********************************************************************
//
// Demo program for education in subject
// Computer Architectures and Parallel Systems.
// Petr Olivka, dep. of Computer Science, FEI, VSB-TU Ostrava
// email:petr.olivka@vsb.cz
//
// Example of CUDA Technology Usage.
//
// Simple animation.
//
// ***********************************************************************

#include "pic_type.h"

class Animation
{
public:
	CudaPic m_cuda_bg_pic, m_cuda_ins_pic, m_cuda_res_pic;
	int m_initialized;

	Animation() : m_initialized( 0 ) {}

	void start( CudaPic t_bg_pic, CudaPic t_ins_pic );

	void next( CudaPic t_res_pic, int2 t_position );

	void stop();

};
