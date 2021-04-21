/* Copyright © 2017-2020 ABBYY Production LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
--------------------------------------------------------------------------------------------------------------*/

#pragma once

#include "MatrixMultiplier.h"

// The kernel is chosen depending on the architecture
// The kernel header files do not have these ifdef, so they may not be included in the project without a pre-check
// We use relative paths for #include "" which lets you store the multiplication code in a separate project
// This may simplify testing the performance times

#ifdef __aarch64__

#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM64_8x12.h"
#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM64_8x4.h"
#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM64_8x1.h"
#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM64_4x12.h"
#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM64_4x4.h"
#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM64_4x1.h"
#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM64_1x12.h"
#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM64_1x4.h"
#include "CPU/arm/MatrixMultiplyingInterleaved/Interleavers/Interleaver_ARM64.h"
using CMicroKernel1x1 = CMicroKernelBase<1, 1>;
using CMicroKernelDefault = CKernelCombineVertical<
	CKernelCombineHorizontal<CMicroKernel8x12, CMicroKernel8x4, CMicroKernel8x1>,
	CKernelCombineHorizontal<CMicroKernel4x12, CMicroKernel4x4, CMicroKernel4x1>,
	CKernelCombineHorizontal<CMicroKernel1x12, CMicroKernel1x4, CMicroKernel1x1>
>;
template <bool Transpose, size_t Len> using CInterleaverDefault = CInterleaver<Transpose, Len>;

#elif __arm__ && __ARM_NEON

#include "CPU/arm/MatrixMultiplyingInterleaved/MicroKernels/Kernel_ARM32NEON_6x8.h"
#include "Interleavers/InterleaverBase.h"
using CMicroKernelDefault = CMicroKernel6x8;
template <bool Transpose, size_t Len> using CInterleaverDefault = CInterleaverBase<Transpose, Len>;

#else

#include "MicroKernels/MicroKernelBase.h"
#include "Interleavers/InterleaverBase.h"
using CMicroKernelDefault = CMicroKernelBase<1, 1>;
template <bool Transpose, size_t Len> using CInterleaverDefault = CInterleaverBase<Transpose, Len>;

inline __m256 _mm256_loadu2_m128 ( float const *hiAddr, float const *loAddr )
{
  return _mm256_insertf128_ps( _mm256_castps128_ps256( _mm_loadu_ps ( loAddr ) ), _mm_loadu_ps( hiAddr ), 1 );
}
inline void _mm256_storeu2_m128( float *hiAddr, float *loAddr, __m256 data )
{
  _mm_storeu_ps ( loAddr, _mm256_castps256_ps128( data ) );
  _mm_storeu_ps ( hiAddr, _mm256_extractf128_ps( data, 1) );
}

