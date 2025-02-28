#include "globals.h"
#include "getAppName.h"

int handler(uint8_t INS,
            uint8_t *cdata,
            uint8_t p1,
            uint8_t p2,
            uint8_t lc,
            volatile unsigned int *flags,
            bool isInitialCall) {
    switch (INS) {
        case INS_GET_PUBLIC_KEY:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleGetPublicKey(cdata, p1, p2, lc, flags);
            break;
        case INS_VERIFY_ADDRESS:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleVerifyAddress(cdata, p1, lc, flags);
            break;
        case INS_SIGN_TRANSFER:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignTransfer(cdata, lc, flags);
            break;
        case INS_SIGN_TRANSFER_WITH_MEMO:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignTransferWithMemo(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_SIGN_TRANSFER_WITH_SCHEDULE:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignTransferWithSchedule(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignTransferWithScheduleAndMemo(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_CREDENTIAL_DEPLOYMENT:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignCredentialDeployment(cdata, p1, p2, lc, flags, isInitialCall);
            break;
        case INS_EXPORT_PRIVATE_KEY:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleExportPrivateKey(cdata, p1, p2, lc, flags);
            break;
        case INS_TRANSFER_TO_PUBLIC:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignTransferToPublic(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_REGISTER_DATA:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignRegisterData(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_PUBLIC_INFO_FOR_IP:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignPublicInformationForIp(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_CONFIGURE_BAKER:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignConfigureBaker(cdata, p1, lc, flags, isInitialCall);
            break;
        case INS_CONFIGURE_DELEGATION:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignConfigureDelegation(cdata, lc, flags);
            break;
        case INS_SIGN_UPDATE_CREDENTIAL:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleSignUpdateCredential(cdata, p1, p2, lc, flags, isInitialCall);
            break;
        case INS_GET_APP_NAME:
            return handleGetAppName();
            break;
        case INS_DEPLOY_MODULE:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleDeployModule(cdata, p1, lc);
            break;
        case INS_INIT_CONTRACT:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleInitContract(cdata, p1, lc);
            break;
        case INS_UPDATE_CONTRACT:
            LEDGER_ASSERT(cdata != NULL, "NULL cdata");
            handleUpdateContract(cdata, p1, lc);
            break;
        default:
            THROW(ERROR_INVALID_INSTRUCTION);
            break;
    }
    return 0;
}
