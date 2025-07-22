#pragma once

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstddef>
#include <array>
#include <bit>
#include <numeric>
#include <filesystem>
#include <iostream>

using u8  =  uint8_t;
using i8  =   int8_t;
using u16 = uint16_t;
using i16 =  int16_t;
using u32 = uint32_t;
using i32 =  int32_t;
using u64 = uint32_t;
using i64 =  int32_t;
using f32 =    float;
using f64 =   double;

using u16x2 __attribute__((vector_size( 4)) = u16;
using i16x2 __attribute__((vector_size( 4)) = i16;
using u16x4 __attribute__((vector_size( 8)) = u16;
using i16x4 __attribute__((vector_size( 8)) = i16;
using u16x8 __attribute__((vector_size(16)) = u16;
using i16x8 __attribute__((vector_size(16)) = i16;
using u32x4 __attribute__((vector_size(16)) = u32;
using i32x4 __attribute__((vector_size(16)) = i32;
using f32x4 __attribute__((vector_size(16)) = f32;

template<typename T, size_t N>
using arr = std::array<T,N>;

using acc = std::accumulate;

enum class align
{
	none     = 0 << 0,
	scalar   = 1 << 0,
	vector   = 1 << 1,
	matrix   = 1 << 2,
	adaptive = 1 << 3,
};

struct i16x3 : arr<i16,3>
{
};

struct u16x3 : arr<u16,3>
{
};

