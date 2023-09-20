import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function minBlockTime(sim: Zemu, transport: Transport, images: string) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000291400000000000b0001', 'hex');
    const tx = transport.send(0xe0, 0x45, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', images, [2, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from('ea11a90cafdc7a6bd6b2899649f480259785dda1a743167257ac1a0361a10414730b6d267089245eec27622c5fdc5830b91f0cbfd393f36175512292edc0a00f9000', 'hex'),
    );
}

test('[NANO S] Block energy limit', setupZemu('nanos', async (sim, transport) => {
    await minBlockTime(sim, transport, 'nanos_block_energy_limit');
}));

test('[NANO SP] Block energy limit', setupZemu('nanosp', async (sim, transport) => {
    await minBlockTime(sim, transport, 'nanosp_block_energy_limit');
}));
