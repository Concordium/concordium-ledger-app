import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function minBlockTime(sim: Zemu, transport: Transport, images: string) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000291300000f0a00000001', 'hex');
    const tx = transport.send(0xe0, 0x44, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', images, [2, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from('b5ad05e150633db414215d10532f656624ba481119bec95240dbb6c556d6afcfc5c8f8a6e01a0de73a57ae38d1ecd50534e0930e626ec984ba6f69dfd0ee24069000', 'hex'),
    );
}

test('[NANO S] Min block time', setupZemu('nanos', async (sim, transport) => {
    await minBlockTime(sim, transport, 'nanos_min_block_time');
}));

test('[NANO SP] Min block time', setupZemu('nanosp', async (sim, transport) => {
    await minBlockTime(sim, transport, 'nanosp_min_block_time');
}));
