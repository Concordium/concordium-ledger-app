import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

async function timeParameters(sim: Zemu, transport: Transport, device: LedgerModel) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029' + '10' + '000075300000EA60' + '00734B9F' + '09', 'hex');
    const tx = transport.send(0xe0, 0x42, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', `${device}_time_parameters`, [3]);
    await sim.clickBoth(undefined, false);
    await expect(tx).resolves.toEqual(
        Buffer.from('05b6a58557aa005b78ab21ea8ecf3741c5f8efd7ca1e950adf595044b4770ffc9f223e0d87825a268eb6406519110917456efda1bcd7be96de0d3e46d9e52d0e9000', 'hex'),
    );
}

test('[NANO S] Time parameters', setupZemu('nanos', timeParameters));

test('[NANO SP] Time parameters', setupZemu('nanosp', timeParameters));

test('[NANO X] Time parameters', setupZemu('nanox', timeParameters));
