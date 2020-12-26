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

struct CMicroKernel_4x24 : public CMicroKernelBase<4, 24> {
	static void Calculate( const float* aPtr, const float* bPtr, float* cPtr, size_t cRowSize, size_t k )
	{
        __m256 c00 = _mm256_setzero_ps();
        __m256 c01 = _mm256_setzero_ps();
        __m256 c02 = _mm256_setzero_ps();
        __m256 c10 = _mm256_setzero_ps();
        __m256 c11 = _mm256_setzero_ps();
        __m256 c12 = _mm256_setzero_ps();
        __m256 c20 = _mm256_setzero_ps();
        __m256 c21 = _mm256_setzero_ps();
        __m256 c22 = _mm256_setzero_ps();
        __m256 c30 = _mm256_setzero_ps();
        __m256 c31 = _mm256_setzero_ps();
        __m256 c32 = _mm256_setzero_ps();
        __m256 b0, b1, b2, a;
        for( int l = 0; l < k; l++ ) {
            a = _mm256_broadcast_ss( aPtr + 0 );
            b0 = _mm256_loadu_ps( bPtr + 0 );
            b1 = _mm256_loadu_ps( bPtr + 8 );
            b2 = _mm256_loadu_ps( bPtr + 16 );
            c00 = _mm256_fmadd_ps( a, b0, c00 );
            c01 = _mm256_fmadd_ps( a, b1, c01 );
            c02 = _mm256_fmadd_ps( a, b2, c02 );
            a = _mm256_broadcast_ss( aPtr + 1 );
            c10 = _mm256_fmadd_ps( a, b0, c10 );
            c11 = _mm256_fmadd_ps( a, b1, c11 );
            c12 = _mm256_fmadd_ps( a, b2, c12 );
            a = _mm256_broadcast_ss( aPtr + 2 );
            c20 = _mm256_fmadd_ps( a, b0, c20 );
            c21 = _mm256_fmadd_ps( a, b1, c21 );
            c22 = _mm256_fmadd_ps( a, b2, c22 );
            a = _mm256_broadcast_ss( aPtr + 3 );
            c30 = _mm256_fmadd_ps( a, b0, c30 );
            c31 = _mm256_fmadd_ps( a, b1, c31 );
            c32 = _mm256_fmadd_ps( a, b2, c32 );
            bPtr += 24; aPtr += 4;
        }

        _mm256_storeu_ps( cPtr + 0, _mm256_add_ps( c00, _mm256_loadu_ps( cPtr + 0 ) ) );
        _mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c01, _mm256_loadu_ps( cPtr + 8 ) ) );
        _mm256_storeu_ps( cPtr + 16, _mm256_add_ps( c02, _mm256_loadu_ps( cPtr + 16 ) ) );
        cPtr += cRowSize;
        _mm256_storeu_ps( cPtr + 0, _mm256_add_ps( c10, _mm256_loadu_ps( cPtr + 0 ) ) );
        _mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c11, _mm256_loadu_ps( cPtr + 8 ) ) );
        _mm256_storeu_ps( cPtr + 16, _mm256_add_ps( c12, _mm256_loadu_ps( cPtr + 16 ) ) );
        cPtr += cRowSize;
        _mm256_storeu_ps( cPtr + 0, _mm256_add_ps( c20, _mm256_loadu_ps( cPtr + 0 ) ) );
        _mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c21, _mm256_loadu_ps( cPtr + 8 ) ) );
        _mm256_storeu_ps( cPtr + 16, _mm256_add_ps( c22, _mm256_loadu_ps( cPtr + 16 ) ) );
        cPtr += cRowSize;
        _mm256_storeu_ps( cPtr + 0, _mm256_add_ps( c30, _mm256_loadu_ps( cPtr + 0 ) ) );
        _mm256_storeu_ps( cPtr + 8, _mm256_add_ps( c31, _mm256_loadu_ps( cPtr + 8 ) ) );
        _mm256_storeu_ps( cPtr + 16, _mm256_add_ps( c32, _mm256_loadu_ps( cPtr + 16 ) ) );

        //_mm256_storeu_ps( cPtr + 0, c00 ); // _mm256_add_ps( c00, _mm256_loadu_ps( cPtr + 0 ) ) );
        //_mm256_storeu_ps( cPtr + 8, c01 ); // _mm256_add_ps( c01, _mm256_loadu_ps( cPtr + 8 ) ) );
        //_mm256_storeu_ps( cPtr + 16, c02 ); // _mm256_add_ps( c02, _mm256_loadu_ps( cPtr + 16 ) ) );
        //cPtr += cRowSize;
        //_mm256_storeu_ps( cPtr + 0, c10 ); // _mm256_add_ps( c10, _mm256_loadu_ps( cPtr + 0 ) ) );
        //_mm256_storeu_ps( cPtr + 8, c11 ); // _mm256_add_ps( c11, _mm256_loadu_ps( cPtr + 8 ) ) );
        //_mm256_storeu_ps( cPtr + 16, c12 ); // _mm256_add_ps( c12, _mm256_loadu_ps( cPtr + 16 ) ) );
        //cPtr += cRowSize;
        //_mm256_storeu_ps( cPtr + 0, c20 ); // _mm256_add_ps( c20, _mm256_loadu_ps( cPtr + 0 ) ) );
        //_mm256_storeu_ps( cPtr + 8, c21 ); // _mm256_add_ps( c21, _mm256_loadu_ps( cPtr + 8 ) ) );
        //_mm256_storeu_ps( cPtr + 16, c22 ); // _mm256_add_ps( c22, _mm256_loadu_ps( cPtr + 16 ) ) );
        //cPtr += cRowSize;
        //_mm256_storeu_ps( cPtr + 0, c30 ); // _mm256_add_ps( c30, _mm256_loadu_ps( cPtr + 0 ) ) );
        //_mm256_storeu_ps( cPtr + 8, c31 ); // _mm256_add_ps( c31, _mm256_loadu_ps( cPtr + 8 ) ) );
        //_mm256_storeu_ps( cPtr + 16, c32 ); // _mm256_add_ps( c32, _mm256_loadu_ps( cPtr + 16 ) ) );

		//for( size_t i = 0; i < height; ++i ) {
		//	for( size_t j = 0; j < width; ++j ) {
		//		for( size_t l = 0; l < k; ++l ) {
		//			cPtr[i * cRowSize + j] += aPtr[l * height + i] * bPtr[l * width + j];
		//		}
		//	}
		//}
	}
};
#endif

template<bool ATransposed, bool BTransposed, class MemoryHandler, class Engine, class CCPUInfo>
inline void MultiplyMatrix(Engine *engine, const CCPUInfo &cpuInfo,
	const float* aPtr, size_t aRowSize,
	const float* bPtr, size_t bRowSize,
	float* cPtr, size_t cRowSize,
	size_t m, size_t n, size_t k)
{
	if( m % 4 == 0 && n % 24 == 0 ) {
		CMatrixMultiplier<CMicroKernel_4x24, CInterleaverDefault, ATransposed, BTransposed, MemoryHandler, Engine>::Multiply
		( engine, cpuInfo, aPtr, aRowSize, bPtr, bRowSize, cPtr, cRowSize, m, n, k );
	} else {
		CMatrixMultiplier<CMicroKernelDefault, CInterleaverDefault, ATransposed, BTransposed, MemoryHandler, Engine>::Multiply
		( engine, cpuInfo, aPtr, aRowSize, bPtr, bRowSize, cPtr, cRowSize, m, n, k );
	}
}