struct CMicroKernel_6x16 : public CMicroKernelBase<6, 16> {
	static void Calculate( const float* aPtr, const float* bPtr, float* cPtr, size_t cRowSize, size_t k ) {
		_mm_prefetch( cPtr + 0 * cRowSize, _MM_HINT_T0 );
		_mm_prefetch( cPtr + 1 * cRowSize, _MM_HINT_T0 );
		_mm_prefetch( cPtr + 2 * cRowSize, _MM_HINT_T0 );
		_mm_prefetch( cPtr + 3 * cRowSize, _MM_HINT_T0 );
		_mm_prefetch( cPtr + 4 * cRowSize, _MM_HINT_T0 );
		_mm_prefetch( cPtr + 5 * cRowSize, _MM_HINT_T0 );
		__m256 c00 = _mm256_setzero_ps();
		__m256 c01 = _mm256_setzero_ps();
		__m256 c10 = _mm256_setzero_ps();
		__m256 c11 = _mm256_setzero_ps();
		__m256 c20 = _mm256_setzero_ps();
		__m256 c21 = _mm256_setzero_ps();
		__m256 c30 = _mm256_setzero_ps();
		__m256 c31 = _mm256_setzero_ps();
		__m256 c40 = _mm256_setzero_ps();
		__m256 c41 = _mm256_setzero_ps();
		__m256 c50 = _mm256_setzero_ps();
		__m256 c51 = _mm256_setzero_ps();

		__m256 b0, b1, a0, a1;

		for( ; k >= 4; k -= 4 ) {
			// Iteration 0
			_mm_prefetch( aPtr + 80, _MM_HINT_T0 );
			b0 = _mm256_loadu_ps( bPtr + 0 );
			b1 = _mm256_loadu_ps( bPtr + 8 );
			a0 = _mm256_broadcast_ss( aPtr + 0 );
			a1 = _mm256_broadcast_ss( aPtr + 1 );
			c00 = _mm256_fmadd_ps( a0, b0, c00 );
			c01 = _mm256_fmadd_ps( a0, b1, c01 );
			c10 = _mm256_fmadd_ps( a1, b0, c10 );
			c11 = _mm256_fmadd_ps( a1, b1, c11 );

			a0 = _mm256_broadcast_ss( aPtr + 2 );
			a1 = _mm256_broadcast_ss( aPtr + 3 );
			c20 = _mm256_fmadd_ps( a0, b0, c20 );
			c21 = _mm256_fmadd_ps( a0, b1, c21 );
			c30 = _mm256_fmadd_ps( a1, b0, c30 );
			c31 = _mm256_fmadd_ps( a1, b1, c31 );

			a0 = _mm256_broadcast_ss( aPtr + 4 );
			a1 = _mm256_broadcast_ss( aPtr + 5 );
			c40 = _mm256_fmadd_ps( a0, b0, c40 );
			c41 = _mm256_fmadd_ps( a0, b1, c41 );
			c50 = _mm256_fmadd_ps( a1, b0, c50 );
			c51 = _mm256_fmadd_ps( a1, b1, c51 );

			// Iteration 1
			b0 = _mm256_loadu_ps( bPtr + 16 );
			b1 = _mm256_loadu_ps( bPtr + 24 );
			a0 = _mm256_broadcast_ss( aPtr + 6 );
			a1 = _mm256_broadcast_ss( aPtr + 7 );
			c00 = _mm256_fmadd_ps( a0, b0, c00 );
			c01 = _mm256_fmadd_ps( a0, b1, c01 );
			c10 = _mm256_fmadd_ps( a1, b0, c10 );
			c11 = _mm256_fmadd_ps( a1, b1, c11 );

			a0 = _mm256_broadcast_ss( aPtr + 8 );
			a1 = _mm256_broadcast_ss( aPtr + 9 );
			c20 = _mm256_fmadd_ps( a0, b0, c20 );
			c21 = _mm256_fmadd_ps( a0, b1, c21 );
			c30 = _mm256_fmadd_ps( a1, b0, c30 );
			c31 = _mm256_fmadd_ps( a1, b1, c31 );

			a0 = _mm256_broadcast_ss( aPtr + 10 );
			a1 = _mm256_broadcast_ss( aPtr + 11 );
			c40 = _mm256_fmadd_ps( a0, b0, c40 );
			c41 = _mm256_fmadd_ps( a0, b1, c41 );
			c50 = _mm256_fmadd_ps( a1, b0, c50 );
			c51 = _mm256_fmadd_ps( a1, b1, c51 );

			// Iteration 2
			_mm_prefetch( aPtr + 76, _MM_HINT_T0 );
			b0 = _mm256_loadu_ps( bPtr + 32 );
			b1 = _mm256_loadu_ps( bPtr + 40 );
			a0 = _mm256_broadcast_ss( aPtr +12 );
			a1 = _mm256_broadcast_ss( aPtr + 13 );
			c00 = _mm256_fmadd_ps( a0, b0, c00 );
			c01 = _mm256_fmadd_ps( a0, b1, c01 );
			c10 = _mm256_fmadd_ps( a1, b0, c10 );
			c11 = _mm256_fmadd_ps( a1, b1, c11 );

			a0 = _mm256_broadcast_ss( aPtr + 14 );
			a1 = _mm256_broadcast_ss( aPtr + 15 );
			c20 = _mm256_fmadd_ps( a0, b0, c20 );
			c21 = _mm256_fmadd_ps( a0, b1, c21 );
			c30 = _mm256_fmadd_ps( a1, b0, c30 );
			c31 = _mm256_fmadd_ps( a1, b1, c31 );

			a0 = _mm256_broadcast_ss( aPtr + 16 );
			a1 = _mm256_broadcast_ss( aPtr + 17 );
			c40 = _mm256_fmadd_ps( a0, b0, c40 );
			c41 = _mm256_fmadd_ps( a0, b1, c41 );
			c50 = _mm256_fmadd_ps( a1, b0, c50 );
			c51 = _mm256_fmadd_ps( a1, b1, c51 );

			// Iteration 3
			b0 = _mm256_loadu_ps( bPtr + 48 );
			b1 = _mm256_loadu_ps( bPtr + 56 );
			a0 = _mm256_broadcast_ss( aPtr + 18 );
			a1 = _mm256_broadcast_ss( aPtr + 19 );
			c00 = _mm256_fmadd_ps( a0, b0, c00 );
			c01 = _mm256_fmadd_ps( a0, b1, c01 );
			c10 = _mm256_fmadd_ps( a1, b0, c10 );
			c11 = _mm256_fmadd_ps( a1, b1, c11 );

			a0 = _mm256_broadcast_ss( aPtr + 20 );
			a1 = _mm256_broadcast_ss( aPtr + 21 );
			c20 = _mm256_fmadd_ps( a0, b0, c20 );
			c21 = _mm256_fmadd_ps( a0, b1, c21 );
			c30 = _mm256_fmadd_ps( a1, b0, c30 );
			c31 = _mm256_fmadd_ps( a1, b1, c31 );

			a0 = _mm256_broadcast_ss( aPtr + 22 );
			a1 = _mm256_broadcast_ss( aPtr + 23 );
			c40 = _mm256_fmadd_ps( a0, b0, c40 );
			c41 = _mm256_fmadd_ps( a0, b1, c41 );
			c50 = _mm256_fmadd_ps( a1, b0, c50 );
			c51 = _mm256_fmadd_ps( a1, b1, c51 );


			bPtr += 64; aPtr += 24;
		}

		for( ; k > 0; k-- ) {
			b0 = _mm256_loadu_ps( bPtr + 0 );
			b1 = _mm256_loadu_ps( bPtr + 8 );
			a0 = _mm256_broadcast_ss( aPtr + 0 );
			a1 = _mm256_broadcast_ss( aPtr + 1 );
			c00 = _mm256_fmadd_ps( a0, b0, c00 );
			c01 = _mm256_fmadd_ps( a0, b1, c01 );
			c10 = _mm256_fmadd_ps( a1, b0, c10 );
			c11 = _mm256_fmadd_ps( a1, b1, c11 );

			a0 = _mm256_broadcast_ss( aPtr + 2 );
			a1 = _mm256_broadcast_ss( aPtr + 3 );
			c20 = _mm256_fmadd_ps( a0, b0, c20 );
			c21 = _mm256_fmadd_ps( a0, b1, c21 );
			c30 = _mm256_fmadd_ps( a1, b0, c30 );
			c31 = _mm256_fmadd_ps( a1, b1, c31 );

			a0 = _mm256_broadcast_ss( aPtr + 4 );
			a1 = _mm256_broadcast_ss( aPtr + 5 );
			c40 = _mm256_fmadd_ps( a0, b0, c40 );
			c41 = _mm256_fmadd_ps( a0, b1, c41 );
			c50 = _mm256_fmadd_ps( a1, b0, c50 );
			c51 = _mm256_fmadd_ps( a1, b1, c51 );

			bPtr += 16; aPtr += 6;
		}

		_mm256_storeu_ps( cPtr, _mm256_add_ps( c00, _mm256_loadu_ps( cPtr ) ) );
		_mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c01, _mm256_loadu_ps( cPtr + 8 ) ) );
		cPtr += cRowSize;
		_mm256_storeu_ps( cPtr, _mm256_add_ps( c10, _mm256_loadu_ps( cPtr ) ) );
		_mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c11, _mm256_loadu_ps( cPtr + 8 ) ) );
		cPtr += cRowSize;
		_mm256_storeu_ps( cPtr, _mm256_add_ps( c20, _mm256_loadu_ps( cPtr ) ) );
		_mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c21, _mm256_loadu_ps( cPtr + 8 ) ) );
		cPtr += cRowSize;
		_mm256_storeu_ps( cPtr, _mm256_add_ps( c30, _mm256_loadu_ps( cPtr ) ) );
		_mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c31, _mm256_loadu_ps( cPtr + 8 ) ) );
		cPtr += cRowSize;
		_mm256_storeu_ps( cPtr, _mm256_add_ps( c40, _mm256_loadu_ps( cPtr ) ) );
		_mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c41, _mm256_loadu_ps( cPtr + 8 ) ) );
		cPtr += cRowSize;
		_mm256_storeu_ps( cPtr, _mm256_add_ps( c50, _mm256_loadu_ps( cPtr ) ) );
		_mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c51, _mm256_loadu_ps( cPtr + 8 ) ) );
	}
};

