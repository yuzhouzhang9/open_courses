#include <stdio.h>
#include <algorithm>
#include <math.h>
#include "CMU418intrin.h"
#include "logger.h"
using namespace std;

void absSerial(float *values, float *output, int N)
{
	for (int i = 0; i < N; i++)
	{
		float x = values[i];
		if (x < 0)
		{
			output[i] = -x;
		}
		else
		{
			output[i] = x;
		}
	}
}

// implementation of absolute value using 15418 instrinsics
void absVector(float *values, float *output, int N)
{
	__cmu418_vec_float x;
	__cmu418_vec_float result;
	__cmu418_vec_float zero = _cmu418_vset_float(0.f);
	__cmu418_mask maskAll, maskIsNegative, maskIsNotNegative;

	//  Note: Take a careful look at this loop indexing.  This example
	//  code is not guaranteed to work when (N % VECTOR_WIDTH) != 0.
	//  Why is that the case?
	for (int i = 0; i < N; i += VECTOR_WIDTH)
	{

		// All ones
		maskAll = _cmu418_init_ones();

		// All zeros
		maskIsNegative = _cmu418_init_ones(0);

		// void _cmu418_vload(__cmu418_vec<T> & dest, T * src, __cmu418_mask & mask)
		// {
		// 	for (int i = 0; i < VECTOR_WIDTH; i++)
		// 	{
		// 		dest.value[i] = mask.value[i] ? src[i] : dest.value[i];
		// 	}
		// 	CMU418Logger.addLog("vload", mask, VECTOR_WIDTH);
		// }
		// Load vector of values from contiguous memory addresses
		_cmu418_vload_float(x, values + i, maskAll); // x = values[i];

		// void _cmu418_vlt(__cmu418_mask & maskResult, __cmu418_vec<T> & veca, __cmu418_vec<T> & vecb, __cmu418_mask & mask)
		// {
		// 	for (int i = 0; i < VECTOR_WIDTH; i++)
		// 	{
		// 		maskResult.value[i] = mask.value[i] ? (veca.value[i] < vecb.value[i]) : maskResult.value[i];
		// 	}
		// 	CMU418Logger.addLog("vlt", mask, VECTOR_WIDTH);
		// }
		// Set mask according to predicate
		_cmu418_vlt_float(maskIsNegative, x, zero, maskAll); // if (x < 0) {

		// void _cmu418_vsub(__cmu418_vec<T> & vecResult, __cmu418_vec<T> & veca, __cmu418_vec<T> & vecb, __cmu418_mask & mask)
		// {
		// 	for (int i = 0; i < VECTOR_WIDTH; i++)
		// 	{
		// 		vecResult.value[i] = mask.value[i] ? (veca.value[i] - vecb.value[i]) : vecResult.value[i];
		// 	}
		// 	CMU418Logger.addLog("vsub", mask, VECTOR_WIDTH);
		// }
		// Execute instruction using mask ("if" clause)
		_cmu418_vsub_float(result, zero, x, maskIsNegative); //   output[i] = -x;
		// __cmu418_mask _cmu418_mask_not(__cmu418_mask & maska)
		// {
		// 	__cmu418_mask resultMask;
		// 	for (int i = 0; i < VECTOR_WIDTH; i++)
		// 	{
		// 		resultMask.value[i] = !maska.value[i];
		// 	}
		// 	CMU418Logger.addLog("masknot", _cmu418_init_ones(), VECTOR_WIDTH);
		// 	return resultMask;
		// }
		// Inverse maskIsNegative to generate "else" mask
		maskIsNotNegative = _cmu418_mask_not(maskIsNegative); // } else {
		// void _cmu418_vload(__cmu418_vec<T> & dest, T * src, __cmu418_mask & mask)
		// {
		// 	for (int i = 0; i < VECTOR_WIDTH; i++)
		// 	{
		// 		dest.value[i] = mask.value[i] ? src[i] : dest.value[i];
		// 	}
		// 	CMU418Logger.addLog("vload", mask, VECTOR_WIDTH);
		// }
		// Execute instruction ("else" clause)
		_cmu418_vload_float(result, values + i, maskIsNotNegative); //   output[i] = x; }
		// void _cmu418_vstore(T * dest, __cmu418_vec<T> & src, __cmu418_mask & mask)
		// {
		// 	for (int i = 0; i < VECTOR_WIDTH; i++)
		// 	{
		// 		dest[i] = mask.value[i] ? src.value[i] : dest[i];
		// 	}
		// 	CMU418Logger.addLog("vstore", mask, VECTOR_WIDTH);
		// }
		// Write results back to memory
		_cmu418_vstore_float(output + i, result, maskAll);
	}
}

