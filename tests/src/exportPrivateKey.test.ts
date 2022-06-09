import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

// TODO: Enable NANO X Tests when the zemu container supports NANO X SDK v2.0

const end = '9000';
const path = '00000134';

const exportPrivateKeyTest = (p1: number, p2: number, picture: string, seed: string) => (async (sim: Zemu, transport: Transport, device: LedgerModel) => {
    const data = Buffer.from(path, 'hex');
    const tx = transport.send(0xe0, 0x05, p1, p2, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', device + '_' + picture, [1, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from(seed + end, 'hex'),
    );
});

const prfKeySeed = 'd9bd8b475cfb4f7c0e42e7059d27906c09f98760e1104c27430624f37da75707';
const prfKey = '48ba35a69084c13aa3bcd592e77390bc922e7283dcf928bd7068c76b9eb9ba58';
const bothSeeds = `${prfKeySeed}1a2f72de118c37e4029245b3acb6ff541ceffb70911b115800be025d8ed1c24b`;
const bothKeys = `${prfKey}60f76c759b19663a1b9cd6e0974e311b52240eb9fe769aafcb63e5da4d5d9ef6`;

// We want to test that all supported p1 choices are working

const exportDecryptPrfKey = 'export_decrypt_prf_key';
test('[NANO S] Export Bls private key for decryption key (PRF-key)', setupZemu('nanos', exportPrivateKeyTest(0x00, 0x02, exportDecryptPrfKey, prfKey)));
test('[NANO SP] Export Bls private key for decryption key (PRF-key)', setupZemu('nanosp', exportPrivateKeyTest(0x00, 0x02, exportDecryptPrfKey, prfKey)));
test('[NANO X] Export Bls private key for decryption key (PRF-key)', setupZemu('nanox', exportPrivateKeyTest(0x00, 0x02, exportDecryptPrfKey, prfKey)));

test('[NANO S] Export private key seed for decryption key (PRF-key seed)', setupZemu('nanos', exportPrivateKeyTest(0x00, 0x01, exportDecryptPrfKey, prfKeySeed)));
test('[NANO SP] Export private key seed for decryption key (PRF-key seed)', setupZemu('nanosp', exportPrivateKeyTest(0x00, 0x01, exportDecryptPrfKey, prfKeySeed)));
test('[NANO X] Export private key seed for decryption key (PRF-key seed)', setupZemu('nanox', exportPrivateKeyTest(0x00, 0x01, exportDecryptPrfKey, prfKeySeed)));

const exportRecoveryPrfKey = 'export_recovery_prf_key';
test('[NANO S] Export Bls private key for recovery (PRF-key)', setupZemu('nanos', exportPrivateKeyTest(0x01, 0x02, exportRecoveryPrfKey, prfKey)));
test('[NANO SP] Export Bls private key for recovery (PRF-key)', setupZemu('nanosp', exportPrivateKeyTest(0x01, 0x02, exportRecoveryPrfKey, prfKey)));
test('[NANO X] Export Bls private key for recovery (PRF-key)', setupZemu('nanox', exportPrivateKeyTest(0x01, 0x02, exportRecoveryPrfKey, prfKey)));
test('[NANO S] Export private key seed for recovery (PRF-key seed)', setupZemu('nanos', exportPrivateKeyTest(0x01, 0x01, exportRecoveryPrfKey, prfKeySeed)));
test('[NANO SP] Export private key seed for recovery (PRF-key seed)', setupZemu('nanosp', exportPrivateKeyTest(0x01, 0x01, exportRecoveryPrfKey, prfKeySeed)));
test('[NANO X] Export private key seed for recovery (PRF-key seed)', setupZemu('nanox', exportPrivateKeyTest(0x01, 0x01, exportRecoveryPrfKey, prfKeySeed)));

const exportCredentialIdCredSec = 'export_credential_idcredsec';
test('[NANO S] Export Bls private key for creating credential (PRF + IdCredSec)', setupZemu('nanos', exportPrivateKeyTest(0x02, 0x02, exportCredentialIdCredSec, bothKeys)));
test('[NANO SP] Export Bls private key for creating credential (PRF + IdCredSec)', setupZemu('nanosp', exportPrivateKeyTest(0x02, 0x02, exportCredentialIdCredSec, bothKeys)));
test('[NANO X] Export Bls private key for creating credential (PRF + IdCredSec)', setupZemu('nanox', exportPrivateKeyTest(0x02, 0x02, exportCredentialIdCredSec, bothKeys)));
test('[NANO S] Export private key seed for creating credential (PRF seed + IdCredSec seed)', setupZemu('nanos', exportPrivateKeyTest(0x02, 0x01, exportCredentialIdCredSec, bothSeeds)));
test('[NANO SP] Export private key seed for creating credential (PRF seed + IdCredSec seed)', setupZemu('nanosp', exportPrivateKeyTest(0x02, 0x01, exportCredentialIdCredSec, bothSeeds)));
test('[NANO X] Export private key seed for creating credential (PRF seed + IdCredSec seed)', setupZemu('nanox', exportPrivateKeyTest(0x02, 0x01, exportCredentialIdCredSec, bothSeeds)));

