import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function addIdentityProviderShared(sim: Zemu, transport: Transport) {
    // Initial pack
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000290d0000006200000001', 'hex');
    transport.send(0xe0, 0x2d, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    const snapshot1 = await sim.clickBoth(undefined, false);

    // Length of the name
    data = Buffer.from('00000009', 'hex');
    await transport.send(0xe0, 0x2d, 0x01, 0x00, data);

    // The name
    data = Buffer.from('54657374206e616d65', 'hex');
    transport.send(0xe0, 0x2d, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot1);
    const snapshot2 = await sim.clickBoth(undefined, false);

    // Length of the URL
    data = Buffer.from('00000015', 'hex');
    await transport.send(0xe0, 0x2d, 0x01, 0x00, data);

    // The URL
    data = Buffer.from('687474703a2f2f636f6e636f726469756d2e636f6d', 'hex');
    transport.send(0xe0, 0x2d, 0x02, 0x00, data);
    Zemu.sleep(1000);
    await sim.waitUntilScreenIsNot(snapshot2);
    // This right click is unnecessary for Nano X, but does nothing
    await sim.clickRight(undefined, false);
    const snapshot3 = await sim.clickBoth(undefined, false);

    // Length of the description
    data = Buffer.from('00000010', 'hex');
    await transport.send(0xe0, 0x2d, 0x01, 0x00, data);

    // The description
    data = Buffer.from('54657374206465736372697074696f6e', 'hex');
    transport.send(0xe0, 0x2d, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot3);
    return sim.clickBoth(undefined, false);
}

test('[NANO S] Add identity provider', setupZemu('nanos', async (sim, transport) => {
    const snapshot4 = await addIdentityProviderShared(sim, transport);

    // Verify key
    let data = Buffer.from('00000010', 'hex');
    transport.send(0xe0, 0x2d, 0x03, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot4);
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    const snapshot5 = await sim.clickBoth(undefined, false);

    // CDI key
    data = Buffer.from('37efcc5b9180fc9c43a5a51a2f27d6581e63e4b2b3dad75b8510061b8c2db39f', 'hex');
    const tx = transport.send(0xe0, 0x2d, 0x04, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot5);
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth(undefined, false);

    await expect(tx).resolves.toEqual(
        Buffer.from('8acfbd91b0726ccfd1830ae8df58a3e6bff8c9c079f6bf3c773ebb84b37a6df3c5fe3e1f0189d3f7aa60ffaa3bbac076962fb5204eabde3c9ca44a7c8354010c9000', 'hex'),
    );
}));

test('[NANO SP] Add identity provider', setupZemu('nanosp', async (sim, transport) => {
    const snapshot4 = await addIdentityProviderShared(sim, transport);

    // Verify key
    let data = Buffer.from('00000010', 'hex');
    transport.send(0xe0, 0x2d, 0x03, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot4);
    await sim.clickRight();
    const snapshot5 = await sim.clickBoth(undefined, false);

    // CDI key
    data = Buffer.from('37efcc5b9180fc9c43a5a51a2f27d6581e63e4b2b3dad75b8510061b8c2db39f', 'hex');
    const tx = transport.send(0xe0, 0x2d, 0x04, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot5);
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth(undefined, false);

    await expect(tx).resolves.toEqual(
        Buffer.from('8acfbd91b0726ccfd1830ae8df58a3e6bff8c9c079f6bf3c773ebb84b37a6df3c5fe3e1f0189d3f7aa60ffaa3bbac076962fb5204eabde3c9ca44a7c8354010c9000', 'hex'),
    );
}));
