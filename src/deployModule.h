#pragma once

/**
 * Handles the DEPLOY_MODULE instruction, which deploys a module
 *
 *
 */
void handleDeployModule(uint8_t *cdata, uint8_t p1, uint8_t p2);

typedef struct {
    uint32_t version;
    uint32_t sourceLength;
    uint32_t remainingSourceLength;
    uint8_t sourceHash[32];
    char sourceHashDisplay[65];
    char versionDisplay[11];
} deployModule_t;
