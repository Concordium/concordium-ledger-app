import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

const registerDataTest = (picture: string, steps: number) => (async (sim: Zemu, transport: Transport) => {
    let data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da7150005', 'hex');
    transport.send(0xe0, 0x35, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', `${picture}_0`, [steps]);
    await sim.clickBoth(undefined, false);
    const snapshot1 = await sim.snapshot();
    data = Buffer.from('6474657374', 'hex');
    const tx = transport.send(0xe0, 0x35, 0x01, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot1);
    await sim.navigateAndCompareSnapshots('.', `${picture}_1`, [0]);
    await sim.clickBoth(undefined, false);
    await expect(tx).resolves.toEqual(
        Buffer.from('cd4031eed073a6a4e3c883ab95dd5ad281de88bc17713c540852048522e7cd61c3c6ba06fb07d22447b073fec3c52dc0f847317745838ccbd83930bfe00809099000', 'hex'),
    );
});

test('[NANO S] Sign a register data transaction', setupZemu('nanos', registerDataTest('nanos_register_data', 6)));
test('[NANO SP] Sign a register data transaction', setupZemu('nanosp', registerDataTest('nanosp_register_data', 3)));
test('[NANO X] Sign a register data transaction', setupZemu('nanox', registerDataTest('nanox_register_data', 3)));

const registerDataTestTooLargeDataLength = (async (sim: Zemu, transport: Transport) => {
    expect.assertions(1);
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da715045A', 'hex');
    transport.send(0xe0, 0x35, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27395));
});

const registerDataTestTooMuchData = (steps: number) => (async (sim: Zemu, transport: Transport) => {
    expect.assertions(1);
    let data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da7150005', 'hex');
    transport.send(0xe0, 0x35, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    for (let i = 0; i < steps; i += 1) {
        await sim.clickRight();
    }
    await sim.clickBoth(undefined, false);
    data = Buffer.from('647465737410', 'hex');
    transport.send(0xe0, 0x35, 0x01, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
});

test('[NANO S] Attempt to sign a register data transaction with dataLength larger than is allowed', setupZemu('nanos', registerDataTestTooLargeDataLength));
test('[NANO SP] Attempt to sign a register data transaction with dataLength larger than is allowed', setupZemu('nanosp', registerDataTestTooLargeDataLength));
test('[NANO X] Attempt to sign a register data transaction with dataLength larger than is allowed', setupZemu('nanox', registerDataTestTooLargeDataLength));
test('[NANO S] Attempt to sign a register data transaction with more data than dataLength specifies', setupZemu('nanos', registerDataTestTooMuchData(6)));
test('[NANO SP] Attempt to sign a register data transaction with more data than dataLength specifies', setupZemu('nanosp', registerDataTestTooMuchData(3)));
test('[NANO X] Attempt to sign a register data transaction with more data than dataLength specifies', setupZemu('nanox', registerDataTestTooMuchData(3)));
