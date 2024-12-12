#pragma once

/**
 * Handles the GET_APP_NAME instruction, which returns the application name
 *
 * @param[in,out] flags
 *   Set to IO_RETURN_AFTER_TX if successful
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handleGetAppName();
