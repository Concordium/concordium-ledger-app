import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateMintRateDistributionVersion0(sim: Zemu, transport: Transport) {
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

async function updateMintRateDistributionVersion1(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029060000ea6000007530', 'hex');
    const tx = transport.send(0xe0, 0x25, 0x00, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('0fc6a8ef9d0d841ed4c269a1ff51f3ff097e780b9b57788c5fdd6cf977dad136b4077c798c14e76a19b59cddb7fb9431a1476c70c5faabc8ca848f6c301ca3089000', 'hex'),
    );
}

test('[NANO S] Update mint rate distribution v0', setupZemu('nanos', async (sim, transport) => {
    await updateMintRateDistributionVersion0(sim, transport);
}));

test('[NANO X] Update mint rate distribution v0', setupZemu('nanox', async (sim, transport) => {
    await updateMintRateDistributionVersion0(sim, transport);
}));

test('[NANO S] Update mint rate distribution v1', setupZemu('nanos', async (sim, transport) => {
    await updateMintRateDistributionVersion1(sim, transport);
}));

test('[NANO X] Update mint rate distribution v1', setupZemu('nanox', async (sim, transport) => {
    await updateMintRateDistributionVersion1(sim, transport);
}));
