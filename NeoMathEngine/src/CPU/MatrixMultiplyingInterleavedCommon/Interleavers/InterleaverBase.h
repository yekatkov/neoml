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

#include <immintrin.h>
#include <algorithm>
#include <cstring>

// Prepare the submatrix and write it into a temporary buffer
// The matrix is split into blocks of Len width
// All blocks are written into memory together
// If Transpose == true, each block will be transposed
//                         +----+
//                         | M1 |
//                         +----+
// +----+----+ . +----+    | M2 |
// | M1 | M2 |   | MN | => +----+
// +----+----+ . +----+    .    .
//                         +----+
//                         | MN |
//                         +----+
template <bool Transpose, size_t Len>
struct CInterleaverBase;

// Prepare the matrix, don't transpose
template<size_t Len>
struct CInterleaverBase<false, Len> {
	static void Prepare(float* out, const float* in, size_t stride, size_t height, size_t width)
	{
		constexpr size_t KBlock = 4;
		const size_t hStride = height * Len;
		for( ; height >= KBlock; height -= KBlock ) {
			float* outStart = out;
			out += KBlock * Len;
			const float* in0 = in;
			const float* in1 = in0 + stride;
			const float* in2 = in1 + stride;
			const float* in3 = in2 + stride;

			in = in3 + stride;
			size_t lineWidth = width;
			for( ; lineWidth >= Len; lineWidth -= Len ) {
				float* outLine = outStart;
				outStart += hStride;
				memcpy(outLine, in0, Len * sizeof(float));
				outLine += Len;
				in0 += Len;
				memcpy(outLine, in1, Len * sizeof(float));
				outLine += Len;
				in1 += Len;
				memcpy(outLine, in2, Len * sizeof(float));
				outLine += Len;
				in2 += Len;
				memcpy(outLine, in3, Len * sizeof(float));
				in3 += Len;
			}
			if( lineWidth > 0 ) {
				const size_t nullifySize = (Len - lineWidth) * sizeof(float);

				memcpy(outStart, in0, lineWidth * sizeof(float));
				memset(outStart + lineWidth, 0, nullifySize);
				outStart += Len;
				memcpy(outStart, in1, lineWidth * sizeof(float));
				memset(outStart + lineWidth, 0, nullifySize);
				outStart += Len;
				memcpy(outStart, in2, lineWidth * sizeof(float));
				memset(outStart + lineWidth, 0, nullifySize);
				outStart += Len;
				memcpy(outStart, in3, lineWidth * sizeof(float));
				memset(outStart + lineWidth, 0, nullifySize);
			}
		}
		if( height > 0 ) {
			const float* in0 = in;
			const float* in1 = in0 + stride;
			const float* in2 = in1 + stride;
			size_t lineWidth = width;
			for( ; lineWidth >= Len; lineWidth -= Len ) {
				float* outLine = out;
				out += hStride;
				memcpy(outLine, in0, Len * sizeof(float));
				in0 += Len;
				if( height > 1 ) {
					outLine += Len;
					memcpy(outLine, in1, Len * sizeof(float));
					in1 += Len;
					if( height > 2 ) {
						outLine += Len;
						memcpy(outLine, in2, Len * sizeof(float));
						in2 += Len;
					}
				}
			}
			if( lineWidth > 0 ) {
				const size_t nullifySize = (Len - lineWidth) * sizeof(float);
				memcpy(out, in0, lineWidth * sizeof(float));
				memset(out + lineWidth, 0, nullifySize);
				if( height > 1 ) {
					out += Len;
					memcpy(out, in1, lineWidth * sizeof(float));
					memset(out + lineWidth, 0, nullifySize);
					if( height > 2 ) {
						out += Len;
						memcpy(out, in2, lineWidth * sizeof(float));
						memset(out + lineWidth, 0, nullifySize);
					}
				}
			}
		}
	}
};

// The variant that degenerates into a simple copy
template<>
struct CInterleaverBase<true, 1> {
	static void Prepare(float* out, const float* in, size_t stride, size_t width, size_t height)
	{
		if( stride == width ) {
			memcpy(out, in, width * height * sizeof(float));
		} else {
			for( ; height > 0; --height ) {
				memcpy(out, in, width * sizeof(float));
				out += width;
				in += stride;
			}
		}
	}
};

// The variant with transposition
template<>
struct CInterleaverBase<false, 1> {
	static void Transpose(float* out, size_t oStride, const float* in, size_t iStride, size_t height, size_t width)
	{
		const size_t oStep4 = oStride - 4;
		const size_t oStep2 = oStride - 2;
		const size_t oStep1 = oStride - 1;
		for( ; height >= 4; height -= 4 ) {
			const float* in0 = in;
			const float* in1 = in0 + iStride;
			const float* in2 = in1 + iStride;
			const float* in3 = in2 + iStride;
			in = in3 + iStride;
			float* outLine = out;
			out += 4;
			for( size_t len = width; len > 0; --len ) {
				*outLine++ = *in0++;
				*outLine++ = *in1++;
				*outLine++ = *in2++;
				*outLine++ = *in3++;
				outLine += oStep4;
			}
		}
		if( height >= 2 ) {
			height -= 2;
			const float* in0 = in;
			const float* in1 = in0 + iStride;
			in = in1 + iStride;
			float* outLine = out;
			out += 2;
			for( size_t len = width; len > 0; --len ) {
				*outLine++ = *in0++;
				*outLine++ = *in1++;
				outLine += oStep2;
			}
		}
		if( height >= 1 ) {
			for (; width > 0; --width) {
				*out++ = *in++;
				out += oStep1;
			}
		}
	}