struct CMicroKernel_96x1 : public CMicroKernelBase<96, 1> {
	static void Calculate( const float* aPtr, const float* bPtr, float* cPtr, size_t cRowSize, size_t k ) {
				__m256 c0 = _mm256_setzero_ps();
				__m256 c1 = _mm256_setzero_ps();
				__m256 c2 = _mm256_setzero_ps();
				__m256 c3 = _mm256_setzero_ps();
				__m256 c4 = _mm256_setzero_ps();
				__m256 c5 = _mm256_setzero_ps();
				__m256 c6 = _mm256_setzero_ps();
				__m256 c7 = _mm256_setzero_ps();
				__m256 c8 = _mm256_setzero_ps();
				__m256 c9 = _mm256_setzero_ps();
				__m256 c10 = _mm256_setzero_ps();
				__m256 c11 = _mm256_setzero_ps();
				__m256 b, a0, a1, a2;
				for( int l = 0; l < k; l++ ) {
					b = _mm256_broadcast_ss( bPtr );
					a0 = _mm256_loadu_ps( aPtr + 0 );
					a1 = _mm256_loadu_ps( aPtr + 8 );
					a2 = _mm256_loadu_ps( aPtr + 16 );
					c0 = _mm256_fmadd_ps( a0, b, c0 );
					c1 = _mm256_fmadd_ps( a1, b, c1 );
					c2 = _mm256_fmadd_ps( a2, b, c2 );

					a0 = _mm256_loadu_ps( aPtr + 24 );
					a1 = _mm256_loadu_ps( aPtr + 32 );
					a2 = _mm256_loadu_ps( aPtr + 40 );
					c3 = _mm256_fmadd_ps( a0, b, c3 );
					c4 = _mm256_fmadd_ps( a1, b, c4 );
					c5 = _mm256_fmadd_ps( a2, b, c5 );

					a0 = _mm256_loadu_ps( aPtr + 48 );
					a1 = _mm256_loadu_ps( aPtr + 56 );
					a2 = _mm256_loadu_ps( aPtr + 64 );
					c6 = _mm256_fmadd_ps( a0, b, c6 );
					c7 = _mm256_fmadd_ps( a1, b, c7 );
					c8 = _mm256_fmadd_ps( a2, b, c8 );

					a0 = _mm256_loadu_ps( aPtr + 72 );
					a1 = _mm256_loadu_ps( aPtr + 80 );
					a2 = _mm256_loadu_ps( aPtr + 88 );
					c9 = _mm256_fmadd_ps( a0, b, c9 );
					c10 = _mm256_fmadd_ps( a1, b, c10 );
					c11 = _mm256_fmadd_ps( a2, b, c11 );

					bPtr++; aPtr += 96;
				}

				_mm256_storeu_ps( cPtr + 0, _mm256_add_ps( c0, _mm256_loadu_ps( cPtr + 0 ) ) );
				_mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c1, _mm256_loadu_ps( cPtr + 8 ) ) );
				_mm256_storeu_ps( cPtr + 16, _mm256_add_ps( c2, _mm256_loadu_ps( cPtr + 16 ) ) );
				_mm256_storeu_ps( cPtr + 24, _mm256_add_ps( c3, _mm256_loadu_ps( cPtr + 24 ) ) );
				_mm256_storeu_ps( cPtr + 32, _mm256_add_ps( c4, _mm256_loadu_ps( cPtr + 32 ) ) );
				_mm256_storeu_ps( cPtr + 40, _mm256_add_ps( c5, _mm256_loadu_ps( cPtr + 40 ) ) );
				_mm256_storeu_ps( cPtr + 48, _mm256_add_ps( c6, _mm256_loadu_ps( cPtr + 48 ) ) );
				_mm256_storeu_ps( cPtr + 56, _mm256_add_ps( c7, _mm256_loadu_ps( cPtr + 56 ) ) );
				_mm256_storeu_ps( cPtr + 64, _mm256_add_ps( c8, _mm256_loadu_ps( cPtr + 64 ) ) );
				_mm256_storeu_ps( cPtr + 72, _mm256_add_ps( c9, _mm256_loadu_ps( cPtr + 72 ) ) );
				_mm256_storeu_ps( cPtr + 80, _mm256_add_ps( c10, _mm256_loadu_ps( cPtr + 80 ) ) );
				_mm256_storeu_ps( cPtr + 88, _mm256_add_ps( c11, _mm256_loadu_ps( cPtr + 88) ) );
	}
};

