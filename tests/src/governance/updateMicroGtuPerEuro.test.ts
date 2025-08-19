import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function updateMicroGtuPerEuro(sim: Zemu, transport: Transport, handleUi: () => Promise<void>) {
    const data = Buffer.from(
        '080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029040000000f000000010000000000000001',
        'hex'
    );
    const tx = transport.send(0xe0, 0x06, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();
    await expect(tx).resolves.toEqual(
        Buffer.from(
            '2f5133466ee50829f97f1062ed60708b0bf0307b1459a2e0c853d5180fae05d485a175756b918de13f0aebbb87bc8e7a9d2f84b3f27d3d443f4506fd9ffad70d9000',
            'hex'
        )
    );
}

test(
    '[NANO S] Update micro GTU per euro',
    setupZemu('nanos', async (sim, transport) => {
        await updateMicroGtuPerEuro(sim, transport, async () => {
            await sim.navigateAndCompareSnapshots('.', 'nanos_update_micro_ccd_per_euro', [2, 0]);
        });
    })
);

test(
    '[NANO SP] Update micro GTU per euro',
    setupZemu('nanosp', async (sim, transport) => {
        await updateMicroGtuPerEuro(sim, transport, async () => {
            await sim.navigateAndCompareSnapshots('.', 'nanosp_update_micro_ccd_per_euro', [2, 0]);
        });
    })
);
