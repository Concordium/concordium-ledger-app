#include <stdint.h>
#include "numberHelpers.h"
// TODO: add guards for the size of the output buffer.

int lengthOfNumber(uint64_t number) {
    if (number == 0) {
        return 1;
    }
    int len = 0;
    for (uint64_t nn = number; nn != 0; nn /= 10) {
        len++;
    }
    return len;
}

int numberToText(uint8_t *dst, uint64_t number) {
    int len = lengthOfNumber(number);

    // Build the number in big-endian order.
    for (int i = len - 1; i >= 0; i--) {
        dst[i] = (number % 10) + '0';
        number /= 10;
    }
    return len;
}

int bin2dec(uint8_t *dst, uint64_t number) {
    int characterLength = numberToText(dst, number);
    dst[characterLength] = '\0';
    return characterLength + 1;
}

/**
 * Write the display version of the decimal part of a GTU amount,
 * i.e. the numbers on the right-side of the decimal point.
 */
int decimalAmountToGtuDisplay(uint8_t *dst, uint64_t microGtuAmount) {
    // Fill with zeroes if the number is less than 6 digits,
    // so that input like 5304 become 005304 in their display version.
    int length = lengthOfNumber(microGtuAmount);
    int zeroFillLength = 6 - length;
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

    return numberToText(dst + zeroFillLength, microGtuAmount) + zeroFillLength;
}

/**
 * Constructs a display text version of a micro GTU amount, so that it
 * can displayed as GTU, i.e. not as the micro version, as it is easier
 * to relate to in the GUI.
 */
int amountToGtuDisplay(uint8_t *dst, uint64_t microGtuAmount) {
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
        int length = decimalAmountToGtuDisplay(dst + 2, microGtuAmount) + 2;
        dst[length] = '\0';
        return length + 1;
    }

    // If we reach this case, then the number is greater than 1.000.000 and we will
    // need to consider thousand separators for the whole number part.
    int wholeNumberLength = length - 6;
    int current = 0;
    int separatorCount = wholeNumberLength / 3;
    if (wholeNumberLength % 3 == 0) {
        separatorCount -= 1;
    }

    // 100,000
    
    // The first 6 digits should be without thousand separators,
    // as they are part of the decimal part of the number. Write those
    // characters first to the destination output and separate with ','
    uint8_t decimalSeparatorCount = 0;
    int decimalPartLength = 0;
    uint64_t decimalPart = microGtuAmount % 1000000;
    if (decimalPart != 0) {
        decimalPartLength = decimalAmountToGtuDisplay(dst + wholeNumberLength + separatorCount + 1, decimalPart);
        dst[wholeNumberLength + separatorCount] = '.';
        decimalSeparatorCount = 1;

        // Adjust length, as we might not have exactly 6 decimals anymore, as we have removed
        // non-significant zeroes at the end of the number.
        length -= 6 - decimalPartLength;
    } else {
        // The number does not have any decimals (they are all 0), so we reduce the total
        // length of the number to remove the decimals, as we don't need them to display the number.
        length -= 6;
    }
    microGtuAmount /= 1000000;

    // Write the whole number part of the amount to the output destination. This
    // part has to have thousand separators added.
    for (int i = wholeNumberLength - 1 + separatorCount; i >= 0; i--) {
		dst[i] = (microGtuAmount % 10) + '0';
		microGtuAmount /= 10;
        
        current += 1;
        if (current == 3 && i != 0) {
            dst[i - 1] = ',';
            i--;
            current = 0;
        }
    }

    dst[length + separatorCount + decimalSeparatorCount] = '\0';
    return length + separatorCount + decimalSeparatorCount + 1;
}

void toPaginatedHex(uint8_t *byteArray, const uint64_t len, char *asHex) {
    static uint8_t const hex[] = "0123456789abcdef";
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