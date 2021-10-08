import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateGasRewards(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002908000061a8000001f4000000c800000190', 'hex');
    const tx = transport.send(0xe0, 0x23, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('7bdaf94567fa4233bbbe65d0cd64717266c301471edfac6629a7ab195e6d62b23fcc08f65a265900e589152a635b9d45d3a86211a9faf131b21d72009b8b06049000', 'hex'),
    );
}

test('[NANO S] Update GAS rewards', setupZemu('nanos', async (sim, transport) => {
    await updateGasRewards(sim, transport);
}));

test('[NANO X] Update GAS rewards', setupZemu('nanox', async (sim, transport) => {
    await updateGasRewards(sim, transport);
}));
