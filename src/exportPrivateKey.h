#pragma once
/**
 * Handles the export of private keys that are allowed to leave the device.
 * The export paths are restricted so that the method cannot access any account paths.
 * @param p1 has to be 0x00 for export of PRF key for decryption, 0x01 for export of PRF key for
 * recovering credentials and 0x02 for export of PRF key and IdCredSec.
 * @param p2 If set to 0x01, then the seeds are exported (Using this is deprecated). If set to 0x02,
 * then the BLS keys are exported. 0x00 is not used to ensure that old clients fail when calling
 * this functionality.
 */
void handleExportPrivateKey(uint8_t *dataBuffer,
                            uint8_t p1,
                            uint8_t p2,
                            uint8_t lc,
                            bool legacyDerivationPath,
                            volatile unsigned int *flags);

typedef struct {
    uint8_t displayHeader[20];
    uint8_t display[22];
    bool exportBoth;
    bool exportSeed;
    uint32_t path[6];
    uint8_t pathLength;
    bool isNewPath;
} exportPrivateKeyContext_t;

void uiExportPrivateKey(volatile unsigned int *flags);
void exportPrivateKey(void);