	static void Prepare(float* out, const float* in, size_t stride, size_t height, size_t width)
	{
		Transpose(out, height, in, stride, height, width);
	}
};

// Prepare and transpose the matrix
template<size_t Len>
struct CInterleaverBase<true, Len> {
	static void Prepare(float* out, const float* in, size_t stride, size_t width, size_t height)
	{
		const size_t iStep = stride * Len;
		const size_t oStep = width * Len;
		for( ; height >= Len; height -= Len ) {
			CInterleaverBase<false, 1>::Transpose(out, Len, in, stride, Len, width);
			in += iStep;
			out += oStep;
		}
		if( height > 0 ) {
			CInterleaverBase<false, 1>::Transpose(out, Len, in, stride, height, width);
			out += height;
			const size_t len = (Len - height) * sizeof(float);
			for( ; width > 0; --width ) {
				memset(out, 0, len);
				out += Len;
			}
		}
	}
};

// Prepare and transpose the matrix
template<>
struct CInterleaverBase<true, 4> {
	static void Prepare( float* out, const float* in, size_t stride, size_t width, size_t height )
	{
		const int Len = 4;

		const size_t iStep = stride * Len;
		const size_t oStep = width * Len;

		for( ; height >= Len; height -= Len ) {
			int tempWidth = width;
			const float* tempIn = in;
			float* tempOut = out;
			for( ; tempWidth >= 8; tempWidth -= 8 ) {
				__m256 a = _mm256_loadu_ps( tempIn + 0 * stride );
				__m256 b = _mm256_loadu_ps( tempIn + 1 * stride );
				__m256 c = _mm256_loadu_ps( tempIn + 2 * stride );
				__m256 d = _mm256_loadu_ps( tempIn + 3 * stride );

				// a:     a0 a1 a2 a3 a4 a5 a6 a7
				// b:     b0 b1 b2 b3 b4 b5 b6 b7
				// ab_lo: a0 b0 a1 b1 a4 b4 a5 b5
				__m256 ab_lo = _mm256_unpacklo_ps( a, b );
				// a:     a0 a1 a2 a3 a4 a5 a6 a7
				// b:     b0 b1 b2 b3 b4 b5 b6 b7
				// ab_hi: a2 b2 a3 b3 a6 b6 a7 b7
				__m256 ab_hi = _mm256_unpackhi_ps( a, b );
				// c:     c0 c1 c2 c3 c4 c5 c6 c7
				// d:     d0 d1 d2 d3 d4 d5 d6 d7
				// cd_lo: c0 d0 c1 d1 c4 d4 c5 d5
				__m256 cd_lo = _mm256_unpacklo_ps( c, d );
				// c:     c0 c1 c2 c3 c4 c5 c6 c7
				// d:     d0 d1 d2 d3 d4 d5 d6 d7
				// cd_hi: c2 d2 c3 d3 c6 d6 c7 d7
				__m256 cd_hi = _mm256_unpackhi_ps( c, d );

				// ab_lo:   a0 b0 a1 b1 a4 b4 a5 b5
				// cd_lo:   c0 d0 c1 d1 c4 d4 c5 d5
				// abcd_04: a0 b0 c0 d0 a4 b4 c4 d4
				__m256 abcd_04 = _mm256_shuffle_ps( ab_lo, cd_lo, _MM_SHUFFLE( 1, 0, 1, 0 ) );
				__m256 abcd_15 = _mm256_shuffle_ps( ab_lo, cd_lo, _MM_SHUFFLE( 3, 2, 3, 2 ) );
				__m256 abcd_26 = _mm256_shuffle_ps( ab_hi, cd_hi, _MM_SHUFFLE( 1, 0, 1, 0 ) );
				__m256 abcd_37 = _mm256_shuffle_ps( ab_hi, cd_hi, _MM_SHUFFLE( 3, 2, 3, 2 ) );

				// __m256 abcd_01 = _mm256_permute2f128_ps( abcd_04, abcd_15, 0 | 2 << 4 );
				_mm256_storeu_ps( tempOut + 0, _mm256_permute2f128_ps( abcd_04, abcd_15, 0 | 2 << 4 ) );
				// __m256 abcd_23 = _mm256_permute2f128_ps( abcd_26, abcd_37, 0 | 2 << 4 );
				_mm256_storeu_ps( tempOut + 8, _mm256_permute2f128_ps( abcd_26, abcd_37, 0 | 2 << 4 ) );
				// __m256 abcd_45 = _mm256_permute2f128_ps( abcd_04, abcd_15, 1 | 3 << 4 );
				_mm256_storeu_ps( tempOut + 16, _mm256_permute2f128_ps( abcd_04, abcd_15, 1 | 3 << 4 ) );
				// __m256 abcd_67 = _mm256_permute2f128_ps( abcd_26, abcd_37, 1 | 3 << 4 );
				_mm256_storeu_ps( tempOut + 24, _mm256_permute2f128_ps( abcd_26, abcd_37, 1 | 3 << 4 ) );
				tempIn += 8;
				tempOut += 32;
			}
			if( tempWidth != 0 ) {
				CInterleaverBase<false, 1>::Transpose( tempOut, Len, tempIn, stride, Len, tempWidth );
			}
			in += iStep;
			out += oStep;
		}
		height %= Len;

		if( height > 0 ) {
			CInterleaverBase<false, 1>::Transpose( out, Len, in, stride, height, width );
			out += height;
			const size_t len = ( Len - height ) * sizeof( float );
			for( ; width > 0; --width ) {
				memset( out, 0, len );
				out += Len;
			}
		}
	}
};