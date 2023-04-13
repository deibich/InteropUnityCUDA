#pragma once
#include "log.h"
#include "cuda_runtime.h"
#include <string>

// use this macro if you want to check cuda function
#define CUDA_CHECK(ans) cudaAssert((ans), __FILE__, __LINE__)
#define CUDA_CHECK_RETURN(ans) {int ret = cudaAssert((ans), __FILE__, __LINE__); if(ret != 0)return ret;}
#define CUDA_CHECK_RETURN_VOID(ans, msg) {int ret = cudaAssert((ans), __FILE__, __LINE__); if(ret != 0){Log::log().debugLogError(msg);return;}}
#define CUFFT_CHECK(ans) cufftAssert((int)(ans), __FILE__, __LINE__)
#define CUFFT_CHECK_RETURN(ans) { int ret  = cufftAssert((int)(ans), __FILE__, __LINE__); return ret; }


inline int cudaAssert(cudaError_t code, const char* file, int line)
{
	if (code != cudaSuccess)
	{
		char buffer[2048];
		sprintf_s(buffer, "Cuda error: %i %s %s %d\n", code, cudaGetErrorString(code), file, line);
		std::string strError(buffer);
		Log::log().debugLogError(buffer);
		return (int)code;
	}
	return 0;
}

inline int cufftAssert(int cufftResult, const char* file, int line)
{
	
	if (cufftResult != 0)
	{
		std::string cufftInterpret;
		switch (cufftResult)
		{
		case(0):
			cufftInterpret = "The cuFFT operation was successful";
			break;
		case(1):
			cufftInterpret = "cuFFT was passed an invalid plan handle";
			break;
		case(2):
			cufftInterpret = "cuFFT failed to allocate GPU or CPU memory";
			break;
		case(3):
			cufftInterpret = "No longer used";
			break; 
		case(4):
			cufftInterpret = "User specified an invalid pointer or parameter";
			break;
		case(5):
			cufftInterpret = "Driver or internal cuFFT library error";
			break;
		case(6):
			cufftInterpret = "Failed to execute an FFT on the GPU";
			break;
		case(7):
			cufftInterpret = "The cuFFT library failed to initialize";
			break;
		case(8):
			cufftInterpret = "User specified an invalid transform size";
			break;
		case(9):
			cufftInterpret = "No longer used";
			break;
		case(10):
			cufftInterpret = "Missing parameters in call";
			break;
		case(11):
			cufftInterpret = "Execution of a plan was on different GPU than plan creation";
			break;
		case(12):
			cufftInterpret = "Internal plan database error";
			break;
		case(13):
			cufftInterpret = "No workspace has been provided prior to plan execution";
			break;
		case(14):
			cufftInterpret = "Function does not implement functionality for parameters given.";
			break;
		case(15):
			cufftInterpret = "Used in previous versions.";
			break;
		case(16):
			cufftInterpret = "Operation is not supported for parameters given.";
			break;
		default:
			cufftInterpret = "Unknown error.";
			break;
		}
		char buffer[2048];
		sprintf_s(buffer, "Cufft error: %i %s %s %d\n", cufftResult, cufftInterpret.c_str(), file, line);
		std::string strError(buffer);
		Log::log().debugLogError(buffer);
	}
	return cufftResult;

}



/// <summary>
/// Get thedim grid to use for a dispatch, from a multiple of
/// dim block that are used by the kernel, and the number of
/// calculation that has to be done.
/// </summary>
/// <param name="dimBlock">Number of threads per block
/// </param> 
/// <param name="numCalculation">Number of calculation
/// to do on kernel (eg. if we make calculation on a 1024x1024 texture, and
/// we only want to compute a value on the first 528x528 pixels , then
/// numCalculation = 528,528,1)
/// </param> 
/// <param name="getUp">If true will get the
/// upper multiple of dimBlock, else will get the lower multiple. By
/// default its true.
/// </param> 
/// <param name="mustDoAllCalculation">if true
/// imply that dimBlock must be multiple of numCalculation
/// </param>
/// <returns>The dim of grid to use in dispatch</returns>
inline dim3 calculateDimGrid(dim3 dimBlock, dim3 numCalculation, bool getUp = true,
	bool mustDoAllCalculation = false)
{
	int addFactor = getUp ? 1 : 0;
	float invDimBlockX = 1.0f / dimBlock.x;
	float invDimBlockY = 1.0f / dimBlock.y;
	float invDimBlockZ = 1.0f / dimBlock.z;

	if (mustDoAllCalculation)
	{
		if (numCalculation.x % dimBlock.x != 0 ||
			numCalculation.y % dimBlock.y != 0 ||
			numCalculation.z % dimBlock.z != 0)
		{
			Log::log().debugLogError(
				"Number of threads per block (" + std::to_string(dimBlock.x) +
				", " + std::to_string(dimBlock.y) + ", " +
				std::to_string(dimBlock.z) +
				")"
				" is not a multiple of (" +
				std::to_string(numCalculation.x) + ", " +
				std::to_string(numCalculation.y) + ", " +
				std::to_string(numCalculation.z) +
				")"
				", therefore the compute shader will not compute on all data.");
		}
	}

	unsigned int multipleDimBlockX =
		dimBlock.x * ((int)(numCalculation.x * invDimBlockX) + addFactor);
	//unsigned int multipleDimBlockX =
	//    dimBlock.x * (numCalculation.x / dimBlock.x) + addFactor);
	// TODO remove dimBlock.x above and bellow
	unsigned int dimGridX = multipleDimBlockX / dimBlock.x;

	unsigned int multipleDimBlockY =
		dimBlock.y * ((int)(numCalculation.y * invDimBlockY) + addFactor);
	unsigned int dimGridY = multipleDimBlockY / dimBlock.y;

	unsigned int multipleDimBlockZ =
		dimBlock.z * ((int)(numCalculation.z * invDimBlockZ) + addFactor);
	unsigned int dimGridZ = multipleDimBlockZ / dimBlock.z;

	if (dimGridX < 1 || dimGridY < 1 || dimGridZ < 1)
	{
		Log::log().debugLogError(
			"Threads group size " + std::to_string(dimGridX) +
			std::to_string(dimGridY) + std::to_string(dimGridZ) +
			" must be above zero.");
	}

	return dim3{ dimGridX, dimGridY, dimGridZ };
}