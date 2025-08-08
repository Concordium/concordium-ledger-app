import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function cooldownParameters(sim: Zemu, transport: Transport, images: string) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029' + '0E' + '000075300000EA60' + '0000000000001000', 'hex');
    const tx = transport.send(0xe0, 0x40, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', images, [4]);
    await sim.clickBoth(undefined, false);
    await expect(tx).resolves.toEqual(
        Buffer.from('5353d3e06fc910caf404e4adb40028ca159c787c558f860d41590afda94e1f866231fad63f8d73eb65b1ab1bc4b9618b0bc681860f377ad329c4c2a36a6136069000', 'hex'),
    );
}

test('[NANO S] Cooldown parameters', setupZemu('nanos', async (sim, transport) => {
    await cooldownParameters(sim, transport, 'nanos_cooldown_parameters');
}));

test('[NANO SP] Cooldown parameters', setupZemu('nanosp', async (sim, transport) => {
    await cooldownParameters(sim, transport, 'nanosp_cooldown_parameters');
}));
