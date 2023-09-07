import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateTransactionDistribution(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029070000afc8000088b8', 'hex');
    const tx = transport.send(0xe0, 0x22, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('e7cc965144f0f5b3be84c7f5a1da0188058938772ad9a9c2a62a0d2c50d677deaa4c1aa747cfb7f7379f71ebb29a33a4a5cabcc765fe94153975462e2e759e009000', 'hex'),
    );
}

test('[NANO S] Update transaction distribution', setupZemu('nanos', async (sim, transport) => {
    await updateTransactionDistribution(sim, transport);
}));
