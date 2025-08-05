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

    // Phase 2: Payload
    data = Buffer.from('00000003545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d706', 'hex');
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
    data = Buffer.from('00000020', 'hex');
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

    // Phase 2: Payload
    data = Buffer.from('00000003545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d706', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from('9000', 'hex'));

    // Phase 3: Init params length (minimal - 1 byte) - No UI yet, just returns success
    data = Buffer.from('00000001', 'hex');
    const response3 = await transport.send(0xe0, 0x48, 0x02, 0x00, data);
    expect(response3).toEqual(Buffer.from('9000', 'hex'));

    // Phase 4: Init params (1 byte) - This will show UI and wait
    data = Buffer.from('ff', 'hex');
    const tx = transport.send(0xe0, 0x48, 0x03, 0x00, data);

    // Wait for UI to appear after Phase 4
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

    // Navigate through the UI screens quickly - just navigate to sign
    // Skip through all screens to get to the final sign/decline choice
    // Standard screens: Review -> Token Symbol -> Token Module (4 pages) -> Governance Account (4 pages) -> Decimals -> Init Params (1 page hex) -> Sign -> Decline
    for (let i = 0; i < 12; i++) {
        await sim.clickRight();
    }

    // Now we should be at the Sign/Decline screen - click Sign
    await sim.clickBoth(); // Sign

    await expect(tx).resolves.toEqual(
        Buffer.from('b19bfe5fc65c4aed527a00a388e7328450c3c8762cf87c4058f4272741d943622b2ace5f8fe7530758bdff76e896562d973170e9daf240cde9fdfd19eda46e0f9000', 'hex'),
    );
}));

test('[NANO S] Create PLT - Large init params (192 bytes)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Phase 2: Payload
    data = Buffer.from('00000003545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d706', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from('9000', 'hex'));

    // Phase 3: Init params length (192 bytes)
    data = Buffer.from('000000c0', 'hex');
    const response3 = await transport.send(0xe0, 0x48, 0x02, 0x00, data);
    expect(response3).toEqual(Buffer.from('9000', 'hex'));

    // Phase 4: Init params (192 bytes of data)
    // Create a pattern that will be easy to identify
    const initParamsData = Buffer.alloc(192);
    for (let i = 0; i < 192; i++) {
        initParamsData[i] = i % 256;
    }

    const tx = transport.send(0xe0, 0x48, 0x03, 0x00, initParamsData);

    // Wait for UI to appear after Phase 4
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

    // Navigate through the UI screens - init params shows hex content across multiple pages
    // Standard screens: Review -> Token Symbol -> Token Module (4 pages) -> Governance Account (4 pages) -> Decimals -> Init Params (25+ hex pages) -> Sign -> Decline
    // Total: 1 + 1 + 4 + 4 + 1 + 25+ + 1 + 1 = 38+ screens
    for (let i = 0; i < 36; i++) { // move through 36 screens to land on the sign page
        await sim.clickRight();
    }

    // Now we should be at the Sign/Decline screen - click Sign
    await sim.clickBoth(); // Sign

    await expect(tx).resolves.toEqual(
        Buffer.from('985f0b44e0496eb45587d7b8299221b6afd984f495cb34a676d11982fda11b3d333ecb62802a83d1740ea2c12655df6bf211b4ed7e80aae608ce090da02cfb019000', 'hex'),
    );
}));