// Accepts an array of values and an array of exponents
// For each element, compute values[i]^exponents[i] and clamp value to
// 4.18.  Store result in outputs.
// Uses iterative squaring, so that total iterations is proportional
// to the log_2 of the exponent
void clampedExpSerial(float *values, int *exponents, float *output, int N)
{
	for (int i = 0; i < N; i++)
	{
		float x = values[i];
		float result = 1.f;
		int y = exponents[i];
		float xpower = x;
		while (y > 0)
		{
			if (y & 0x1)
			{
				result *= xpower;
			}
			xpower = xpower * xpower;
			y >>= 1;
		}
		if (result > 4.18f)
		{
			result = 4.18f;
		}
		output[i] = result;
	}
}
// 矢量操作相关的掩码函数声明

// 初始化一个掩码，前first个元素为true，其余为false
// __cmu418_mask _cmu418_init_ones(int first);

// 对输入掩码进行NOT操作
// __cmu418_mask _cmu418_mask_not(__cmu418_mask &maska);

// 对两个掩码进行OR操作
// __cmu418_mask _cmu418_mask_or(__cmu418_mask &maska, __cmu418_mask &maskb);

// 对两个掩码进行AND操作
// __cmu418_mask _cmu418_mask_and(__cmu418_mask &maska, __cmu418_mask &maskb);

// 计算掩码中为true的位数
// int _cmu418_cntbits(__cmu418_mask &maska);

// 矢量设置操作声明

// 根据掩码将矢量中的元素设置为指定值
// template <typename T>
// void _cmu418_vset(__cmu418_vec<T> &vecResult, T value, __cmu418_mask &mask);

// float类型的矢量设置操作
// void _cmu418_vset_float(__cmu418_vec_float &vecResult, float value, __cmu418_mask &mask);

// int类型的矢量设置操作
// void _cmu418_vset_int(__cmu418_vec_int &vecResult, int value, __cmu418_mask &mask);

// 创建一个所有元素都为指定值的float矢量
// __cmu418_vec_float _cmu418_vset_float(float value);

// 创建一个所有元素都为指定值的int矢量
// __cmu418_vec_int _cmu418_vset_int(int value);

// // 矢量移动和加载操作声明

// // 根据掩码将源矢量的值移动到目标矢量
// template <typename T>
// void _cmu418_vmove(__cmu418_vec<T> &dest, __cmu418_vec<T> &src, __cmu418_mask &mask);

// void _cmu418_vmove_float(__cmu418_vec_float &dest, __cmu418_vec_float &src, __cmu418_mask &mask);
// void _cmu418_vmove_int(__cmu418_vec_int &dest, __cmu418_vec_int &src, __cmu418_mask &mask);

// // 根据掩码从内存加载数据到矢量
// template <typename T>
// void _cmu418_vload(__cmu418_vec<T> &dest, T *src, __cmu418_mask &mask);

// void _cmu418_vload_float(__cmu418_vec_float &dest, float *src, __cmu418_mask &mask);
// void _cmu418_vload_int(__cmu418_vec_int &dest, int *src, __cmu418_mask &mask);

// // 根据掩码将矢量数据存储到内存
// template <typename T>
// void _cmu418_vstore(T *dest, __cmu418_vec<T> &src, __cmu418_mask &mask);

// void _cmu418_vstore_float(float *dest, __cmu418_vec_float &src, __cmu418_mask &mask);
// void _cmu418_vstore_int(int *dest, __cmu418_vec_int &src, __cmu418_mask &mask);

// // 矢量算术运算声明

// // 矢量加法运算
// template <typename T>
// void _cmu418_vadd(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask);

// void _cmu418_vadd_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
// void _cmu418_vadd_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 矢量减法运算
// template <typename T>
// void _cmu418_vsub(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask);

// void _cmu418_vsub_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
// void _cmu418_vsub_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 矢量乘法运算
// template <typename T>
// void _cmu418_vmult(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask);

// void _cmu418_vmult_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
// void _cmu418_vmult_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 矢量除法运算
// template <typename T>
// void _cmu418_vdiv(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask);

// void _cmu418_vdiv_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
// void _cmu418_vdiv_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 整数位运算操作声明

// // 矢量右移操作
// void _cmu418_vshiftright_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 矢量按位与操作
// void _cmu418_vbitand_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 矢量比较操作声明

// // 绝对值运算
// template <typename T>
// void _cmu418_vabs(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &veca, __cmu418_mask &mask);

// void _cmu418_vabs_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &veca, __cmu418_mask &mask);
// void _cmu418_vabs_int(__cmu418_vec_int &vecResult, __cmu418_vec_int &veca, __cmu418_mask &mask);

// // 大于比较
// template <typename T>
// void _cmu418_vgt(__cmu418_mask &maskResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask);

// void _cmu418_vgt_float(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
// void _cmu418_vgt_int(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 小于比较
// template <typename T>
// void _cmu418_vlt(__cmu418_mask &maskResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask);

