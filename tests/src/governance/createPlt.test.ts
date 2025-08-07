import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

test('[NANO S] Create PLT - Phase 1 (Initial)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command with derivation path and update header
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165', 'hex');
    const response = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response).toEqual(Buffer.from('9000', 'hex'));
}));

test('[NANO S] Create PLT - Phase 1 & 2 (Initial + Payload)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Phase 2: Payload + init params length
    data = Buffer.from('00000003545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000001', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);

    // This should succeed if our state machine is working
    expect(response2).toEqual(Buffer.from('9000', 'hex'));
}));

test('[NANO S] Create PLT - Wrong P1 order (should fail)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Skip Phase 2, try Phase 3 directly (should fail with INVALID_STATE)
    data = Buffer.from('ff', 'hex');
    const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

    await expect(tx).rejects.toMatchObject({
        message: expect.stringContaining('6b01')
    });
}));

test('[NANO S] Create PLT - Full flow minimal', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Phase 2: Payload + init params length
    data = Buffer.from('00000003545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000001', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from('9000', 'hex'));

    // Phase 3: Init params (1 byte) - This will show UI and wait
    data = Buffer.from('ff', 'hex');
    const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

    // Wait for UI to appear after Phase 3
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

    // Navigate through the UI screens quickly - just navigate to sign
    // Skip through all screens to get to the final sign/decline choice
    // Standard screens: Review -> Token Symbol -> Token Module (4 pages) -> Decimals -> Init Params (1 page hex) -> Sign
    for (let i = 0; i < 8; i++) {
        await sim.clickRight();
    }

    // Now we should be at the Sign/Decline screen - click Sign
    await sim.clickBoth(); // Sign

    await expect(tx).resolves.toEqual(
        Buffer.from('0b84908dd2b8abf26c5a502b8a2f23c02bf38a5debcee6c2e9d480b9c57e6ddf98d3cd3d2d49c84be51a370b3f32379fc8cd4f9c5969e14886af2abb79dfa00a9000', 'hex'),
    );
}));

test('[NANO S] Create PLT - Large init params (192 bytes)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Phase 2: Payload + init params length (192 bytes)
    data = Buffer.from('00000003545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db2006000000c0', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from('9000', 'hex'));

    // Phase 3: Init params (192 bytes of data)
    // Create a pattern that will be easy to identify
    const initParamsData = Buffer.alloc(192);
    for (let i = 0; i < 192; i++) {
        initParamsData[i] = i % 256;
    }

    const tx = transport.send(0xe0, 0x48, 0x02, 0x00, initParamsData);

    // Wait for UI to appear after Phase 3
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

    // Navigate through the UI screens - init params shows hex content across multiple pages
    // Standard screens: Review -> Token Symbol -> Token Module (4 pages) -> Decimals -> Init Params (25+ hex pages) -> Sign
    // Total: 1 + 1 + 4 + 1 + 25+ + 1 = 32+ screens
    for (let i = 0; i < 32; i++) { // move through 32 screens to land on the sign page
        await sim.clickRight();
    }

    // Now we should be at the Sign/Decline screen - click Sign
    await sim.clickBoth(); // Sign

    await expect(tx).resolves.toEqual(
        Buffer.from('0549aea02cbe4f53e7b4e6827cdab0fd967831893c56c3cfb45eb0b3b2f68593b5fa8d28dc7619f34cf357a6493cc70e5034bf86c3fc12e888f3dc0d3d74ca0e9000', 'hex'),
    );
}));
