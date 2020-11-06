#include <os.h>
#include <os_io_seproxyhal.h>
#include "cx.h"
#include <stdint.h>
#include "menu.h"
#include "util.h"

// Variable length.
static uint8_t displayStr[128];
static uint8_t displayAmount[9];

// UI definitions for displaying the transaction contents for verification before approval by
// the user.
UX_STEP_NOCB(
    ux_sign_flow_0_step,
    bnnn_paging,
    {
      .title = "Recipient",
      .text = (char *) displayStr
    });
UX_STEP_VALID(
    ux_sign_flow_1_step,
    bn,
    ui_idle(),
    {
      "Amount",
      (char *) displayAmount
    });
UX_FLOW(ux_sign_flow,
    &ux_sign_flow_0_step,
    &ux_sign_flow_1_step
);


// TODO Understand this function, and clean it up.
int bin2dec(uint8_t *dst, uint64_t n) {
	if (n == 0) {
		dst[0] = '0';
		dst[1] = '\0';
		return 1;
	}
	// determine final length
	int len = 0;
	for (uint64_t nn = n; nn != 0; nn /= 10) {
		len++;
	}
	// write digits in big-endian order
	for (int i = len-1; i >= 0; i--) {
		dst[i] = (n % 10) + '0';
		n /= 10;
	}
	dst[len] = '\0';
	return len;
}

void signTransaction(uint8_t *dataBuffer, uint16_t dataLength, volatile unsigned int *flags) {
    // Verify that the transaction header has the expected size. If that is not the case, then the received command
    // must be corrupt. A transaction header is exactly 60 bytes, and has to received in the initial exchange.
    if (dataLength != 60) {
        THROW(0x6B01);
    }

    // Initialize the hash that will be the hash of the whole transaction, which is what will be signed
    // if the user approves.
    cx_sha256_t hash;
    cx_sha256_init(&hash);

    // Add the transaction header to the hash.
    cx_hash((cx_hash_t *) &hash, 0, dataBuffer, 60, NULL, 0);
    dataBuffer += 60;

    // Transaction payload/body comes right after the transaction header. For this initial version we assume it is
    // received within the same command, but this will probably be changed into a multiple commands kind of
    // protocol, because we cannot assume all transaction payloads can be within a single command (255 bytes).
    uint8_t transactionKind[1];
    os_memmove(transactionKind, dataBuffer, 1);
    dataBuffer += 1;

    // Extract the destination address.
    uint8_t toAddress[32];
    os_memmove(toAddress, dataBuffer, 32);
    dataBuffer += 32;

    // Used in display of recipient address
    char toAddressAsHex[65];
    publicKeyToHex(toAddress, sizeof(toAddress), toAddressAsHex);
    os_memmove(displayStr, &toAddressAsHex, 65);


    // Used to display the amount being transferred.
    uint64_t amount = U8BE(dataBuffer, 0);
    os_memmove(displayAmount, "GTU ", 4);
    bin2dec(displayAmount + 4, amount);
    dataBuffer += 8;

    // Extract the amount to transfer.
    // uint8_t amount[8];
    // os_memmove(amount, dataBuffer, 8);

    // This is probably not a good idea. Consider direct conversion with shifts instead.
    // uint64_t amountAsInt = *((uint64_t *) amount);

    // os_memmove(displayStr, "GTU ", 4);
    // os_memmove(displayStr + 4, &amountAsInt, 8);

    ux_flow_init(0, ux_sign_flow, NULL);

    // TODO Remove this - just used for testing.
    THROW(0x9000);
}
