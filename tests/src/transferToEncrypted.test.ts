import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function transferToEncrypted(sim: Zemu, transport: Transport, handleUi: () => Promise<void>) {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da711f0000000000f4240', 'hex');
    const tx = transport.send(0xe0, 0x11, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();

    await expect(tx).resolves.toEqual(
        Buffer.from('8e8d02df2f029faaf07150c988043a3d1f0901196a596e4e93b3765cf491b97c7d5ad1cedc5a523ea7b5c02e32542b0f401e64390f65b3146150ab2773bb37009000', 'hex'),
    );
}

test('[NANO S] Transfer to encrypted', setupZemu('nanos', async (sim, transport) => {
    await transferToEncrypted(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_transfer_to_encrypted', [8, 0]);
    });
}));

test('[NANO X] Transfer to encrypted', setupZemu('nanox', async (sim, transport) => {
    await transferToEncrypted(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanox_transfer_to_encrypted', [4, 0]);
    });
}));
