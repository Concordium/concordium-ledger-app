import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function addAnonymityRevokerShared(sim: Zemu, transport: Transport) {
    // Initial pack
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000290c0000009e00000001', 'hex');
    transport.send(0xe0, 0x2c, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    const snapshot1 = await sim.clickBoth(undefined, false);

    // Length of the name
    data = Buffer.from('00000009', 'hex');
    await transport.send(0xe0, 0x2c, 0x01, 0x00, data);

    // The name
    data = Buffer.from('54657374206e616d65', 'hex');
    transport.send(0xe0, 0x2c, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot1);
    const snapshot2 = await sim.clickBoth(undefined, false);

    // Length of the URL
    data = Buffer.from('00000015', 'hex');
    await transport.send(0xe0, 0x2c, 0x01, 0x00, data);

    // The URL
    data = Buffer.from('687474703a2f2f636f6e636f726469756d2e636f6d', 'hex');
    transport.send(0xe0, 0x2c, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot2);
    await sim.clickRight(undefined, false);
    const snapshot3 = await sim.clickBoth(undefined, false);

    // Length of the description
    data = Buffer.from('00000010', 'hex');
    await transport.send(0xe0, 0x2c, 0x01, 0x00, data);

    // The description
    data = Buffer.from('54657374206465736372697074696f6e', 'hex');
    transport.send(0xe0, 0x2c, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot3);
    return sim.clickBoth(undefined, false);
}

test('[NANO S] Add anonymity revoker', setupZemu('nanos', async (sim, transport) => {
    const snapshot = await addAnonymityRevokerShared(sim, transport);

    // The AR public key
    const data = Buffer.from('993fdc40bb8af4cb75caf8a53928d247be6285784b29578a06df312c28854c1bfac2fd0183967338b578772398d412018a4afcfaae1ba3ccd63a5b0868a8a9c49deec35a8817d35d0082761b39c0c6bd2357ef997c0f319fefd5b336e6667b7b', 'hex');
    const tx = transport.send(0xe0, 0x2c, 0x03, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot);

    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('82f58dd72bb0ead83139282032da854b1077a341646db5bacd227554ab985b7814fb9aae9ae2cd6a5557b6e6174c1524a9cf81acae3bf1422cd43e63f5189c099000', 'hex'),
    );
}));
