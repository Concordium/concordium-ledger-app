import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { Model } from './helpers';
import { setupZemu } from './options';

async function updateBakerStake(sim: Zemu, transport: Transport, handleUi: () => Promise<void>) {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da7060000000000001a9b', 'hex');
    const tx = transport.send(0xe0, 0x15, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();
    await expect(tx).resolves.toEqual(
        Buffer.from('f3f3703046a3762c205315884f6f3bb416785c795d7725a44c59fced3439768603481965719f766f03b9c3851494d5e17e010cbd6cdaac643ecd636c3b49d1049000', 'hex'),
    );
}

test('[NANO S] Update baker stake', setupZemu('nanos', async (sim, transport) => {
    await updateBakerStake(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_baker_update_stake', [7, 0]);
    });
}));

async function updateBakerStakeXAndSP(sim: Zemu, transport: Transport, device: Model) {
    await updateBakerStake(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', device + '_baker_update_stake', [4, 0]);
    });
}

test('[NANO SP] Update baker stake', setupZemu('nanosp', async (sim, transport) => {
    await updateBakerStakeXAndSP(sim, transport, 'nanosp');
}));

test('[NANO X] Update baker stake', setupZemu('nanox', async (sim, transport) => {
    await updateBakerStakeXAndSP(sim, transport, 'nanox');
}));
