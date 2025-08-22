import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateAuthorizations(
    sim: Zemu,
    transport: Transport,
    ins: number,
    level: string,
    type: string,
    prefix: string,
    expectedSignature: string
) {
    const device = sim.startOptions.model;

    let structureCount: number;
    let p2: number;

    switch (true) {
        // V0
        case type === '0a' && prefix === '02':
        case type === '0b' && prefix === '01':
            p2 = 0x00;
            structureCount = 12;
            break;
        // V1
        case type === '0a' && prefix === '03':
        case type === '0b' && prefix === '02':
            p2 = 0x01;
            structureCount = 14;
            break;
        // V2
        case type === '0a' && prefix === '04':
        case type === '0b' && prefix === '03':
            p2 = 0x02;
            structureCount = 15;
            break;
        default:
            throw new Error('Unsupported type/prefix pair');
    }

    const data = Buffer.concat([
        Buffer.from(
            '080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029',
            'hex'
        ),
        Buffer.from(type, 'hex'),
        Buffer.from(prefix, 'hex'),
        Buffer.from('0002', 'hex'),
    ]);

    transport.send(0xe0, ins, 0x00, p2, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', `${device}_update_authorizations/init_${level}`, [1]);
    let snapshot = await sim.clickBoth(undefined, false);

    const key1 = Buffer.from('00b6bc751f1abfb6440ff5cce27d7cdd1e7b0b8ec174f54de426890635b27e7daf', 'hex');
    const key2 = Buffer.from('0046a3e38ddf8b493be6e979034510b91db5448da9cba48c106139c288d658a004', 'hex');
    const keys = [key1, key2];

    for (const key of keys) {
        transport.send(0xe0, ins, 0x01, p2, key);
        await sim.waitUntilScreenIsNot(snapshot);
        switch (device) {
            case 'nanos':
                await sim.clickRight();
                await sim.clickRight();
                await sim.clickRight();
                await sim.clickRight(undefined, false);
                snapshot = await sim.clickBoth(undefined, false);
                break;
            case 'nanosp':
                await sim.clickRight();
                snapshot = await sim.clickBoth(undefined, false);
                break;
            default:
                throw new Error(`Unsupported device${device}`);
        }
    }

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

        await sim.navigateAndCompareSnapshots('.', `${device}_update_authorizations/${i}`, [0]);

        await sim.clickBoth(undefined, false);
        await sim.clickBoth(undefined, false);
        await sim.clickBoth(undefined, false);
        snapshot = await sim.clickBoth(undefined, false);

        const threshold = Buffer.from('0002', 'hex');
        const tx = transport.send(0xe0, ins, 0x04, p2, threshold);
        await sim.waitUntilScreenIsNot(snapshot);
        snapshot = await sim.clickBoth(undefined, false);
        if (i === structureCount - 1) {
            await sim.clickBoth();
            const txHex = (await tx).toString('hex');
            expect(txHex).toEqual(`${expectedSignature}9000`);
        }
    }
}

// Auths V0 tests
test(
    '[NANO SP] Update level 2 keys with root keys (V0)',
    setupZemu('nanosp', async (sim, transport) => {
        await updateAuthorizations(
            sim,
            transport,
            0x2a,
            '2_root',
            '0a',
            '02',
            'e9740c83110c2bc4fe876da81050cff27c1ab282178a3049327c6e97ebea34f2cd5f038e2b8dff466b1455e4d667d95d050fed1af04ea80c40360854dede1204'
        );
    })
);

test(
    '[NANO SP] Update level 2 keys with level 1 keys (V0)',
    setupZemu('nanosp', async (sim, transport) => {
        await updateAuthorizations(
            sim,
            transport,
            0x2b,
            '2_1',
            '0b',
            '01',
            'b3843361c16bc8f9f7792df766f7192e8b77aae420c958d290d8408b9ac224350998d4c264e4e60c577385963a574391b122948afae0dcb1e0ae7fda8076150e'
        );
    })
);

// Auths V1 tests
test(
    '[NANO SP] Update level 2 keys with root keys (V1)',
    setupZemu('nanosp', async (sim, transport) => {
        await updateAuthorizations(
            sim,
            transport,
            0x2a,
            '2_root',
            '0a',
            '03',
            '02f76c94c76d552364e8b054001f86c2cd4417de90343920558ac00529e2bc429f6db1a37c7f75fea69ec663bfadcc847ad568996545de93adfa1674b12d4107'
        );
    })
);

test(
    '[NANO SP] Update level 2 keys with level 1 keys (V1)',
    setupZemu('nanosp', async (sim, transport) => {
        await updateAuthorizations(
            sim,
            transport,
            0x2b,
            '2_1',
            '0b',
            '02',
            'ab92d78f158730042a1a4c2f738600f064ad5100551afad3760053f8f9e28e12d47a4e0484417f879e065332612e3cc4c5315e4f610f3fd4e3d7469c05855601'
        );
    })
);

// Auths V2 tests
test(
    '[NANO S] Update level 2 keys with root keys (V2)',
    setupZemu('nanos', async (sim, transport) => {
        await updateAuthorizations(
            sim,
            transport,
            0x2a,
            '2_root',
            '0a',
            '04',
            '8796c38ae70e36f292610ea2b4e2d593464e3edeecb86855fad40d8fb90d68b6b8d001cb1bf910057ec9ffdf67096d7d35819cf37b7b160e756bf07991be8202'
        );
    })
);

test(
    '[NANO SP] Update level 2 keys with root keys (V2)',
    setupZemu('nanosp', async (sim, transport) => {
        await updateAuthorizations(
            sim,
            transport,
            0x2a,
            '2_root',
            '0a',
            '04',
            '8796c38ae70e36f292610ea2b4e2d593464e3edeecb86855fad40d8fb90d68b6b8d001cb1bf910057ec9ffdf67096d7d35819cf37b7b160e756bf07991be8202'
        );
    })
);

test(
    '[NANO S] Update level 2 keys with level 1 keys (V2)',
    setupZemu('nanos', async (sim, transport) => {
        await updateAuthorizations(
            sim,
            transport,
            0x2b,
            '2_1',
            '0b',
            '03',
            '5c738eeff10af8196ddaaca50bca1c148a75e8606bb3290bf6c4ac97fc93c3c167e766b1e82b1b921038b246a240ad5f8de100cbc7574c4b049f315bfad6b50a'
        );
    })
);

test(
    '[NANO SP] Update level 2 keys with level 1 keys (V2)',
    setupZemu('nanosp', async (sim, transport) => {
        await updateAuthorizations(
            sim,
            transport,
            0x2b,
            '2_1',
            '0b',
            '03',
            '5c738eeff10af8196ddaaca50bca1c148a75e8606bb3290bf6c4ac97fc93c3c167e766b1e82b1b921038b246a240ad5f8de100cbc7574c4b049f315bfad6b50a'
        );
    })
);
