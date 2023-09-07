import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateHigherLevelKeys(
    sim: Zemu, transport: Transport,
    ins: number,
    updateType: string,
    keyUpdateType: string,
    expectedSignature: string,
    handleKeyUi: () => Promise<any>,
) {
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029', 'hex');
    data = Buffer.concat([data, Buffer.from(updateType, 'hex'), Buffer.from(keyUpdateType, 'hex'), Buffer.from('0002', 'hex')]);
    transport.send(0xe0, ins, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    let snapshot = await sim.clickBoth(undefined, false);

    const key1 = Buffer.from('00b6bc751f1abfb6440ff5cce27d7cdd1e7b0b8ec174f54de426890635b27e7daf', 'hex');
    const key2 = Buffer.from('0046a3e38ddf8b493be6e979034510b91db5448da9cba48c106139c288d658a004', 'hex');
    const keys = [key1, key2];

    for (const key of keys) {
        transport.send(0xe0, ins, 0x01, 0x00, key);
        await sim.waitUntilScreenIsNot(snapshot);
        snapshot = await handleKeyUi();
    }

    data = Buffer.from('0002', 'hex');
    const tx = transport.send(0xe0, ins, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot);
    await sim.clickRight();
    await sim.clickBoth(undefined, false);

    await expect(tx).resolves.toEqual(
        Buffer.from(expectedSignature, 'hex'),
    );
}

test('[NANO S] Update root keys', setupZemu('nanos', async (sim, transport) => {
    await updateHigherLevelKeys(
        sim,
        transport,
        0x28,
        '0A',
        '00',
        'dec8a3bfc46ee9c374c78d83ab1e4ce5af0461017c476b011863ee969cbd52b833cc0a32d2411bcade7940c754c3c25d33dad7260517409f7700982eb5b81e029000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight(undefined, false);
            return sim.clickBoth(undefined, false);
        },
    );
}));

test('[NANO S] Update level 1 with root keys', setupZemu('nanos', async (sim, transport) => {
    await updateHigherLevelKeys(
        sim,
        transport,
        0x28,
        '0A',
        '01',
        '54e9acf8880101da599f908aa55bff5c5e5bc1a1ce186cd1136d227e7370a42529bd1c1238aff0d603dce787fb705e043e7ef560ae12fabc0a9f22e21966d8099000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight(undefined, false);
            return sim.clickBoth(undefined, false);
        },
    );
}));

test('[NANO S] Update level 1 with level 1 keys', setupZemu('nanos', async (sim, transport) => {
    await updateHigherLevelKeys(
        sim,
        transport,
        0x29,
        '0B',
        '00',
        '8652d8ce5c3385712a636e0af7297b9355f3f0e7191220a8b1db337f31ac04bb1fc7f2dbc1e326e03f37e3e586c54c53c43428e874e713cd04a06b82094f49099000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight(undefined, false);
            return sim.clickBoth(undefined, false);
        },
    );
}));
