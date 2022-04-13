import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function poolParameters(sim: Zemu, transport: Transport, images: string, equity_steps: number) {
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029' + '0F' + '0000EA30' + '00000200' + '0000EA04', 'hex');
    transport.send(0xe0, 0x41, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', images + "_initial", [3]);
    let snapshot = await sim.clickBoth();
    data = Buffer.from('00001001' + '0000EA64' + '0000EA63' + '0000EA65' + '00001000' + '0000EA62', 'hex');
    transport.send(0xe0, 0x41, 0x01, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot);
    await sim.navigateAndCompareSnapshots('.', images + "_ranges", [5]);
    snapshot = await sim.clickBoth();

    data = Buffer.from('00000A00C60D5000' + 'C60D5000' + '00000A00C60D5000' + '0000EA600000EA60', 'hex');
    const tx = transport.send(0xe0, 0x41, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot);
    await sim.navigateAndCompareSnapshots('.', images + "_equity", [equity_steps, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from('f3ff3598d29e66836fe4b2712295f57e37f82a47b9772fe0a6c33128eb478e2dc5e0f199691bd1f62dc6c24d459e70bb835360795de84f09be6ae38a5dc76b0a9000', 'hex'),
    );
}

test('[NANO S] Pool parameters', setupZemu('nanos', async (sim, transport) => {
    await poolParameters(sim, transport, 'nanos_pool_parameters', 4);
}));

test('[NANO X] Pool parameters', setupZemu('nanox', async (sim, transport) => {
    await poolParameters(sim, transport, 'nanox_pool_parameters', 3);
}));
