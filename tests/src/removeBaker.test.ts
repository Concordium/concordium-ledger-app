import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { Model } from './helpers';
import { setupZemu } from './options';

async function removeBakerShared(transport: Transport) {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da705', 'hex');
    return transport.send(0xe0, 0x14, 0x00, 0x00, data);
}

test('[NANO S] Remove baker', setupZemu('nanos', async (sim, transport) => {
    const tx = removeBakerShared(transport);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanos_remove_baker', [7]);
    await sim.clickBoth(undefined, false);

    await expect(tx).resolves.toEqual(
        Buffer.from('4dcdbe47a5b2e55022d1206b1e4f2c3cce527529f954d2d17a3162e68834d9b54a4f0542b4d4b1b44894fd890c282c7282fe66cd0b7aae29c8383bb2ecd8a80f9000', 'hex'),
    );
}));

async function removeBakerXAndSP(sim: Zemu, transport: Transport, device: Model) {
    const tx = removeBakerShared(transport);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', device + '_remove_baker', [4]);
    await sim.clickBoth(undefined, false);

    await expect(tx).resolves.toEqual(
        Buffer.from('4dcdbe47a5b2e55022d1206b1e4f2c3cce527529f954d2d17a3162e68834d9b54a4f0542b4d4b1b44894fd890c282c7282fe66cd0b7aae29c8383bb2ecd8a80f9000', 'hex'),
    );
}

test('[NANO SP] Remove baker', setupZemu('nanosp', async (sim, transport) => {
    await removeBakerXAndSP(sim, transport, 'nanosp');
}));

test('[NANO X] Remove baker', setupZemu('nanox', async (sim, transport) => {
    await removeBakerXAndSP(sim, transport, 'nanox');
}));
