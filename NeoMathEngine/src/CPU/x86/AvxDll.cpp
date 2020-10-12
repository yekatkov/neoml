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

#include <common.h>
#pragma hdrstop

#if FINE_PLATFORM( FINE_DARWIN ) || FINE_PLATFORM( FINE_LINUX )
#include <cpuid.h>
#endif
#if FINE_PLATFORM( FINE_WINDOWS )
#include <intrin.h>
#endif

#include <AvxDll.h>
#include <MathEngineCommon.h>
#include <MemoryHandleInternal.h>

namespace NeoML {

CAvxDll::CAvxDll() : isLoaded( false ), functionAdresses{}
{
	if( !isAvxAvailable() || !Load( libName ) ) {
		return;
	}

	loadFunction( TFunctionPointers::IsBlobConvolutionAvailable, "IsBlobConvolutionAvailable" );
	loadFunction( TFunctionPointers::ProcessBlobConvolution, "ProcessBlobConvolution" );
	loadFunction( TFunctionPointers::BlobConvolution_f3x3_c24_fc24, "BlobConvolution_f3x3_c24_fc24" );

	isLoaded = true;
}

CAvxDll& CAvxDll::GetInstance()
{
	static CAvxDll instance;
	return instance;
}

void CAvxDll::loadFunction( TFunctionPointers functionType, const char* functionName )
{
	void* functionAdress = reinterpret_cast<void*>( GetProcAddress( functionName ) );
	ASSERT_EXPR( functionAdress != nullptr );

	functionAdresses[static_cast<size_t>(functionType)] = functionAdress;
}

bool CAvxDll::IsBlobConvolutionAvailable( const CCommonConvolutionDesc& desc ) const
{
	typedef bool ( *FuncType )( int C, int FC, int FH, int FW );
	FuncType func = reinterpret_cast<FuncType>( functionAdresses.at( static_cast<size_t>( TFunctionPointers::IsBlobConvolutionAvailable ) ) );
	return isLoaded && func( desc.Filter.Channels(), desc.Filter.BatchWidth(), desc.Filter.Height(), desc.Filter.Width() );
}

void CAvxDll::ProcessBlobConvolution( int threadCount, const CCommonConvolutionDesc& desc, const float* sourceData,
	const float* filterData, const float* freeTermData, float* resultData ) const
{
	typedef bool ( *FuncType )( int C, int FC, int FH, int FW, int threadCount,
		int sourceHeight, int sourceWidth, int strideHeight, int strideWidth,
		int dilationHeight, int dilationWidth, int resultHeight, int resultWidth,
		const float* sourceData, const float* filterData, const float* freeTermData, float* resultData );
	FuncType func = reinterpret_cast<FuncType>( functionAdresses.at( static_cast<size_t>( TFunctionPointers::ProcessBlobConvolution ) ) );
	
	// Data should be alligned
	ASSERT_EXPR( reinterpret_cast<std::uintptr_t>( sourceData ) % 32 == 0 );
	ASSERT_EXPR( reinterpret_cast<std::uintptr_t>( resultData ) % 32 == 0 );

	ASSERT_EXPR( func( desc.Filter.Channels(), desc.Filter.BatchWidth(), desc.Filter.Height(), desc.Filter.Width(), threadCount,
		desc.Source.Height(), desc.Source.Width(), desc.StrideHeight, desc.StrideWidth,
		desc.DilationHeight, desc.DilationWidth, desc.Result.Height(), desc.Result.Width(),
		sourceData, filterData, freeTermData, resultData ) );
}

void CAvxDll::CallBlobConvolution_f3x3_c24_fc24( int threadCount, const CCommonConvolutionDesc& desc,
	const float* sourceData, const float* filterData, const float* freeTermData, float* resultData ) const
{
	typedef void ( *FuncType )( int, int, int, int, int, int, const float*, const float*, const float*, float* );
	FuncType func = reinterpret_cast<FuncType>( functionAdresses.at( static_cast<size_t>( TFunctionPointers::BlobConvolution_f3x3_c24_fc24 ) ) );

	ASSERT_EXPR( func != nullptr );

	// Data should be alligned
	ASSERT_EXPR( reinterpret_cast<std::uintptr_t>( sourceData ) % 32 == 0 );
	ASSERT_EXPR( reinterpret_cast<std::uintptr_t>(  resultData  ) % 32 == 0 );

	func( desc.Source.Width(), desc.Result.Height(), desc.Result.Width(), desc.StrideWidth, desc.DilationWidth,
		threadCount, sourceData,  filterData, freeTermData,  resultData );
}

bool CAvxDll::isAvxAvailable()
{
	// Check for AVX
	#if FINE_PLATFORM(FINE_WINDOWS)
	
	#if _MSC_VER < 1900
	// VS 2015 compiles code which doesn't use all ymm registers. It brings to decrease performance compared to MKL.
	// Therefore we just disable AVX convolution  enhancement in VS2015.
	return false;
	#endif
	
	int cpuId[4] = { 0, 0, 0, 0 };
	__cpuid( cpuId, 1 );
	#elif FINE_PLATFORM(FINE_LINUX) || FINE_PLATFORM(FINE_DARWIN)
	unsigned int cpuId[4] = { 0, 0, 0, 0 };
	__get_cpuid( 1, cpuId, cpuId + 1, cpuId + 2, cpuId + 3 );
	#elif FINE_PLATFORM(FINE_ANDROID) || FINE_PLATFORM(FINE_IOS)
	unsigned int cpuId[4] = { 0, 0, 0, 0 };
	#else
	#error "Platform isn't supported!"
	#endif

	return ( cpuId[2] & 0x10000000 ) != 0;

}
}