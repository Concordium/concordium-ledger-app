import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

test('[NANO S] Create PLT - Phase 1 (Initial)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command with derivation path and update header
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918', 'hex');
    const response = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response).toEqual(Buffer.from('9000', 'hex'));
}));

test('[NANO S] Create PLT - Phase 1 & 2 (Initial + Payload)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Phase 2: Payload + init params length
    data = Buffer.from('03545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000001', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);

    // This should succeed if our state machine is working
    expect(response2).toEqual(Buffer.from('9000', 'hex'));
}));

test('[NANO S] Create PLT - Wrong P1 order (should fail)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Skip Phase 2, try Phase 3 directly (should fail with INVALID_STATE)
    data = Buffer.from('ff', 'hex');
    const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

    await expect(tx).rejects.toMatchObject({
        message: expect.stringContaining('6b01')
    });
}));

async function createPlt(sim: Zemu, transport: Transport, images: string) {
    // Phase 1: Initial
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Phase 2: Payload + init params length
    data = Buffer.from('03545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000001', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from('9000', 'hex'));

    // Phase 3: Init params (1 byte)
    data = Buffer.from('ff', 'hex');
    const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', images, [8, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from('4a623bcb4b21f7aa79eed1d443e7835c6079d43a414be67a96e21064a31d5b8d09643841754199f75d92595ee4a0a6e7220ce47fab75a1906e944fcb9cd8e3059000', 'hex'),
    );
}

test('[NANO S] Create PLT snapshots', setupZemu('nanos', async (sim, transport) => {
    await createPlt(sim, transport, 'nanos_create_plt');
}));
