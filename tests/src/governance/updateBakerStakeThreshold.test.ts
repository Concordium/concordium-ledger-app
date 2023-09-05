import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateBakerStakeThreshold(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000290900000a00c60d5000', 'hex');
    const tx = transport.send(0xe0, 0x27, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('8d1a8190b4e9b5db118c15db45af9bc0441910775030582a13ed8f98493032881e3a5fef3873300875b140156ff103a052877821589db4695e0a6dda9313c10b9000', 'hex'),
    );
}

test('[NANO S] Update baker stake threshold', setupZemu('nanos', async (sim, transport) => {
    await updateBakerStakeThreshold(sim, transport);
}));

test('[NANO X] Update baker stake threshold', setupZemu('nanox', async (sim, transport) => {
    await updateBakerStakeThreshold(sim, transport);
}));
