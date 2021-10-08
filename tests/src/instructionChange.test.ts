import Transport from '@ledgerhq/hw-transport';
import { setupZemu } from './options';

async function changeInstructionTest(transport: Transport) {
    let data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da712', 'hex');
    await transport.send(0xe0, 0x12, 0x00, 0x00, data);

    data = Buffer.from('97a9558421515a34f7c27c3b55c8dc561ed8c45a983387edf4f95ef834c5fd12d4fc5bb772490cb5653d249926bd9fc9938768d3921a4aa2361ae620e294637beb2d052cb0351745d81e2b34fee677c1e37cd8fedd1e4afbd66557d08f827db6803e57a7473ee78c7ba2db84c6910d355f497d257f89588a5e7176265b890841add54085462b35c3d0a01d562a29023db80fbac86061bf8970ceb5cf0889cc762e0cc6720ffac0b932c6168038aaec96e824c68cdb95e8cea823fa32792f43600000c0a0000f4240000c0a00000000010a9f', 'hex');
    await transport.send(0xe0, 0x12, 0x01, 0x00, data);

    data = Buffer.from('97a9558421515a34f7c27c3b55c8dc561ed8c45a983387edf4f95ef834c5fd12d4fc5bb772490cb5653d249926bd9fc9938768d3921a4aa2361ae620e294637beb2d052cb0351745d81e2b34fee677c1e37cd8fedd1e4afbd66557d08f827db6803e57a7473ee78c7ba2db84c6910d355f497d257f89588a5e7176265b890841add54085462b35c3d0a01d562a29023db80fbac86061bf8970ceb5cf0889cc762e0cc6720ffac0b932c6168038aaec96e824c68cdb95e8cea823fa32792f4360', 'hex');
    const tx = transport.send(0xe0, 0x10, 0x01, 0x00, data);

    await expect(tx).rejects.toThrow();
}

test('[NANO S] Changing instruction fails', setupZemu('nanos', async (sim, transport) => {
    await changeInstructionTest(transport);
}));

test('[NANO X] Changing instruction fails', setupZemu('nanox', async (sim, transport) => {
    await changeInstructionTest(transport);
}));
