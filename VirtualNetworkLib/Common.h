#pragma once

#ifdef __COMPILING_VNET
#define VNET_API __declspec(dllexport)
#else
#define VNET_API __declspec(dllimport)
#endif

#include <string>
#include <vector>
#include <stdint.h>
#include <memory>

namespace vnet {

typedef int32_t int32;
typedef uint32_t uint32;

typedef int8_t int8;
typedef uint8_t uint8;

} // End vnet.