import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

test(
    '[NANO S] Create PLT - Phase 1 (Initial)',
    setupZemu('nanos', async (sim, transport) => {
        // Phase 1: Initial command with derivation path and update header
        const data = Buffer.from(
            '080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165',
            'hex'
        );
        const response = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
        expect(response).toEqual(Buffer.from('9000', 'hex'));
    })
);

test(
    '[NANO S] Create PLT - Phase 1 & 2 (Initial + Payload)',
    setupZemu('nanos', async (sim, transport) => {
        // Phase 1: Initial command
        let data = Buffer.from(
            '080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165',
            'hex'
        );
        const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
        expect(response1).toEqual(Buffer.from('9000', 'hex'));

        // Phase 2: Payload + init params length
        data = Buffer.from(
            '00000003545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000001',
            'hex'
        );
        const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);

        // This should succeed if our state machine is working
        expect(response2).toEqual(Buffer.from('9000', 'hex'));
    })
);

test(
    '[NANO S] Create PLT - Wrong P1 order (should fail)',
    setupZemu('nanos', async (sim, transport) => {
        // Phase 1: Initial command
        let data = Buffer.from(
            '080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165',
            'hex'
        );
        const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
        expect(response1).toEqual(Buffer.from('9000', 'hex'));

        // Skip Phase 2, try Phase 3 directly (should fail with INVALID_STATE)
        data = Buffer.from('ff', 'hex');
        const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

        await expect(tx).rejects.toMatchObject({
            message: expect.stringContaining('6b01'),
        });
    })
);

async function createPlt(sim: Zemu, transport: Transport, images: string, device?: 'nanos' | 'nanosp') {
    // Phase 1: Initial
    let data = Buffer.from(
        '080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029180000000000000165',
        'hex'
    );
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Phase 2: Payload + init params length
    data = Buffer.from(
        '00000003545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000001',
        'hex'
    );
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from('9000', 'hex'));

    // Phase 3: Init params (1 byte)
    data = Buffer.from('ff', 'hex');
    const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    switch (device) {
        case 'nanos':
            await sim.navigateAndCompareSnapshots('.', images, [9, 0]);
            break;
        case 'nanosp':
            await sim.navigateAndCompareSnapshots('.', images, [7, 0]);
            break;
        default:
            throw new Error(`Unsupported device: ${device}`);
    }
    await expect(tx).resolves.toEqual(
        Buffer.from(
            '0b84908dd2b8abf26c5a502b8a2f23c02bf38a5debcee6c2e9d480b9c57e6ddf98d3cd3d2d49c84be51a370b3f32379fc8cd4f9c5969e14886af2abb79dfa00a9000',
            'hex'
        )
    );
}

test(
    '[NANO S] Create PLT snapshots',
    setupZemu('nanos', async (sim, transport) => {
        await createPlt(sim, transport, 'nanos_create_plt', 'nanos');
    })
);

test(
    '[NANO SP] Create PLT snapshots',
    setupZemu('nanosp', async (sim, transport) => {
        await createPlt(sim, transport, 'nanosp_create_plt', 'nanosp');
    })
);
