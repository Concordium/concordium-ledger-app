import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

async function poolParameters(sim: Zemu, transport: Transport, device: LedgerModel, equitySteps: number) {
    const images = `${device}_pool_parameters`;
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029' + '0F' + '0000EA30' + '00000200' + '0000EA04', 'hex');
    transport.send(0xe0, 0x41, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', `${images}/initial`, [3]);
    let snapshot = await sim.clickBoth(undefined, false);
    data = Buffer.from('00001001' + '0000EA64' + '0000EA63' + '0000EA65' + '00001000' + '0000EA62', 'hex');
    transport.send(0xe0, 0x41, 0x01, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot);
    await sim.navigateAndCompareSnapshots('.', `${images}/ranges`, [5]);
    snapshot = await sim.clickBoth(undefined, false);

    data = Buffer.from('00000A00C60D5000' + '00005000' + '00000A00C60D5000' + '0000EA600000EA60', 'hex');
    const tx = transport.send(0xe0, 0x41, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot);
    await sim.navigateAndCompareSnapshots('.', `${images}/equity`, [equitySteps, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from('1349789ad4cf49bda28d2e1178bcbf71831163b3a00b627b379d9aeb3096a35dd43a60a77a3a7a9eb922ba749686bdf1fbc1756357db65f127a732326614eb0d9000', 'hex'),
    );
}

test('[NANO S] Pool parameters', setupZemu('nanos', async (sim, transport, device) => {
    await poolParameters(sim, transport, device, 4);
}));

test('[NANO SP] Pool parameters', setupZemu('nanosp', async (sim, transport, device) => {
    await poolParameters(sim, transport, device, 3);
}));

test('[NANO X] Pool parameters', setupZemu('nanox', async (sim, transport, device) => {
    await poolParameters(sim, transport, device, 3);
}));
