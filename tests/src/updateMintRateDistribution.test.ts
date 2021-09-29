import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateMintRateDistribution(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000290600734b9f100000ea6000007530', 'hex');
    const tx = transport.send(0xe0, 0x25, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('463b5095528401379903e9eb63f8db202ec818d621d500f1018f96bef2da6617e9222e5900eb3029cac52360969908b1e037e88d3db5b9f14b64f3f51a7a1a079000', 'hex'),
    );
}

test('[NANO S] Update mint rate distribution', setupZemu('nanos', async (sim, transport) => {
    await updateMintRateDistribution(sim, transport);
}));

test('[NANO X] Update mint rate distribution', setupZemu('nanox', async (sim, transport) => {
    await updateMintRateDistribution(sim, transport);
}));
