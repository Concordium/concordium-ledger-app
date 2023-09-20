import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateGasRewards(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002915000061a8000000c800000190', 'hex');
    const tx = transport.send(0xe0, 0x23, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('e4e0bd1d1f565362172004c0ff45a7c3468bee69f3344ff9d4b8bdf9b3538d0212103243014a6f28df0262aba42ec43c2e53dc1de3e5b0cab7588ab498a5a5019000', 'hex'),
    );
}

test('[NANO S] Update GAS rewards', setupZemu('nanos', async (sim, transport) => {
    await updateGasRewards(sim, transport);
}));

test('[NANO SP] Update GAS rewards', setupZemu('nanosp', async (sim, transport) => {
    await updateGasRewards(sim, transport);
}));
