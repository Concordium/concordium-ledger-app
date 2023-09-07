import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateEuroPerEnergy(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002903000000000000000100000000000000f2', 'hex');
    const tx = transport.send(0xe0, 0x06, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('fb3b5c56421c488ccf8d77b7d384856cbfb6b15095628cf1758d08c33b8ec63d222b0c2ea5394a84512b872ffb7cd5922e8addd1aa8a218283ad73e34cf463049000', 'hex'),
    );
}

test('[NANO S] Update euro per energy', setupZemu('nanos', async (sim, transport) => {
    await updateEuroPerEnergy(sim, transport);
}));
