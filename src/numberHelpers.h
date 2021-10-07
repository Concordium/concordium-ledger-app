
/**
 * Writes the input amount of µGTU to the supplied destination as its value in
 * GTU with thousand separators. 
 * @param dst where to write the thousand separated representation of the µGTU
 * @param number the integer µGTU amount to convert to a GTU display version
 * @return number of bytes written to 'dst'
 */ 
int amountToGtuDisplay(uint8_t *dst, uint64_t microGtuAmount);

/**
 * Helper method that writes the input integer to a format that the device
 * can display on screen. The result is not string terminated.
 * @param dst where to write the text representation of the integer
 * @param number the integer to convert to characters
 * @return number of bytes written to 'dst', i.e. the number of characters in the integer 'number'
 */
int numberToText(uint8_t *dst, uint64_t number);

/**
 * Helper method that writes the input integer to a format that the device can 
 * display on the screen.
 * @param dst where to write the text representation of the integer
 * @param number the integer to convert to characters
 * @return number of bytes written to 'dst', i.e. the number of characters in the integer 'number' + 1 for string termination
 */
int bin2dec(uint8_t *dst, uint64_t number);

/**
 * Helper method for converting a byte array into a character array, where the bytes
 * are translated into their hexadecimal representation. This is used for getting human-readable
 * representations of e.g. keys and credential ids. The output array is 'paginated' by inserting
 * a space after 16 characters, as this will make the Ledger pagination change page after 
 * 16 characters.
 * @param byteArray [in] the bytes to convert to paginated hex
 * @param len the length of 'byteArray', i.e. the number of bytes to convert to paginated hex
 * @param asHex [out] where to write the output hexadecimal characters
 */
void toPaginatedHex(uint8_t *byteArray, const uint64_t len, char *asHex);