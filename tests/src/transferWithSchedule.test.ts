import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { ISnapshot } from '@zondax/zemu/dist/types';
import { LedgerModel, setupZemu } from './options';

async function transferWithSchedule(sim: Zemu, transport: Transport, handleUi: () => Promise<ISnapshot>) {
    let data = Buffer.from(
        '08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d705',
        'hex'
    );
    transport.send(0xe0, 0x03, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    let snapshot = await handleUi();

    // Schedule points
    data = Buffer.from(
        '0000017a396883d90000000005f5e1000000017a396883d90000000005f5e1000000017a396883d90000000005f5e1000000017a396883d90000000005f5e1000000017a396883d90000000005f5e100',
        'hex'
    );
    const tx = transport.send(0xe0, 0x03, 0x01, 0x00, data);
    for (let i = 0; i < 5; i += 1) {
        await sim.waitUntilScreenIsNot(snapshot);
        await sim.clickRight();
        snapshot = await sim.clickRight();
        await sim.clickBoth();
    }

    await expect(tx).resolves.toEqual(
        Buffer.from(
            '6bbf7bf1f5ece1b1f2e8942fa49fd8932691d542ca1120fc5d50400b0532dc5c222e2083c9946ac5267adeb67573f9e1027abf63f7bc76ffe4cc7a31c8f093019000',
            'hex'
        )
    );
}

test(
    '[NANO S] Transfer with schedule',
    setupZemu('nanos', async (sim, transport) => {
        await transferWithSchedule(sim, transport, async () => {
            await sim.navigateAndCompareSnapshots('.', 'nanos_transfer_with_schedule', [11]);
            await sim.clickBoth(undefined, false);
            return sim.snapshot();
        });
    })
);

async function transferWithScheduleXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await transferWithSchedule(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_transfer_with_schedule`, [5]);
        await sim.clickBoth(undefined, false);
        return sim.snapshot();
    });
}

test('[NANO SP] Transfer with schedule', setupZemu('nanosp', transferWithScheduleXAndSP));

test('[NANO X] Transfer with schedule', setupZemu('nanox', transferWithScheduleXAndSP));

async function transferWithScheduleWithMemo(sim: Zemu, transport: Transport, handleUi: () => Promise<ISnapshot>) {
    let data = Buffer.from(
        '08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71820a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7050005',
        'hex'
    );
    await transport.send(0xe0, 0x34, 0x02, 0x00, data);

    data = Buffer.from('6474657374', 'hex');
    transport.send(0xe0, 0x34, 0x03, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    let snapshot = await handleUi();

    // Schedule points
    data = Buffer.from(
        '0000017a396883d90000000005f5e1000000017a396883d90000000005f5e1000000017a396883d90000000005f5e1000000017a396883d90000000005f5e1000000017a396883d90000000005f5e100',
        'hex'
    );
    const tx = transport.send(0xe0, 0x34, 0x01, 0x00, data);
    for (let i = 0; i < 5; i += 1) {
        await sim.waitUntilScreenIsNot(snapshot);
        await sim.clickRight();
        snapshot = await sim.clickRight();
        await sim.clickBoth(undefined, false);
    }

    await expect(tx).resolves.toEqual(
        Buffer.from(
            'ddc951a303e20273ee96add8b0599b66d423604a6dc1e97e68868aabcc2a143798792051a44be84c54884b389ca57b0c7410c7b756eb8f6a14c0caded5802c019000',
            'hex'
        )
    );
}

test(
    '[NANO S] Transfer with schedule and memo',
    setupZemu('nanos', async (sim, transport) => {
        await transferWithScheduleWithMemo(sim, transport, async () => {
            await sim.navigateAndCompareSnapshots('.', 'nanos_transfer_with_schedule_and_memo', [12]);
            await sim.clickBoth(undefined, false);
            return sim.snapshot();
        });
    })
);

async function transferWithScheduleWithMemoXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await transferWithScheduleWithMemo(sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_transfer_with_schedule_and_memo`, [6]);
        await sim.clickBoth(undefined, false);
        return sim.snapshot();
    });
}

test('[NANO SP] Transfer with schedule and memo', setupZemu('nanosp', transferWithScheduleWithMemoXAndSP));

test('[NANO X] Transfer with schedule and memo', setupZemu('nanox', transferWithScheduleWithMemoXAndSP));
