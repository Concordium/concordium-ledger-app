import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function finalizationCommitteeParameters(sim: Zemu, transport: Transport, images: string) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002916000000030000000500008005', 'hex');
    const tx = transport.send(0xe0, 0x46, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', images, [4, 0]);
    await expect(tx).resolves.toEqual(
        Buffer.from('054c7007038b0c7c208cc5bcbb79e80cb6afc681ebfc54bcda53fd91b33110c71255cefc458ab7521fa6ea15dedc1b2c7ef2090b2036a96eff086f9be6a223089000', 'hex'),
    );
}

test('[NANO S] Finalization committee parameters', setupZemu('nanos', async (sim, transport) => {
    await finalizationCommitteeParameters(sim, transport, 'nanos_finalization_committee_parameters');
}));
