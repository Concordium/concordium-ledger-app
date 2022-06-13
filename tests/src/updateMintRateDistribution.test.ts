import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateMintRateDistribution(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029110000ea6000007530', 'hex');
    const tx = transport.send(0xe0, 0x25, 0x00, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from('c11fb940dd3c7f514d3727fe5f50164fb5618b3df64ef43efe231835b7c779499e29df1bbdabd97976b045fcaebb25c762e8e0b2228db2f180e62bbeace936029000', 'hex'),
    );
}

test('[NANO S] Update mint rate distribution', setupZemu('nanos', updateMintRateDistribution));

test('[NANO X] Update mint rate distribution', setupZemu('nanosp', updateMintRateDistribution));

test('[NANO X] Update mint rate distribution', setupZemu('nanox', updateMintRateDistribution));