const negativePrivateKeyExportTest = (p1: number, p2: number, picture: string) => (async (sim: Zemu, transport: Transport, device: LedgerModel) => {
    expect.assertions(1);
    const data = Buffer.from(path, 'hex');
    const tx = transport.send(0xe0, 0x05, p1, p2, data).catch((e) => expect(e.statusCode).toEqual(27013));
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', device + '_' + picture, [2, 0]);
});

const declineDecrypt = 'decline_decrypt';
test('[NANO S] Decline to export private key seed for decryption', setupZemu('nanos', negativePrivateKeyExportTest(0x00, 0x01, declineDecrypt)));
test('[NANO SP] Decline to export private key seed for decryption', setupZemu('nanosp', negativePrivateKeyExportTest(0x00, 0x01, declineDecrypt)));
test('[NANO X] Decline to export private key seed for decryption', setupZemu('nanox', negativePrivateKeyExportTest(0x00, 0x01, declineDecrypt)));

const declineRecovery = 'decline_recovery';
test('[NANO S] Decline to export private key seed for recovery', setupZemu('nanos', negativePrivateKeyExportTest(0x01, 0x01, declineRecovery)));
test('[NANO SP] Decline to export private key seed for recovery', setupZemu('nanosp', negativePrivateKeyExportTest(0x01, 0x01, declineRecovery)));
test('[NANO X] Decline to export private key seed for recovery', setupZemu('nanox', negativePrivateKeyExportTest(0x01, 0x01, declineRecovery)));

const declineCreateCredential = 'decline_create_credential';
test('[NANO S] Decline to export private key seed for creating credential', setupZemu('nanos', negativePrivateKeyExportTest(0x02, 0x01, declineCreateCredential)));
test('[NANO SP] Decline to export private key seed for creating credential', setupZemu('nanosp', negativePrivateKeyExportTest(0x02, 0x01, declineCreateCredential)));
test('[NANO X] Decline to export private key seed for creating credential', setupZemu('nanox', negativePrivateKeyExportTest(0x02, 0x01, declineCreateCredential)));

const exportPrivateKeyTestNoPictures = (path: string, p1: number, key: string) => (async (sim: Zemu, transport: Transport) => {
    const data = Buffer.from(path, 'hex');
    const tx = transport.send(0xe0, 0x05, p1, 0x02, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from(key + end, 'hex'),
    );
});

/** * We want to test that the keygen works with different seeds ** */
// Keys have been derived using https://github.com/paulmillr/bls12-381-keygen
// and double checked with https://github.com/ChainSafe/bls-hd-key
const tests = [];

// seed = 09e74ad3ead373439388bf7cfb52b151c450632e67f3c84e6ed762bc0928d5eb, 9dcce7d6b6a70ddb382d2c212273ad9b99d8ea206353313beabc14b7e5833a0f
tests.push(['00000001', '57cb278c9deb055f12cc807c3068f2ce804654a54de54801f0cb6a774c211de2' + '273e082676db5baff938c9aa5f92ea73689a3de4bf20f71c4710eba2ced4925e']);

// seed = 8a430bd1343b5cb1686961d7959095214952767f80bb681e465aebf467ca05c0, c940716ce93f72b2d4f9ff792193ee541e3748ff5b13f1bb01d5d091af4e2635
tests.push(['00000002', '710b517c90a46f8c13455e9d50ebe74ba660c611430e134bda449f766b605ac6' + '340d8fe689c3c19064bdbadff74a23a57b22ff5ec56a9bb836923c1d9d1720ee']);

// seed = bc14d035741cd9225795ad1e7070a1b488deb0cc19daedcb47f12408cbfe71cc, 00bbfd863df0068b841c12f6e6a8d6c571c80eaf37fae324c669d214f90a762c
tests.push(['00000003', '2b614017245f7b7bd2afc89e6cc9287391b7c063cd92f5dedf881f6d430558f6' + '5dcba268f56cec11fac41f31779d35592baa44f7bc1e8278bf1ea394b452805d']);

// seed = 30c0a09b5175febb462b64efe28475fa434b0ccfb05bc77d16fd2f1ba13bc05c, 34a3f5d8b0cfaa9d235c8c1ecfbf10223259d45be5f4d30b33c697e19ac9e525
tests.push(['00000004', '2f72e2fedead5ab70eea0da85ab016cdd083b4805f6374ed4313ea1a643ff7b1' + '32ce1b167e4220499c2033f2574611c5598f2cabfcdb0d5f2efc66c083ad9d91']);

// seed = d57e90d96aab5616c266568ad4659dfe985f8baf24042e5e28e901801944f49d, 725150bb38d49fc2a7ad8a8cba1f0dc32c3a468739e88b9aa62c450ce3ce1f32
tests.push(['00000005', '5ca731e9adf63fa8ac75918824aaab3bafcea4480dfd8636b5d82ce693cfefaa' + '397048d5f83ecb69fe96f3157bb5a350298248f8650b9092a8c55028fb577463']);

for (const [path, seed] of tests) {
    test('[NANO S] Export Bls private key', setupZemu('nanos', exportPrivateKeyTestNoPictures(path, 0x02, seed)));
}
