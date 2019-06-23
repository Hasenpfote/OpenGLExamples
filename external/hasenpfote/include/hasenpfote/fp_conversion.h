/*!
 * @file fp_conversion.h
 * @brief IEEE-754 floating-point conversion.
 * @author Hasenpfote
 */
#pragma once
#include <cstdint>

namespace hasenpfote{

 /*!
  * Conversion from binary32 to binary16.
  * @param [in] half     IEEE 754 single-precision binary floating-point format: binary32.
  * @return IEEE 754 half-precision binary floating-point format: binary16.
  */
std::uint16_t ConvertSingleToHalf(float single);

/*!
 * Conversion from binary16 to binary32.
 * @param [in] half     IEEE 754 half-precision binary floating-point format: binary16.
 * @return IEEE 754 single-precision binary floating-point format: binary32.
 */
float ConvertHalfToSingle(std::uint16_t half);

}