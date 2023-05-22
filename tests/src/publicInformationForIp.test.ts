import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

async function identityProvider(_device: 'nanos' | 'nanox' | 'nanosp', sim: Zemu, transport: Transport, handleKeyUi: (key: string, extraStep: number) => Promise<any>) {
    let data = Buffer.from('0800000451000000000000000000000000000000000000000200000000000000008196e718f392ec8d07216b22b555cbb71bcee88037566d3f758b9786b945e3b614660f4bf954dbe57bc2304e5a863d2e89a1f69196a1d0423f4936aa664da95de16f40a639dba085073c5a7c8e710c2a402136cc89a39c12ed044e1035649c0f03', 'hex');
    await transport.send(0xe0, 0x20, 0x00, 0x00, data);
    data = Buffer.from('0000b6bc751f1abfb6440ff5cce27d7cdd1e7b0b8ec174f54de426890635b27e7daf', 'hex');
    transport.send(0xe0, 0x20, 0x01, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    const snapshot1 = await handleKeyUi('key1', 2);

    data = Buffer.from('000146a3e38ddf8b493be6e979034510b91db5448da9cba48c106139c288d658a004', 'hex');
    transport.send(0xe0, 0x20, 0x01, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot1);
    const snapshot2 = await handleKeyUi('key2', 1);

    data = Buffer.from('000271d5f16bc3be249043dc0f9e20b4872f5c3477bf2f285336609c5b0873ab3c9c', 'hex');
    await transport.send(0xe0, 0x20, 0x01, 0x00, data);
    data = Buffer.from('02');
    const tx = transport.send(0xe0, 0x20, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot2);
    await handleKeyUi('final', 2);

    await expect(tx).resolves.toEqual(
        Buffer.from('ff3da33b55c1d0763d9d4b823f91af9db75986a548a4bc031b81dee1c4f418a902245f8b0934e3f125163948b2a50bd3661b91dae17442613f0c34707a4789059000', 'hex'),
    );
}

function getKeyUiHandler(sim: Zemu, keySteps: number, device: LedgerModel) {
    return async (key: string, extraSteps: number) => {
        await sim.navigateAndCompareSnapshots('.', `${device}_public_info_for_ip/${key}`, [keySteps + extraSteps]);
        return sim.clickBoth(undefined, false);
    };
}

test('[NANO S] Public information for identity provider', setupZemu('nanos', async (sim, transport) => {
    await identityProvider('nanos', sim, transport, getKeyUiHandler(sim, 3, 'nanos'));
}));

async function identityProviderXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await identityProvider(device, sim, transport, getKeyUiHandler(sim, 1, device));
}

test('[NANO SP] Public information for identity provider', setupZemu('nanosp', identityProviderXAndSP));

test('[NANO X] Public information for identity provider', setupZemu('nanox', identityProviderXAndSP));
