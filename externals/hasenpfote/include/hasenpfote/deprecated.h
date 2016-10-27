/*!
* @file deprecated.h
* @brief Defines the assert deprecated and related functionality.
* @author Hasenpfote
* @date 2016/07/15
*/
#pragma once

#ifdef __GNUC__
#define HASENPFOTE_DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define HASENPFOTE_DEPRECATED __declspec(deprecated)
#else
#define HASENPFOTE_DEPRECATED
#pragma message("HASENPFOTE_DEPRECATED has not been defined in this compiler.")
#endif