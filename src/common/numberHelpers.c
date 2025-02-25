#include "globals.h"

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

size_t numberToTextWithUnit(uint8_t *dst,
                            size_t dstLength,
                            uint64_t number,
                            uint8_t *unit,
                            size_t unitLength) {
    size_t len = numberToText(dst, dstLength, number);

    if (dstLength - len < unitLength + 2) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    memmove(dst + len, " ", 1);
    memmove(dst + len + 1, unit, unitLength);
    memmove(dst + len + 1 + unitLength, "\0", 1);

    return len + unitLength + 2;
}

size_t bin2dec(uint8_t *dst, size_t dstLength, uint64_t number) {
    size_t characterLength = numberToText(dst, dstLength, number);
    if (dstLength < characterLength + 1) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    dst[characterLength] = '\0';
    return characterLength + 1;
}

size_t decimalDigitsDisplay(uint8_t *dst,
                            size_t dstLength,
                            uint64_t decimalPart,
                            uint8_t decimalDigitsLength) {
    // Fill with zeroes if the number is less than decimalDigits,
    // so that input like 5304 become 005304 in their display version.
    size_t length = lengthOfNumber(decimalPart);
    int zeroFillLength = decimalDigitsLength - length;

    if (zeroFillLength < 0 || dstLength < (size_t)zeroFillLength) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    for (int i = 0; i < zeroFillLength; i++) {
        dst[i] = '0';
    }

    // Remove any non-significant zeroes from the number.
    // This avoids displaying numbers like 5300, as it will
    // instead become 53.
    for (int i = length - 1; i >= 0; i--) {
        uint64_t currentNumber = (decimalPart % 10);
        if (currentNumber != 0) {
            break;
        } else {
            decimalPart /= 10;
        }
    }

    return numberToText(dst + zeroFillLength, dstLength - zeroFillLength, decimalPart) +
           zeroFillLength;
}

size_t decimalNumberToDisplay(uint8_t *dst,
                              size_t dstLength,
                              uint64_t amount,
                              uint32_t resolution,
                              uint8_t decimalDigitsLength) {
    // In every case we need to write at least 2 characters
    if (dstLength < 2) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    // A zero amount should be displayed as a plain '0'.
    if (amount == 0) {
        dst[0] = '0';
        return 1;
    }

    int length = lengthOfNumber(amount);

    // If the amount is less than than the resolution, then the
    // amount has to be prefixed by '0.' as it will purely consist
    // of the decimals.
    if (amount < resolution) {
        dst[0] = '0';
        dst[1] = '.';
        // We decrement the length an extra time, to make sure there is space for the termination.
        return decimalDigitsDisplay(dst + 2, dstLength - 3, amount, decimalDigitsLength) + 2;
    }

    size_t offset = 0;

    // If we reach this case, then the number is greater than the resolution and we will
    // need to consider thousand separators for the whole number part.
    size_t wholeNumberLength = length - decimalDigitsLength;
    int current = 0;
    size_t separatorCount = wholeNumberLength / 3;
    if (wholeNumberLength % 3 == 0) {
        separatorCount -= 1;
    }
    uint64_t wholePart = amount / resolution;

    // We check that the entire number and termination fits,
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
    // characters first to the destination output and separate with '.'
    uint64_t decimalPart = amount % resolution;
    if (decimalPart != 0) {
        dst[offset] = '.';
        offset += 1;
        offset += decimalDigitsDisplay(dst + offset,
                                       dstLength - offset,
                                       decimalPart,
                                       decimalDigitsLength);
    }

    // We check that we can fit the termination character
    if (dstLength < offset + 1) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    return offset;
}

size_t fractionToPercentageDisplay(uint8_t *dst, size_t dstLength, uint32_t number) {
    if (number > 100000) {
        THROW(ERROR_INVALID_TRANSACTION);
    }

    size_t offset = decimalNumberToDisplay(dst, dstLength, number, 1000, 3);
    if (dstLength < offset + 2) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }
    dst[offset] = '%';
    dst[offset + 1] = '\0';
    return offset + 2;
}

/**
 * Constructs a display text version of a micro GTU amount, so that it
 * can displayed as GTU, i.e. not as the micro version, as it is easier
 * to relate to in the GUI.
 */
size_t amountToGtuDisplay(uint8_t *dst, size_t dstLength, uint64_t microGtuAmount) {
    if (dstLength < 5) return 0;  // Prevent overflow
    memmove(dst, "CCD ", 4);
    size_t offset = decimalNumberToDisplay(dst + 4, dstLength, microGtuAmount, 1000000, 6) + 4;
    dst[offset] = '\0';
    return offset + 1;
}

void toPaginatedHex(uint8_t *byteArray, const uint64_t len, char *asHex, const size_t asHexSize) {
    LEDGER_ASSERT(byteArray != NULL, "NULL byteArray");

    static uint8_t const hex[] = "0123456789abcdef";

    if (asHexSize < len * 2 + len / 16 + 1) {
        THROW(ERROR_BUFFER_OVERFLOW);
    }

    uint8_t offset = 0;
    for (uint64_t i = 0; i < len; i++) {
        asHex[2 * i + offset] = hex[(byteArray[i] >> 4) & 0x0F];
        asHex[2 * i + (offset + 1)] = hex[(byteArray[i] >> 0) & 0x0F];

        // Insert a space to force the Ledger to paginate the string every
        // 16 characters.
        if ((2 * (i + 1)) % 16 == 0 && i != len - 1) {
            asHex[2 * i + (offset + 2)] = ' ';
            offset += 1;
        }
    }
    asHex[2 * len + offset] = '\0';
}
