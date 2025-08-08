import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateGasRewards(sim: Zemu, transport: Transport, images: string) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002915000061a8000000c800000190', 'hex');
    const tx = transport.send(0xe0, 0x23, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    // Extra click for update type screen
    await sim.navigateAndCompareSnapshots('.', images, [5, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from('6de9e500bdd742b37923055f4e22c00124692395fa39ac5f178a53fc2ab66a3c7aef896d6b7e31a1a133f656fbc35a7e0cea365dbc120a80667c0ee4e7dd7d059000', 'hex'),
    );
}

test('[NANO S] Update GAS rewards', setupZemu('nanos', async (sim, transport) => {
    await updateGasRewards(sim, transport, 'nanos_update_gas_rewards');
}));

test('[NANO SP] Update GAS rewards', setupZemu('nanosp', async (sim, transport) => {
    await updateGasRewards(sim, transport, 'nanosp_update_gas_rewards');
}));