struct CMicroKernel_6x8 : public CMicroKernelBase<6, 8> {
	static void Calculate( const float* aPtr, const float* bPtr, float* cPtr, size_t cRowSize, size_t k ) {
				__m256 c0 = _mm256_setzero_ps();
				__m256 c1 = _mm256_setzero_ps();
				__m256 c2 = _mm256_setzero_ps();
				__m256 c3 = _mm256_setzero_ps();
				__m256 c4 = _mm256_setzero_ps();
				__m256 c5 = _mm256_setzero_ps();

				__m256 b, a0, a1, a2, a3, a4, a5;

				for( ; k >= 4; k -= 4 ) {
					b = _mm256_loadu_ps( bPtr );
					a0 = _mm256_broadcast_ss( aPtr );
					a1 = _mm256_broadcast_ss( aPtr + 1 );
					a2 = _mm256_broadcast_ss( aPtr + 2 );
					a3 = _mm256_broadcast_ss( aPtr + 3 );
					a4 = _mm256_broadcast_ss( aPtr + 4 );
					a5 = _mm256_broadcast_ss( aPtr + 5 );

					c0 = _mm256_fmadd_ps( a0, b, c0 );
					c1 = _mm256_fmadd_ps( a1, b, c1 );
					c2 = _mm256_fmadd_ps( a2, b, c2 );
					c3 = _mm256_fmadd_ps( a3, b, c3 );
					c4 = _mm256_fmadd_ps( a4, b, c4 );
					c5 = _mm256_fmadd_ps( a5, b, c5 );

					b = _mm256_loadu_ps( bPtr + 8 );
					a0 = _mm256_broadcast_ss( aPtr + 6 );
					a1 = _mm256_broadcast_ss( aPtr + 7 );
					a2 = _mm256_broadcast_ss( aPtr + 8 );
					a3 = _mm256_broadcast_ss( aPtr + 9 );
					a4 = _mm256_broadcast_ss( aPtr + 10 );
					a5 = _mm256_broadcast_ss( aPtr + 11 );

					c0 = _mm256_fmadd_ps( a0, b, c0 );
					c1 = _mm256_fmadd_ps( a1, b, c1 );
					c2 = _mm256_fmadd_ps( a2, b, c2 );
					c3 = _mm256_fmadd_ps( a3, b, c3 );
					c4 = _mm256_fmadd_ps( a4, b, c4 );
					c5 = _mm256_fmadd_ps( a5, b, c5 );

					b = _mm256_loadu_ps( bPtr + 16 );
					a0 = _mm256_broadcast_ss( aPtr + 12 );
					a1 = _mm256_broadcast_ss( aPtr + 13 );
					a2 = _mm256_broadcast_ss( aPtr + 14 );
					a3 = _mm256_broadcast_ss( aPtr + 15 );
					a4 = _mm256_broadcast_ss( aPtr + 16 );
					a5 = _mm256_broadcast_ss( aPtr + 17 );

					c0 = _mm256_fmadd_ps( a0, b, c0 );
					c1 = _mm256_fmadd_ps( a1, b, c1 );
					c2 = _mm256_fmadd_ps( a2, b, c2 );
					c3 = _mm256_fmadd_ps( a3, b, c3 );
					c4 = _mm256_fmadd_ps( a4, b, c4 );
					c5 = _mm256_fmadd_ps( a5, b, c5 );

					b = _mm256_loadu_ps( bPtr + 24 );
					a0 = _mm256_broadcast_ss( aPtr + 18 );
					a1 = _mm256_broadcast_ss( aPtr + 19 );
					a2 = _mm256_broadcast_ss( aPtr + 20 );
					a3 = _mm256_broadcast_ss( aPtr + 21 );
					a4 = _mm256_broadcast_ss( aPtr + 22 );
					a5 = _mm256_broadcast_ss( aPtr + 23 );

					c0 = _mm256_fmadd_ps( a0, b, c0 );
					c1 = _mm256_fmadd_ps( a1, b, c1 );
					c2 = _mm256_fmadd_ps( a2, b, c2 );
					c3 = _mm256_fmadd_ps( a3, b, c3 );
					c4 = _mm256_fmadd_ps( a4, b, c4 );
					c5 = _mm256_fmadd_ps( a5, b, c5 );

					bPtr += 32; aPtr += 24;
				}

				for( ; k > 0; k-- ) {
					b = _mm256_loadu_ps( bPtr );
					a0 = _mm256_broadcast_ss( aPtr );
					a1 = _mm256_broadcast_ss( aPtr + 1 );
					a2 = _mm256_broadcast_ss( aPtr + 2 );
					a3 = _mm256_broadcast_ss( aPtr + 3 );
					a4 = _mm256_broadcast_ss( aPtr + 4 );
					a5 = _mm256_broadcast_ss( aPtr + 5 );

					c0 = _mm256_fmadd_ps( a0, b, c0 );
					c1 = _mm256_fmadd_ps( a1, b, c1 );
					c2 = _mm256_fmadd_ps( a2, b, c2 );
					c3 = _mm256_fmadd_ps( a3, b, c3 );
					c4 = _mm256_fmadd_ps( a4, b, c4 );
					c5 = _mm256_fmadd_ps( a5, b, c5 );

					bPtr += 8; aPtr += 6;
				}

				_mm256_storeu_ps( cPtr, _mm256_add_ps( c0, _mm256_loadu_ps( cPtr ) ) );
				cPtr += cRowSize;
				_mm256_storeu_ps( cPtr, _mm256_add_ps( c1, _mm256_loadu_ps( cPtr ) ) );
				cPtr += cRowSize;
				_mm256_storeu_ps( cPtr, _mm256_add_ps( c2, _mm256_loadu_ps( cPtr ) ) );
				cPtr += cRowSize;
				_mm256_storeu_ps( cPtr, _mm256_add_ps( c3, _mm256_loadu_ps( cPtr ) ) );
				cPtr += cRowSize;
				_mm256_storeu_ps( cPtr, _mm256_add_ps( c4, _mm256_loadu_ps( cPtr ) ) );
				cPtr += cRowSize;
				_mm256_storeu_ps( cPtr, _mm256_add_ps( c5, _mm256_loadu_ps( cPtr ) ) );
				cPtr += cRowSize;
	}
};

