import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function timeoutParameters(sim: Zemu, transport: Transport, images: string) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000291200000f0000000001000000000000000100000000000000f2000000000000c001000000000c0000f2', 'hex');
    const tx = transport.send(0xe0, 0x43, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', images, [4, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from('3f9ec48f43503b0e6db3b91d0e69124ac7e57d99433e6ac5767cb20e4f76231eb1fa73c07f4b2208966986892398e3c58e077f1c6d813e4986d1e4424532df0c9000', 'hex'),
    );
}

test('[NANO S] Timeout parameters', setupZemu('nanos', async (sim, transport) => {
    await timeoutParameters(sim, transport, 'nanos_timeout_parameters');
}));

test('[NANO SP] Timeout parameters', setupZemu('nanosp', async (sim, transport) => {
    await timeoutParameters(sim, transport, 'nanosp_timeout_parameters');
}));
