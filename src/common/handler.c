
#ifndef _HANDLER_C_
#define _HANDLER_C_

#include <stdbool.h>
#include <string.h>

#include "responseCodes.h"
#include "globals.h"
#include "handler.h"
#include "getAppName.h"

void handler(uint8_t INS,
             uint8_t *cdata,
             uint8_t p1,
             uint8_t p2,
             uint8_t lc,
             volatile unsigned int *flags,
             bool isInitialCall) {
    switch (INS) {
        case INS_GET_PUBLIC_KEY:
            handleGetPublicKey(cdata, p1, p2, flags);
            break;
        case INS_VERIFY_ADDRESS:
            handleVerifyAddress(cdata, flags);
            break;
        case INS_SIGN_TRANSFER:
            handleSignTransfer(cdata, flags);
            break;
        case INS_SIGN_TRANSFER_WITH_MEMO:
            handleSignTransferWithMemo(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_SIGN_TRANSFER_WITH_SCHEDULE:
            handleSignTransferWithSchedule(cdata, p1, flags, isInitialCall);
            break;
        case INS_SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO:
            handleSignTransferWithScheduleAndMemo(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_CREDENTIAL_DEPLOYMENT:
            handleSignCredentialDeployment(cdata, p1, p2, flags, isInitialCall);
            break;
        case INS_EXPORT_PRIVATE_KEY:
            handleExportPrivateKey(cdata, p1, p2, flags);
            break;
        case INS_TRANSFER_TO_ENCRYPTED:
            handleSignTransferToEncrypted(cdata, flags);
            break;
        case INS_ENCRYPTED_AMOUNT_TRANSFER:
            handleSignEncryptedAmountTransfer(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_ENCRYPTED_AMOUNT_TRANSFER_WITH_MEMO:
            handleSignEncryptedAmountTransferWithMemo(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_TRANSFER_TO_PUBLIC:
            handleSignTransferToPublic(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_REGISTER_DATA:
            handleSignRegisterData(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_PUBLIC_INFO_FOR_IP:
            handleSignPublicInformationForIp(cdata, p1, flags, isInitialCall);
            break;
        case INS_CONFIGURE_BAKER:
            handleSignConfigureBaker(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_CONFIGURE_DELEGATION:
            handleSignConfigureDelegation(cdata, lc, flags);
            break;
        case INS_SIGN_UPDATE_CREDENTIAL:
            handleSignUpdateCredential(cdata, p1, p2, flags, isInitialCall);
            break;
        case INS_GET_APP_NAME:
            handleGetAppName(flags);
            break;
        default:
            THROW(ERROR_INVALID_INSTRUCTION);
            break;
    }
}

#endif
