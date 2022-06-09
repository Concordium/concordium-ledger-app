import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

async function bakerRestakeShared(sim: Zemu, transport: Transport, handleUi: () => Promise<void>) {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70701', 'hex');
    const tx = transport.send(0xe0, 0x16, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();
    await expect(tx).resolves.toEqual(
        Buffer.from('fe8b2a9173c32d1d1536c067b7ca5773242d6454f6d4a97aaac185a6fe0dbdb7a84ca9027f3fba9731807b7e2c4c65567f536b52c8ba8c1df800f2467fb6b1089000', 'hex'),
    );
}

test('[NANO S] Baker restake earnings', setupZemu('nanos', async (sim, transport) => {
    await bakerRestakeShared(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_baker_restake_earnings', [7]);
        await sim.clickBoth(undefined, false);
    });
}));

async function bakerRestakeXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await bakerRestakeShared(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_baker_restake_earnings`, [4]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Baker restake earnings', setupZemu('nanosp', bakerRestakeXAndSP));

test('[NANO X] Baker restake earnings', setupZemu('nanox', bakerRestakeXAndSP));
