#pragma once
#include "globals.h"

/**
 * Writes the input amount of µGTU to the supplied destination as its value in
 * GTU with thousand separators.
 * @param dst where to write the thousand separated representation of the µGTU
 * @param dstLength the number of bytes that may be written to 'dst'
 * @param number the integer µGTU amount to convert to a GTU display version
 * @return number of bytes written to 'dst'
 */
size_t amountToGtuDisplay(uint8_t *dst, size_t dstLength, uint64_t microGtuAmount);

/**
 * Helper method that writes the input integer to a format that the device
 * can display on screen. The result is not string terminated.
 * @param dst where to write the text representation of the integer
 * @param dstLength the number of bytes that may be written to 'dst'
 * @param number the integer to convert to characters
 * @return number of bytes written to 'dst', i.e. the number of characters in the integer 'number'
 */
size_t numberToText(uint8_t *dst, size_t dstLength, uint64_t number);

/**
 * Helper methods that writes the input integer to a format that the device
 * can display on screen. The integer is postfixed with a unit and is string terminated.
 * @param dst where to write the text representation of the integer and the unit
 * @param dstLength the number of bytes that may be written to 'dst'
 * @param number the integer to convert to characters
 * @return number of bytes written to 'dst'
 */
size_t numberToTextWithUnit(uint8_t *dst,
                            size_t dstLength,
                            uint64_t number,
                            uint8_t *unit,
                            size_t unitLength);

/**
 * Helper method that writes the input integer to a format that the device can
 * display on the screen.
 * @param dst where to write the text representation of the integer
 * @param dstLength the number of bytes that may be written to 'dst'
 * @param number the integer to convert to characters
 * @return number of bytes written to 'dst', i.e. the number of characters in the integer 'number' +
 * 1 for string termination
 */
size_t bin2dec(uint8_t *dst, size_t dstLength, uint64_t number);

/**
 * Writes a fraction of the form "numerator/100000" to the destination formatted
 * as a percentage.
 * @param dst where to write the text representation of the fraction
 * @param dstLength the number of bytes that may be written to 'dst'
 * @param number the numerator of the fraction
 * @return number of bytes written to 'dst'.
 */
size_t fractionToPercentageDisplay(uint8_t *dst, size_t dstLength, uint32_t number);

/**
 * Helper method for converting a byte array into a character array, where the bytes
 * are translated into their hexadecimal representation. This is used for getting human-readable
 * representations of e.g. keys and credential ids. The output array is 'paginated' by inserting
 * a space after 16 characters, as this will make the Ledger pagination change page after
 * 16 characters.
 * @param byteArray [in] the bytes to convert to paginated hex
 * @param len the length of 'byteArray', i.e. the number of bytes to convert to paginated hex
 * @param asHex [out] where to write the output hexadecimal characters
 * @param asHexSize the number of characters that may be written to 'asHex'
 */
void toPaginatedHex(uint8_t *byteArray, const uint64_t len, char *asHex, size_t asHexSize);
