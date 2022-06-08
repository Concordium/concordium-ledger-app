import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateElectionDifficulty(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029020000f000', 'hex');
    const tx = transport.send(0xe0, 0x26, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('1f7d7fb674596b4699caec45ebd594277b6b5a5d2402e71e7260c9183a49a0d3616005d7c636e9aae47f1e2f804fa4db5189dccd0996f45c144c63ffe762280d9000', 'hex'),
    );
}

test('[NANO S] Update election difficulty', setupZemu('nanos', async (sim, transport) => {
    await updateElectionDifficulty(sim, transport);
}));

test('[NANO SP] Update election difficulty', setupZemu('nanosp', async (sim, transport) => {
    await updateElectionDifficulty(sim, transport);
}));

test('[NANO X] Update election difficulty', setupZemu('nanox', async (sim, transport) => {
    await updateElectionDifficulty(sim, transport);
}));