#define PERMUTE2( p1, p0 ) ( ( p0 << 0 ) + ( p1 << 4 ) )
#define PERMUTE4( p3, p2, p1, p0 ) ( ( p0 << 0 ) + ( p1 << 2 ) + ( p2 << 4 ) + ( p3 << 6 ) )
#define PERMUTE8( p7, p6, p5, p4, p3, p2, p1, p0 ) _mm256_set_epi32( p7, p6, p5, p4, p3, p2, p1, p0 )
#define BLEND8( b7, b6, b5, b4, b3, b2, b1, b0 ) ( b0 + ( b1 << 1 ) + ( b2 << 2 ) + ( b3 << 3 ) + \
	( b4 << 4 ) + ( b5 << 5 ) + ( b6 << 6 ) + ( b7 << 7 ) )

struct CMicroKernel_6x4 : public CMicroKernelBase<6, 4> {
	static void Calculate( const float* aPtr, const float* bPtr, float* cPtr, size_t cRowSize, size_t k ) {
				__m256 c0 = _mm256_setzero_ps();
				__m256 c1 = _mm256_setzero_ps();
				__m256 c2 = _mm256_setzero_ps();

				__m256 b, b0, b1;
				__m256 a0, a1, a2, a00, a01, a02, a10, a11, a12;
				
				__m128 const * aPtrVec = reinterpret_cast<__m128 const *>( aPtr );
				
				for( ; k >= 4; k -= 4 ) {
					// Iteration 0, 1:
					b = _mm256_loadu_ps( bPtr );
					a0 = _mm256_broadcast_ps( aPtrVec++ );
					a1 = _mm256_broadcast_ps( aPtrVec++ );
					a2 = _mm256_broadcast_ps( aPtrVec++ );
					b0 = _mm256_permute2f128_ps( b, b, PERMUTE2( 0, 0 ) );
					b1 = _mm256_permute2f128_ps( b, b, PERMUTE2( 1, 1 ) );
					bPtr += 8;
					
					a00 = _mm256_permutevar_ps( a0, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a01 = _mm256_permutevar_ps( a0, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );
					a02 = _mm256_permutevar_ps( a1, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a10 = _mm256_permutevar_ps( a1, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );
					a11 = _mm256_permutevar_ps( a2, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a12 = _mm256_permutevar_ps( a2, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );

					b = _mm256_loadu_ps( bPtr );

					c0 = _mm256_fmadd_ps( a00, b0, c0 );
					c1 = _mm256_fmadd_ps( a01, b0, c1 );
					c2 = _mm256_fmadd_ps( a02, b0, c2 );

					a0 = _mm256_broadcast_ps( aPtrVec++ );
					a1 = _mm256_broadcast_ps( aPtrVec++ );
					a2 = _mm256_broadcast_ps( aPtrVec++ );

					c0 = _mm256_fmadd_ps( a10, b1, c0 );
					c1 = _mm256_fmadd_ps( a11, b1, c1 );
					c2 = _mm256_fmadd_ps( a12, b1, c2 );

					b0 = _mm256_permute2f128_ps( b, b, PERMUTE2( 0, 0 ) );
					b1 = _mm256_permute2f128_ps( b, b, PERMUTE2( 1, 1 ) );
					bPtr += 8;

					a00 = _mm256_permutevar_ps( a0, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a01 = _mm256_permutevar_ps( a0, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );
					a02 = _mm256_permutevar_ps( a1, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a10 = _mm256_permutevar_ps( a1, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );
					a11 = _mm256_permutevar_ps( a2, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a12 = _mm256_permutevar_ps( a2, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );

					c0 = _mm256_fmadd_ps( a00, b0, c0 );
					c1 = _mm256_fmadd_ps( a01, b0, c1 );
					c2 = _mm256_fmadd_ps( a02, b0, c2 );
					c0 = _mm256_fmadd_ps( a10, b1, c0 );
					c1 = _mm256_fmadd_ps( a11, b1, c1 );
					c2 = _mm256_fmadd_ps( a12, b1, c2 );

				}

				if( k >= 2 ) {
					k -= 2;
					b = _mm256_loadu_ps( bPtr );
					a0 = _mm256_broadcast_ps( aPtrVec++ );
					a1 = _mm256_broadcast_ps( aPtrVec++ );
					a2 = _mm256_broadcast_ps( aPtrVec++ );
					b0 = _mm256_permute2f128_ps( b, b, PERMUTE2( 0, 0 ) );
					b1 = _mm256_permute2f128_ps( b, b, PERMUTE2( 1, 1 ) );
					bPtr += 8;

					a00 = _mm256_permutevar_ps( a0, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a01 = _mm256_permutevar_ps( a0, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );
					a02 = _mm256_permutevar_ps( a1, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a10 = _mm256_permutevar_ps( a1, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );
					a11 = _mm256_permutevar_ps( a2, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a12 = _mm256_permutevar_ps( a2, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );

					c0 = _mm256_fmadd_ps( a00, b0, c0 );
					c1 = _mm256_fmadd_ps( a01, b0, c1 );
					c2 = _mm256_fmadd_ps( a02, b0, c2 );
					c0 = _mm256_fmadd_ps( a10, b1, c0 );
					c1 = _mm256_fmadd_ps( a11, b1, c1 );
					c2 = _mm256_fmadd_ps( a12, b1, c2 );
				}

				if( k > 0 ) {
					b0 = _mm256_broadcast_ps( reinterpret_cast<__m128 const *>( bPtr ) );
					a0 = _mm256_broadcast_ps( aPtrVec++ );
					a1 = _mm256_broadcast_ps( aPtrVec++ );

					a00 = _mm256_permutevar_ps( a0, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a01 = _mm256_permutevar_ps( a0, PERMUTE8( 3, 3, 3, 3, 2, 2, 2, 2 ) );
					a02 = _mm256_permutevar_ps( a1, PERMUTE8( 1, 1, 1, 1, 0, 0, 0, 0 ) );

					c0 = _mm256_fmadd_ps( a00, b0, c0 );
					c1 = _mm256_fmadd_ps( a01, b0, c1 );
					c2 = _mm256_fmadd_ps( a02, b0, c2 );
				}

				_mm256_storeu2_m128( cPtr + cRowSize, cPtr, _mm256_add_ps( c0, _mm256_loadu2_m128( cPtr + cRowSize, cPtr ) ) );
				cPtr += 2 * cRowSize;
				_mm256_storeu2_m128( cPtr + cRowSize, cPtr, _mm256_add_ps( c1, _mm256_loadu2_m128( cPtr + cRowSize, cPtr ) ) );
				cPtr += 2 * cRowSize;
				_mm256_storeu2_m128( cPtr + cRowSize, cPtr, _mm256_add_ps( c2, _mm256_loadu2_m128( cPtr + cRowSize, cPtr ) ) );
	}
};

struct CMicroKernel_6x2 : public CMicroKernelBase<6, 2> {
	static void Calculate( const float* aPtr, const float* bPtr, float* cPtr, size_t cRowSize, size_t k ) {
				__m256 c0 = _mm256_setzero_ps();
				__m256 c1 = _mm256_setzero_ps();
				__m256 c2 = _mm256_setzero_ps();

				__m256 b00, b01, b02, b10, b11, b12;
				__m256 a00, a01, a02, a10, a11, a12;

				// TODO: Check bPtr align
				double const * bPtrDouble = reinterpret_cast<double const *>( bPtr );
				__m128 const * aPtrVec = reinterpret_cast<__m128 const *>( aPtr );

				for( ; k >= 4; k -= 4 ) {
					__m256d b = _mm256_loadu_pd( bPtrDouble );
					bPtrDouble += 4;

					a00 = _mm256_broadcast_ps( aPtrVec++ );
					a01 = _mm256_broadcast_ps( aPtrVec++ );
					a02 = _mm256_broadcast_ps( aPtrVec++ );

					b00 = _mm256_castpd_ps( _mm256_permute4x64_pd( b, PERMUTE4( 0, 0, 0, 0 ) ) );
					b02 = _mm256_castpd_ps( _mm256_permute4x64_pd( b, PERMUTE4( 1, 1, 1, 1 ) ) );
					b10 = _mm256_castpd_ps( _mm256_permute4x64_pd( b, PERMUTE4( 2, 2, 2, 2 ) ) );
					b12 = _mm256_castpd_ps( _mm256_permute4x64_pd( b, PERMUTE4( 3, 3, 3, 3 ) ) );

					a10 = _mm256_broadcast_ps( aPtrVec++ );
					a11 = _mm256_broadcast_ps( aPtrVec++ );
					a12 = _mm256_broadcast_ps( aPtrVec++ );

					b01 = _mm256_blend_ps( b00, b02, BLEND8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					b11 = _mm256_blend_ps( b10, b12, BLEND8( 1, 1, 1, 1, 0, 0, 0, 0 ) );

					a00 = _mm256_permutevar_ps( a00, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );
					a01 = _mm256_permutevar_ps( a01, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );
					a02 = _mm256_permutevar_ps( a02, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );
					a10 = _mm256_permutevar_ps( a10, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );
					a11 = _mm256_permutevar_ps( a11, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );
					a12 = _mm256_permutevar_ps( a12, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );

					c0 = _mm256_fmadd_ps( a00, b00, c0 );
					c1 = _mm256_fmadd_ps( a01, b01, c1 );
					c2 = _mm256_fmadd_ps( a02, b02, c2 );
					c0 = _mm256_fmadd_ps( a10, b10, c0 );
					c1 = _mm256_fmadd_ps( a11, b11, c1 );
					c2 = _mm256_fmadd_ps( a12, b12, c2 );
				}

				if( k >= 2 ) {
					k -= 2;
					b00 = _mm256_castpd_ps( _mm256_broadcast_sd( bPtrDouble++ ) );
					b02 = _mm256_castpd_ps( _mm256_broadcast_sd( bPtrDouble++ ) );
					a00 = _mm256_broadcast_ps( aPtrVec++ );
					a01 = _mm256_broadcast_ps( aPtrVec++ );
					a02 = _mm256_broadcast_ps( aPtrVec++ );

					b01 = _mm256_blend_ps( b00, b02, BLEND8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a00 = _mm256_permutevar_ps( a00, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );
					a01 = _mm256_permutevar_ps( a01, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );
					a02 = _mm256_permutevar_ps( a02, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );

					c0 = _mm256_fmadd_ps( a00, b00, c0 );
					c1 = _mm256_fmadd_ps( a01, b01, c1 );
					c2 = _mm256_fmadd_ps( a02, b02, c2 );
				}

				if( k == 1 ) {
					b00 = _mm256_castpd_ps( _mm256_broadcast_sd( bPtrDouble++ ) );
					b02 = _mm256_setzero_ps();
					a00 = _mm256_broadcast_ps( aPtrVec++ );
					a01 = _mm256_broadcast_ps( aPtrVec++ );

					b01 = _mm256_blend_ps( b00, b02, BLEND8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					a00 = _mm256_permutevar_ps( a00, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );
					a01 = _mm256_permutevar_ps( a01, PERMUTE8( 3, 3, 2, 2, 1, 1, 0, 0 ) );

					c0 = _mm256_fmadd_ps( a00, b00, c0 );
					c1 = _mm256_fmadd_ps( a01, b01, c1 );
				}

				__m256 c0t = _mm256_castpd_ps( _mm256_permute2f128_pd( _mm256_castps_pd( c1 ), _mm256_castps_pd( c2 ), PERMUTE2( 2, 1 ) ) );
				__m256 c1t = _mm256_castpd_ps( _mm256_permute2f128_pd( _mm256_castps_pd( c2 ), _mm256_castps_pd( c2 ), PERMUTE2( 4, 1 ) ) );
				c0 = _mm256_add_ps( c0, c0t );
				c1 = _mm256_add_ps( c1, c1t );


				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, 0, 0, 0, -1, -1 ), c0 );
				// Decrease cRowSize because _mm256_storeu2_m128 treate start address as cPtr, but we should shift left our'c' value by 2.
				cPtr += cRowSize - 2;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, 0, -1, -1, 0, 0 ), c0 );
				cPtr += cRowSize - 2;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, -1, -1, 0, 0, 0, 0 ), c0 );
				cPtr += cRowSize - 2;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( -1, -1, 0, 0, 0, 0, 0, 0 ), c0 );
				// Get back start address
				cPtr += cRowSize + 6;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, 0, 0, 0, -1, -1 ), c1 );
				cPtr += cRowSize - 2;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, 0, -1, -1, 0, 0 ), c1 );
	}
};

struct CMicroKernel_6x1 : public CMicroKernelBase<6, 1> {
	static void Calculate( const float* aPtr, const float* bPtr, float* cPtr, size_t cRowSize, size_t k ) {
				__m256 c0 = _mm256_setzero_ps();
				__m256 c1 = _mm256_setzero_ps();
				__m256 c2 = _mm256_setzero_ps();

				__m256 b00, b01, b02, b0t;
				__m256 a00, a01, a02;

				for( ; k >= 4; k -= 4 ) {
					b00 = _mm256_broadcast_ss( bPtr + 0 );
					b01 = _mm256_broadcast_ss( bPtr + 1 );
					b02 = _mm256_broadcast_ss( bPtr + 2 );
					b0t = _mm256_broadcast_ss( bPtr + 3 );

					a00 = _mm256_loadu_ps( aPtr + 0 );
					a01 = _mm256_loadu_ps( aPtr + 8 );
					a02 = _mm256_loadu_ps( aPtr + 16 );

					b00 = _mm256_blend_ps( b00, b01, BLEND8( 1, 1, 0, 0, 0, 0, 0, 0 ) );
					b01 = _mm256_blend_ps( b01, b02, BLEND8( 1, 1, 1, 1, 0, 0, 0, 0 ) );
					b02 = _mm256_blend_ps( b02, b0t, BLEND8( 1, 1, 1, 1, 1, 1, 0, 0 ) );

					c0 = _mm256_fmadd_ps( a00, b00, c0 );
					c1 = _mm256_fmadd_ps( a01, b01, c1 );
					c2 = _mm256_fmadd_ps( a02, b02, c2 );

					bPtr += 4; aPtr += 24;
				}

				if( k >= 2 ) {
					k -= 2;
					__m256i mask = _mm256_set_epi64x( 0, 0, -1, -1 );
					b00 = _mm256_broadcast_ss( bPtr + 0 );
					b01 = _mm256_broadcast_ss( bPtr + 1 );

					a00 = _mm256_loadu_ps( aPtr + 0 );

					b00 = _mm256_blend_ps( b00, b01, BLEND8( 1, 1, 0, 0, 0, 0, 0, 0 ) );

					a01 = _mm256_castps128_ps256( _mm_loadu_ps( aPtr + 8 ) );
					a01 = _mm256_castsi256_ps( _mm256_and_si256( _mm256_castps_si256( a01 ), mask ) );

					c0 = _mm256_fmadd_ps( a00, b00, c0 );
					c1 = _mm256_fmadd_ps( a01, b01, c1 );

					bPtr += 2; aPtr += 12;
				}

				if( k == 1 ) {
					__m256i mask = _mm256_set_epi64x( 0, -1, -1, -1 );
					b00 = _mm256_broadcast_ss( bPtr );
					a00 = _mm256_loadu_ps( aPtr );
					b00 = _mm256_castsi256_ps( _mm256_and_si256( _mm256_castps_si256( b00 ), mask ) );
					c0 = _mm256_fmadd_ps( a00, b00, c0 );
				}


				__m256 c1t = _mm256_castpd_ps( _mm256_permute2f128_pd( _mm256_castps_pd( c0 ), _mm256_castps_pd( c1 ), PERMUTE2( 2, 1 ) ) );
				__m256 c2t = _mm256_castpd_ps( _mm256_permute2f128_pd( _mm256_castps_pd( c1 ), _mm256_castps_pd( c2 ), PERMUTE2( 2, 1 ) ) );
				__m256 c3t = _mm256_castpd_ps( _mm256_permute4x64_pd( _mm256_castps_pd( c2 ), PERMUTE4( 0, 3, 2, 1 ) ) );
				c1t = _mm256_castpd_ps( _mm256_permute4x64_pd( _mm256_castps_pd( c1t ), PERMUTE4( 0, 3, 2, 1 ) ) );


				c0 = _mm256_add_ps( c0, c1t );
				c2t = _mm256_add_ps( c2t, c3t );
				c0 = _mm256_add_ps( c0, c2t );

				// Decrease cRowSize because _mm256_storeu2_m128 treate start address as cPtr, but we should shift left our'c' value by 1.
				cRowSize--;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, 0, 0, 0, 0, -1 ), c0 );
				cPtr += cRowSize;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, 0, 0, 0, -1, 0 ), c0 );
				cPtr += cRowSize;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, 0, 0, -1, 0, 0 ), c0 );
				cPtr += cRowSize;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, 0, -1, 0, 0, 0 ), c0 );
				cPtr += cRowSize;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, 0, -1, 0, 0, 0, 0 ), c0 );
				cPtr += cRowSize;
				_mm256_maskstore_ps( cPtr, _mm256_set_epi32( 0, 0, -1, 0, 0, 0, 0, 0 ), c0 );
	}
};
#endif

