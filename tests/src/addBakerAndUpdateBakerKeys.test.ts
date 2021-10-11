import Transport from '@ledgerhq/hw-transport';
import { setupZemu } from './options';

async function addBakerShared(transport: Transport) {
    let data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da704', 'hex');
    await transport.send(0xe0, 0x13, 0x00, 0x00, data);

    data = Buffer.from('32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c08287873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082832f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828', 'hex');
    await transport.send(0xe0, 0x13, 0x01, 0x00, data);

    data = Buffer.from('a47cdf9133572e9ad5c02c3a7ffd1d05db7bb98860d918092454146153d62788f224c0157c65853ed4a0245ab3e0a593a3f85fa81cc4cb99eeaa643bfc793eab01fc695a8c51d4599cbe032a39832ad49bab900d88105b01d025b760b0d0d555b8c828f2d8fe29cc78c6307d979e6358b8bba9cf4d8200f272cc85b2a3813eff957aec4b2b7ed979ba2079d62246d135aefd61e7f46690c452fec8bcbb593481e229f6f1968194a09cf612490887e71d96730e2d852201e53fec9c89d36f8a9000000000000002e701', 'hex');
    return transport.send(0xe0, 0x13, 0x02, 0x00, data);
}

test('[NANO S] Add baker', setupZemu('nanos', async (sim, transport) => {
    const tx = addBakerShared(transport);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanos_add_baker', [8, 0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('8bde384ab620b1ba6714c1b78521ebfdcae8159cb86e2a6c94964dfd000e21e085c98f2bdef55292bdaf1d8e6dd3e5277c5dc83a8089fca634ffb3713ba9150b9000', 'hex'),
    );
}));

test('[NANO X] Add baker', setupZemu('nanox', async (sim, transport) => {
    const tx = addBakerShared(transport);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanox_add_baker', [5, 0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('8bde384ab620b1ba6714c1b78521ebfdcae8159cb86e2a6c94964dfd000e21e085c98f2bdef55292bdaf1d8e6dd3e5277c5dc83a8089fca634ffb3713ba9150b9000', 'hex'),
    );
}));

async function updateBakerKeysShared(transport: Transport) {
    let data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da708', 'hex');
    await transport.send(0xe0, 0x13, 0x00, 0x01, data);

    data = Buffer.from('32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c08287873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082832f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828', 'hex');
    await transport.send(0xe0, 0x13, 0x01, 0x01, data);

    data = Buffer.from('a47cdf9133572e9ad5c02c3a7ffd1d05db7bb98860d918092454146153d62788f224c0157c65853ed4a0245ab3e0a593a3f85fa81cc4cb99eeaa643bfc793eab01fc695a8c51d4599cbe032a39832ad49bab900d88105b01d025b760b0d0d555b8c828f2d8fe29cc78c6307d979e6358b8bba9cf4d8200f272cc85b2a3813eff957aec4b2b7ed979ba2079d62246d135aefd61e7f46690c452fec8bcbb593481e229f6f1968194a09cf612490887e71d96730e2d852201e53fec9c89d36f8a90', 'hex');
    return transport.send(0xe0, 0x13, 0x02, 0x01, data);
}

test('[NANO S] Update baker keys', setupZemu('nanos', async (sim, transport) => {
    const tx = updateBakerKeysShared(transport);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanos_update_baker_keys', [7, 0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('c1347c15432f5277533d999e2b8a847b21b3c55a1ec0f2415273ae4d90cca9e9ac5950ef47483ee5423739cb1d90989bb50472544e65495b5cd8c3ddc85fa2019000', 'hex'),
    );
}));

test('[NANO X] Update baker keys', setupZemu('nanox', async (sim, transport) => {
    const tx = updateBakerKeysShared(transport);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanox_update_baker_keys', [4, 0]);

    await expect(tx).resolves.toEqual(
        Buffer.from('c1347c15432f5277533d999e2b8a847b21b3c55a1ec0f2415273ae4d90cca9e9ac5950ef47483ee5423739cb1d90989bb50472544e65495b5cd8c3ddc85fa2019000', 'hex'),
    );
}));
