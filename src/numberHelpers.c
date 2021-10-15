#include "os.h"
#include <stdint.h>
#include "numberHelpers.h"
#include "responseCodes.h"

size_t lengthOfNumber(uint64_t number) {
    if (number == 0) {
        return 1;
    }
    size_t len = 0;
    for (uint64_t nn = number; nn != 0; nn /= 10) {
        len++;
    }
    return len;
}

size_t numberToText(uint8_t *dst, size_t dstLength, uint64_t number) {
    size_t len = lengthOfNumber(number);

    if (dstLength < len) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    // Build the number in big-endian order.
    for (int i = len - 1; i >= 0; i--) {
        dst[i] = (number % 10) + '0';
        number /= 10;
    }
    return len;
}

size_t bin2dec(uint8_t *dst, size_t dstLength, uint64_t number) {
    size_t characterLength = numberToText(dst, dstLength, number);
    if (dstLength < characterLength + 1) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    dst[characterLength] = '\0';
    return characterLength + 1;
}

/**
 * Write the display version of the decimal part of a GTU amount,
 * i.e. the numbers on the right-side of the decimal point.
 */
size_t decimalAmountToGtuDisplay(uint8_t *dst, size_t dstLength, uint64_t microGtuAmount) {
    // Fill with zeroes if the number is less than 6 digits,
    // so that input like 5304 become 005304 in their display version.
    size_t length = lengthOfNumber(microGtuAmount);
    int zeroFillLength = 6 - length;

    if (zeroFillLength > 0 && dstLength < zeroFillLength) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    for (int i = 0; i < zeroFillLength; i++) {
        dst[i] = '0';
    }

    // Remove any non-significant zeroes from the number.
    // This avoids displaying numbers like 5300, as it will
    // instead become 53.
    for (int i = length - 1; i >= 0; i--) {
        uint64_t currentNumber = (microGtuAmount % 10);
        if (currentNumber != 0) {
            break;
        } else {
            microGtuAmount /= 10;
        }
    }

    return numberToText(dst + zeroFillLength, dstLength - zeroFillLength, microGtuAmount) + zeroFillLength;
}

/**
 * Constructs a display text version of a micro GTU amount, so that it
 * can displayed as GTU, i.e. not as the micro version, as it is easier
 * to relate to in the GUI.
 */
size_t amountToGtuDisplay(uint8_t *dst, size_t dstLength, uint64_t microGtuAmount) {
    // In every case we need to write atleast 2 characters
    if (dstLength < 2) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    // A zero amount should be displayed as a plain '0'.
    if (microGtuAmount == 0) {
        dst[0] = '0';
        dst[1] = '\0';
        return 2;
    }

    int length = lengthOfNumber(microGtuAmount);

    // If the amount is less than than the resolution (micro), then the
    // GTU amount has to be prefixed by '0.' as it will purely consist
    // of the decimals.
    if (microGtuAmount < 1000000) {
        dst[0] = '0';
        dst[1] = '.';
        size_t length = decimalAmountToGtuDisplay(dst + 2, dstLength - 2, microGtuAmount) + 2;
        if (dstLength < length + 1) {
            THROW(ERROR_BUFFER_OVERFLOW);
        }
        dst[length] = '\0';
        return length + 1;
    }

    size_t offset = 0;

    // If we reach this case, then the number is greater than 1.000.000 and we will
    // need to consider thousand separators for the whole number part.
    size_t wholeNumberLength = length - 6;
    int current = 0;
    size_t separatorCount = wholeNumberLength / 3;
    if (wholeNumberLength % 3 == 0) {
        separatorCount -= 1;
    }
    uint64_t wholePart = microGtuAmount / 1000000;

    // We check that the fit entire number and termination,
    // under the assumption that there is no decimalPart
    if (dstLength < wholeNumberLength + separatorCount + 1) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    // Write the whole number part of the amount to the output destination. This
    // part has to have thousand separators added.
    for (int i = wholeNumberLength - 1 + separatorCount; i >= 0; i--) {
        dst[i] = (wholePart % 10) + '0';
        wholePart /= 10;

        current += 1;
        if (current == 3 && i != 0) {
            dst[i - 1] = ',';
            i--;
            current = 0;
        }
    }

    offset = wholeNumberLength + separatorCount;

    // The first 6 digits should be without thousand separators,
    // as they are part of the decimal part of the number. Write those
    // characters first to the destination output and separate with ','
    uint64_t decimalPart = microGtuAmount % 1000000;
    if (decimalPart != 0) {
        dst[offset] = '.';
        offset += 1;
        offset += decimalAmountToGtuDisplay(dst + offset, dstLength - offset, decimalPart);
    }

    // We check that we can fit the termination character
    if (dstLength < offset + 1) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    dst[offset] = '\0';
    return offset + 1;
}

void toPaginatedHex(uint8_t *byteArray, const uint64_t len, char *asHex, const size_t asHexSize) {
    static uint8_t const hex[] = "0123456789abcdef";

    if (asHexSize < len * 2 + len / 16 + 1) {
        return;
    }

    uint8_t offset = 0;
    for (uint64_t i = 0; i < len; i++) {
        asHex[2 * i + offset] = hex[(byteArray[i]>>4) & 0x0F];
        asHex[2 * i + (offset + 1)] = hex[(byteArray[i]>>0) & 0x0F];

        // Insert a space to force the Ledger to paginate the string every
        // 16 characters.
        if ((2 * (i + 1)) % 16 == 0 && i != len - 1) {
            asHex[2 * i + (offset + 2)] = ' ';
            offset += 1;
        }
    }
    asHex[2 * len + offset] = '\0';
}
