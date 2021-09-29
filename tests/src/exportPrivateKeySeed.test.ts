import { setupZemu } from './options';

test('[NANO S] Export private key seed for decryption key (PRF-key seed)', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('ffffffff', 'hex');
    const tx = transport.send(0xe0, 0x05, 0x00, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanos_export_decrypt_prf_key_seed', [0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('5e623ff0aabdc082b9fb5a295612dee730133ac6a85736e3c12f62242a19ad1b9000', 'hex'),
    );
}));

test('[NANO X] Export private key seed for decryption key (PRF-key seed)', setupZemu('nanox', async (sim, transport) => {
    const data = Buffer.from('ffffffff', 'hex');
    const tx = transport.send(0xe0, 0x05, 0x00, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanox_export_decrypt_prf_key_seed', [0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('5e623ff0aabdc082b9fb5a295612dee730133ac6a85736e3c12f62242a19ad1b9000', 'hex'),
    );
}));

test('[NANO S] Export private key seed for recovery (PRF-key seed)', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('ffffffff', 'hex');
    const tx = transport.send(0xe0, 0x05, 0x01, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanos_export_recovery_prf_key_seed', [0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('5e623ff0aabdc082b9fb5a295612dee730133ac6a85736e3c12f62242a19ad1b9000', 'hex'),
    );
}));

test('[NANO X] Export private key seed for recovery (PRF-key seed)', setupZemu('nanox', async (sim, transport) => {
    const data = Buffer.from('ffffffff', 'hex');
    const tx = transport.send(0xe0, 0x05, 0x01, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanox_export_recovery_prf_key_seed', [0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('5e623ff0aabdc082b9fb5a295612dee730133ac6a85736e3c12f62242a19ad1b9000', 'hex'),
    );
}));

test('[NANO S] Export private key seed for creating credential (IdCredSec seed)', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('ffffffff', 'hex');
    const tx = transport.send(0xe0, 0x05, 0x02, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanos_export_credential_idcredsec_seed', [0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('5e623ff0aabdc082b9fb5a295612dee730133ac6a85736e3c12f62242a19ad1bb374793fd01c1dc4b0f2de2ab5dd34eae8824a6416035338cc1f6c35832f027a9000', 'hex'),
    );
}));

test('[NANO X] Export private key seed for creating credential (IdCredSec seed)', setupZemu('nanox', async (sim, transport) => {
    const data = Buffer.from('ffffffff', 'hex');
    const tx = transport.send(0xe0, 0x05, 0x02, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanox_export_credential_idcredsec_seed', [0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('5e623ff0aabdc082b9fb5a295612dee730133ac6a85736e3c12f62242a19ad1bb374793fd01c1dc4b0f2de2ab5dd34eae8824a6416035338cc1f6c35832f027a9000', 'hex'),
    );
}));
