import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

async function updateFoundationAccount(
    sim: Zemu, transport: Transport, handleUi: () => Promise<void>,
) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000290520a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7', 'hex');
    const tx = transport.send(0xe0, 0x24, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();
    await expect(tx).resolves.toEqual(
        Buffer.from('051a9c592d42a12fd5d373ccfe4a597d826942c2424e25060aff3877e56d03d4d48fc5595944cdc7b92db9110e3aaf10ed05f7983ab55de03a32c97c9d8a99099000', 'hex'),
    );
}

test('[NANO S] Update foundation account', setupZemu('nanos', async (sim, transport) => {
    await updateFoundationAccount(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_update_foundation_account', [6]);
        await sim.clickBoth(undefined, false);
    });
}));

async function updateFoundationAccountXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await updateFoundationAccount(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_update_foundation_account`, [3]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Update foundation account', setupZemu('nanosp', updateFoundationAccountXAndSP));

test('[NANO X] Update foundation account', setupZemu('nanox', updateFoundationAccountXAndSP));