bool HasMyKernel = getenv("MY_KERNEL") != nullptr;

using CKernelCombi = CKernelCombineHorizontal<CMicroKernel_6x16, CMicroKernel_6x8, CMicroKernel_6x4, CMicroKernel_6x2, CMicroKernel_6x1>;

template<bool ATransposed, bool BTransposed, class MemoryHandler, class Engine, class CCPUInfo>
inline void MultiplyMatrix(Engine *engine, const CCPUInfo &cpuInfo,
	const float* aPtr, size_t aRowSize,
	const float* bPtr, size_t bRowSize,
	float* cPtr, size_t cRowSize,
	size_t m, size_t n, size_t k)
{
//	if( getenv("MYGEMM") != 0) {
//		CMatrixMultiplier<CMicroKernel_6x16, CInterleaverDefault, ATransposed, BTransposed, MemoryHandler, Engine>::MyMultiply
//			(engine, cpuInfo, aPtr, aRowSize, bPtr, bRowSize, cPtr, cRowSize, m, n, k);
//	} else {
		if( HasMyKernel ) {
		} else {
			CMatrixMultiplier<CMicroKernel_6x16, CInterleaverDefault, ATransposed, BTransposed, MemoryHandler, Engine>::Multiply
				(engine, cpuInfo, aPtr, aRowSize, bPtr, bRowSize, cPtr, cRowSize, m, n, k);
		}
//	}
}
