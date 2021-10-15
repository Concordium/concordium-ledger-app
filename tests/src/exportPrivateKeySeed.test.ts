import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

const path = '00000134';
const prfKeySeed = 'd9bd8b475cfb4f7c0e42e7059d27906c09f98760e1104c27430624f37da75707';
const bothSeeds = prfKeySeed + '1a2f72de118c37e4029245b3acb6ff541ceffb70911b115800be025d8ed1c24b';
const end = '9000';

const exportPrivateKeyTest = (p1: number, picture: string, seed: string) => (async (sim: Zemu, transport: Transport) => {
    const data = Buffer.from(path, 'hex');
    const tx = transport.send(0xe0, 0x05, p1, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', picture, [0]);
    await expect(tx).resolves.toEqual(
        Buffer.from(seed + end, 'hex'),
    );
});

test('[NANO S] Export private key seed for decryption key (PRF-key seed)', setupZemu('nanos', exportPrivateKeyTest(0x00, 'nanos_export_decrypt_prf_key_seed', prfKeySeed)));
test('[NANO X] Export private key seed for decryption key (PRF-key seed)', setupZemu('nanox', exportPrivateKeyTest(0x00, 'nanox_export_decrypt_prf_key_seed', prfKeySeed)));

test('[NANO S] Export private key seed for recovery (PRF-key seed)', setupZemu('nanos', exportPrivateKeyTest(0x01, 'nanos_export_recovery_prf_key_seed', prfKeySeed)));
test('[NANO X] Export private key seed for recovery (PRF-key seed)', setupZemu('nanox', exportPrivateKeyTest(0x01, 'nanox_export_recovery_prf_key_seed', prfKeySeed)));

test('[NANO S] Export private key seed for creating credential (IdCredSec seed)', setupZemu('nanos', exportPrivateKeyTest(0x02, 'nanos_export_credential_idcredsec_seed', bothSeeds)));
test('[NANO X] Export private key seed for creating credential (IdCredSec seed)', setupZemu('nanox', exportPrivateKeyTest(0x02, 'nanox_export_credential_idcredsec_seed', bothSeeds)));

const negativePrivateKeyExportTest = (p1: number, picture: string, seed: string) => (async (sim: Zemu, transport: Transport) => {
    expect.assertions(1);
    const data = Buffer.from(path, 'hex');
    const tx = transport.send(0xe0, 0x05, p1, 0x01, data).catch((e) => expect(e.statusCode).toEqual(27013));
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', picture, [1, 0]);
});

test('[NANO S] Decline to export private key seed for decryption', setupZemu('nanos', negativePrivateKeyExportTest(0x00, 'nanos_decline_decrypt', prfKeySeed)));
test('[NANO X] Decline to export private key seed for decryption', setupZemu('nanox', negativePrivateKeyExportTest(0x00, 'nanox_decline_decrypt', prfKeySeed)));

test('[NANO S] Decline to export private key seed for recovery', setupZemu('nanos', negativePrivateKeyExportTest(0x01, 'nanos_decline_recovery', prfKeySeed)));
test('[NANO X] Decline to export private key seed for recovery', setupZemu('nanox', negativePrivateKeyExportTest(0x01, 'nanox_decline_recovery', prfKeySeed)));

test('[NANO S] Decline to export private key seed for creating credential', setupZemu('nanos', negativePrivateKeyExportTest(0x02, 'nanos_decline_create_credential', bothSeeds)));
test('[NANO X] Decline to export private key seed for creating credential', setupZemu('nanox', negativePrivateKeyExportTest(0x02, 'nanox_decline_create_credential', bothSeeds)));