// void _cmu418_vlt_float(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
// void _cmu418_vlt_int(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 相等比较
// template <typename T>
// void _cmu418_veq(__cmu418_mask &maskResult, __cmu418_vec<T> &veca, __cmu418_vec<T> &vecb, __cmu418_mask &mask);

// void _cmu418_veq_float(__cmu418_mask &maskResult, __cmu418_vec_float &veca, __cmu418_vec_float &vecb, __cmu418_mask &mask);
// void _cmu418_veq_int(__cmu418_mask &maskResult, __cmu418_vec_int &veca, __cmu418_vec_int &vecb, __cmu418_mask &mask);

// // 特殊矢量操作声明

// // 水平加法操作（相邻元素相加）
// template <typename T>
// void _cmu418_hadd(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &vec);

// void _cmu418_hadd_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &vec);

// // 矢量元素交织操作
// template <typename T>
// void _cmu418_interleave(__cmu418_vec<T> &vecResult, __cmu418_vec<T> &vec);

// void _cmu418_interleave_float(__cmu418_vec_float &vecResult, __cmu418_vec_float &vec);

// // 日志记录函数
// void addUserLog(const char *logStr);
#include <iostream>
void clampedExpVector(float *values, int *exponents, float *output, int N)
{
	// Implement your vectorized version of clampedExpSerial here
	//
	__cmu418_vec_int exponentsVec;
	__cmu418_vec_float valuesVec;
	__cmu418_vec_float resultVec;
	__cmu418_mask mask, mask_zero, mask_one;
	mask_one = _cmu418_init_ones();
	mask_zero = _cmu418_init_ones(0);
	__cmu418_vec_int exp_zero = _cmu418_vset_int(0);
	__cmu418_vec_int exp_one = _cmu418_vset_int(1);
	__cmu418_vec_int bitand_res;
	__cmu418_vec_float compair_value = _cmu418_vset_float(4.18f);
	for (int i = 0; i < N; i += VECTOR_WIDTH)
	{
		// load data
		_cmu418_vload_float(valuesVec, values + i, mask_one);
		_cmu418_vload_int(exponentsVec, exponents + i, mask_one);
		resultVec = _cmu418_vset_float(1.f);
		_cmu418_vgt_int(mask, exponentsVec, exp_zero, mask_one);
		for (int i = 0; i < 8; i++)
		{
			std::cout << exponentsVec.value[i] << " ";
		}
		std::cout << endl;
		// if exp > 0 : 停止计算
		// x**y
		while (_cmu418_cntbits(mask) > 0)
		{
			// if  y&1
			_cmu418_vbitand_int(bitand_res, exponentsVec, exp_one, mask_one);
			_cmu418_vgt_int(mask_zero, bitand_res, exp_zero, mask_one);

			// r = r * x
			_cmu418_vmult_float(resultVec, resultVec, valuesVec, mask_zero);

			// x *=x
			_cmu418_vmult_float(valuesVec, valuesVec, valuesVec, mask);

			// y>>=1
			_cmu418_vshiftright_int(exponentsVec, exponentsVec, exp_one, mask);

			_cmu418_vgt_int(mask, exponentsVec, exp_zero, mask_one);
			// for (int i = 0; i < 8; i++)
			// {
			// 	std::cout << exponentsVec.value[i] << " ";
			// }
			// std::cout << endl;
		}

		// if result > 4.18
		_cmu418_vgt_float(mask_zero, resultVec, compair_value, mask_one);

		// result = 4.18
		_cmu418_vset_float(resultVec, 4.18f, mask_zero);

		// store result
		_cmu418_vstore_float(output + i, resultVec, mask_one);
	}
}

float arraySumSerial(float *values, int N)
{
	float sum = 0;
	for (int i = 0; i < N; i++)
	{
		sum += values[i];
	}

	return sum;
}

// Assume N % VECTOR_WIDTH == 0
// Assume VECTOR_WIDTH is a power of 2
float arraySumVector(float *values, int N)
{
	// Implement your vectorized version here
	//  ...
	__cmu418_vec_float vecSum = _cmu418_vset_float(0.f);
	__cmu418_vec_float vecTemp;
	__cmu418_mask mask = _cmu418_init_ones();
	for (int i = 0; i < N; i += VECTOR_WIDTH)
	{
		_cmu418_vload_float(vecTemp, values + i, mask);
		_cmu418_hadd_float(vecSum, vecTemp);
		// for (int i = 0; i < VECTOR_WIDTH; i++)
		// {
		// 	std::cout << vecSum.value[i] << " ";
		// }
	}
	float sum;
	for (int i = 0; i < VECTOR_WIDTH; i++)
	{
		sum += vecSum.value[i];
		// std::cout << vecSum.value[i] << " ";
	}
	return sum;
}
