import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateAuthorizations(
    sim: Zemu, transport: Transport,
    ins: number,
    p2: number,
    type: string,
    prefix: string,
    expectedSignature: string,
    handleKeyUi: () => Promise<any>,
) {
    const data = Buffer.concat([
        Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029', 'hex'),
        Buffer.from(type, 'hex'),
        Buffer.from(prefix, 'hex'),
        Buffer.from('0002', 'hex'),
    ]);

    transport.send(0xe0, ins, 0x00, p2, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    let snapshot = await sim.clickBoth();

    const key1 = Buffer.from('00b6bc751f1abfb6440ff5cce27d7cdd1e7b0b8ec174f54de426890635b27e7daf', 'hex');
    const key2 = Buffer.from('0046a3e38ddf8b493be6e979034510b91db5448da9cba48c106139c288d658a004', 'hex');
    const keys = [key1, key2];

    for (const key of keys) {
        transport.send(0xe0, ins, 0x01, p2, key);
        await sim.waitUntilScreenIsNot(snapshot);
        snapshot = await handleKeyUi();
    }

    const structureCount = p2 ? 14 : 12;
    // Go through each access structure
    for (let i = 0; i < structureCount; i += 1) {
        const accessStructureSize = Buffer.from('0003', 'hex');
        await transport.send(0xe0, ins, 0x02, p2, accessStructureSize);

        const keyIndex1 = Buffer.from('0001', 'hex');
        const keyIndex2 = Buffer.from('00F1', 'hex');
        const keyIndex3 = Buffer.from('000C', 'hex');

        const accessStructureData = Buffer.concat([keyIndex1, keyIndex2, keyIndex3]);
        transport.send(0xe0, ins, 0x03, p2, accessStructureData);
        await sim.waitUntilScreenIsNot(snapshot);
        await sim.clickBoth();
        await sim.clickBoth();
        await sim.clickBoth();
        snapshot = await sim.clickBoth();

        const threshold = Buffer.from('0002', 'hex');
        const tx = transport.send(0xe0, ins, 0x04, p2, threshold);
        await sim.waitUntilScreenIsNot(snapshot);
        snapshot = await sim.clickBoth();
        if (i === structureCount - 1) {
            await sim.clickBoth();
            await expect(tx).resolves.toEqual(
                Buffer.from(expectedSignature, 'hex'),
            );
        }
    }
}

test('[NANO S] Update level 2 keys with root keys v0', setupZemu('nanos', async (sim, transport) => {
    await updateAuthorizations(
        sim,
        transport,
        0x2a,
        0x00,
        '0a',
        '02',
        'e9740c83110c2bc4fe876da81050cff27c1ab282178a3049327c6e97ebea34f2cd5f038e2b8dff466b1455e4d667d95d050fed1af04ea80c40360854dede12049000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            return sim.clickBoth();
        },
    );
}));

test('[NANO X] Update level 2 keys with root keys v0', setupZemu('nanox', async (sim, transport) => {
    await updateAuthorizations(
        sim,
        transport,
        0x2a,
        0x00,
        '0a',
        '02',
        'e9740c83110c2bc4fe876da81050cff27c1ab282178a3049327c6e97ebea34f2cd5f038e2b8dff466b1455e4d667d95d050fed1af04ea80c40360854dede12049000',
        async () => {
            await sim.clickRight();
            return sim.clickBoth();
        },
    );
}));

test('[NANO S] Update level 2 keys with level 1 keys v0', setupZemu('nanos', async (sim, transport) => {
    await updateAuthorizations(
        sim,
        transport,
        0x2b,
        0x00,
        '0b',
        '01',
        'b3843361c16bc8f9f7792df766f7192e8b77aae420c958d290d8408b9ac224350998d4c264e4e60c577385963a574391b122948afae0dcb1e0ae7fda8076150e9000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            return sim.clickBoth();
        },
    );
}));

test('[NANO X] Update level 2 keys with level 1 keys v0', setupZemu('nanox', async (sim, transport) => {
    await updateAuthorizations(
        sim,
        transport,
        0x2b,
        0x00,
        '0b',
        '01',
        'b3843361c16bc8f9f7792df766f7192e8b77aae420c958d290d8408b9ac224350998d4c264e4e60c577385963a574391b122948afae0dcb1e0ae7fda8076150e9000',
        async () => {
            await sim.clickRight();
            return sim.clickBoth();
        },
    );
}));

test('[NANO S] Update level 2 keys with root keys v1', setupZemu('nanos', async (sim, transport) => {
    await updateAuthorizations(
        sim,
        transport,
        0x2a,
        0x01,
        '0a',
        '02',
        'da4e5df15a4e6b8253b633ec82facc8a79e1a411ecdce9734aa3b302a7ae9652311634c0a6f98b28ce1d2e070c46b6a971977d86edac95e734091e2b11284f019000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            return sim.clickBoth();
        },
    );
}));

test('[NANO X] Update level 2 keys with root keys v1', setupZemu('nanox', async (sim, transport) => {
    await updateAuthorizations(
        sim,
        transport,
        0x2a,
        0x01,
        '0a',
        '02',
        'da4e5df15a4e6b8253b633ec82facc8a79e1a411ecdce9734aa3b302a7ae9652311634c0a6f98b28ce1d2e070c46b6a971977d86edac95e734091e2b11284f019000',
        async () => {
            await sim.clickRight();
            return sim.clickBoth();
        },
    );
}));

test('[NANO S] Update level 2 keys with level 1 keys v1', setupZemu('nanos', async (sim, transport) => {
    await updateAuthorizations(
        sim,
        transport,
        0x2b,
        0x01,
        '0b',
        '01',
        'b59fef225994cab4fc3ff9ea2be4e4acf44e7d1f65fecdddacfa73780e1b88d3736d6239d7e3a6b06ffebc0db4938ad138732c47fcda12b6242702f5ddf033069000',
        async () => {
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            await sim.clickRight();
            return sim.clickBoth();
        },
    );
}));

test('[NANO X] Update level 2 keys with level 1 keys v1', setupZemu('nanox', async (sim, transport) => {
    await updateAuthorizations(
        sim,
        transport,
        0x2b,
        0x01,
        '0b',
        '01',
        'b59fef225994cab4fc3ff9ea2be4e4acf44e7d1f65fecdddacfa73780e1b88d3736d6239d7e3a6b06ffebc0db4938ad138732c47fcda12b6242702f5ddf033069000',
        async () => {
            await sim.clickRight();
            return sim.clickBoth();
        },
    );
}));
