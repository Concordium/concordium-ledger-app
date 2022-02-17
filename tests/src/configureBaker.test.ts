import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import chunkBuffer from './helpers';
import { setupZemu } from './options';

function encodeWord16(value: number): Buffer {
    const arr = new ArrayBuffer(2); // an Int16 takes 2 bytes
    const view = new DataView(arr);
    view.setUint16(0, value, false); // byteOffset = 0; litteEndian = false
    return Buffer.from(new Uint8Array(arr));
}

async function configureBakerStep1(transaction: string, signature: string, sim: Zemu, transport: Transport, handleUi: () => Promise<void>) {
    const data = Buffer.from(transaction, "hex");
    const tx = transport.send(0xe0, 0x18, 0x01, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();
    await expect(tx).resolves.toEqual(
        Buffer.from(signature, "hex")
    );
}

async function configureBakerUrlStep(signature: string, sim: Zemu, transport: Transport) {
    const url = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    const serializedUrl = Buffer.from(url, 'utf-8');
    const serializedUrlLength = encodeWord16(serializedUrl.length);
    await transport.send(0xe0, 0x18, 0x02, 0x00, serializedUrlLength);

    // Batch the URL into at most 255 byte batches.
    const chunkedUrl = chunkBuffer(serializedUrl, 255);
    let tx;
    for (const serializedUrlChunk of chunkedUrl) {
        tx = transport.send(0xe0, 0x18, 0x03, 0x00, serializedUrlChunk);
        await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/url', [20, 0]);
    }

    await expect(tx).resolves.toEqual(
        Buffer.from(signature, "hex")
    );
}

async function configureBakerCommissionStep(transactionFee: boolean, bakingReward: boolean, finalizationReward: boolean, navigationDir: string, signature: string, sim: Zemu, transport: Transport) {
    let serializedCommissionRates = Buffer.alloc(0);
    let navigationSteps = 6;

    if (transactionFee) {
        const transactionFeeCommissionNumerator = Buffer.from("0000000000000001", "hex");
        const transactionFeeCommissionDenominator = Buffer.from("00000000F010B001", "hex");
        serializedCommissionRates = Buffer.concat([transactionFeeCommissionNumerator, transactionFeeCommissionDenominator]);
        navigationSteps += 1;   
    }

    if (bakingReward) {
        const bakingRewardCommissionNumerator = Buffer.from("000000000000C001", "hex");
        const bakingRewardCommissionDenominator = Buffer.from("0000000000000201", "hex");
        serializedCommissionRates = Buffer.concat([serializedCommissionRates, bakingRewardCommissionNumerator, bakingRewardCommissionDenominator]);
        navigationSteps += 1;
    }

    if (finalizationReward) {
        const finalizationRewardCommissionNumerator = Buffer.from("0000000000000B11", "hex");
        const finalizationRewardCommissionDenominator = Buffer.from("0000000000000F01", "hex");
        serializedCommissionRates = Buffer.concat([serializedCommissionRates, finalizationRewardCommissionNumerator, finalizationRewardCommissionDenominator]);
        navigationSteps += 1;
    }

    let tx = transport.send(0xe0, 0x18, 0x04, 0x00, serializedCommissionRates);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/' + navigationDir, [navigationSteps, 0]);
    
    await expect(tx).resolves.toEqual(
        Buffer.from(signature, "hex")
    );
}

test('[NANO S] Configure baker (none)', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da7190000', 'hex');
    transport.send(0xe0, 0x18, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO S] Fail if trying to update incomplete set of keys', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da7190008', 'hex');
    transport.send(0xe0, 0x18, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO S] Capital, restake, open status and keys', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da719003f', 'hex');
    await transport.send(0xe0, 0x18, 0x00, 0x00, data);
    await configureBakerStep1("0000ffffffffffff01027873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c08287873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082832f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828", "f4beb2afde669167b8943dd6add6837539713927114bd19b8d7140e49c0f800161bdc5cccc0ee87d5cd506b8fbb27bcf764898a43f221640ac1b5b25cf4ef60a9000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/capital_restake_openstatus_keys', [9, 0]);
    });
}));

test('[NANO S] Capital, restake, open status, without keys', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da719003f', 'hex');
    await transport.send(0xe0, 0x18, 0x00, 0x00, data);
    await configureBakerStep1("0000ffffffffffff0102", "03fba425c40e4b3d0e2362ab3378aae078aa291b4297a7e0d7235ec5f7ae3c187a775974f1bf856e257f68ed8e7674275ec65db9175855249a8912d45ff15d0a9000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/capital_restake_openstatus', [9, 0]);
    });
}));

test('[NANO S] URL only', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da7190040', 'hex');
    await transport.send(0xe0, 0x18, 0x00, 0x00, data);
    await configureBakerUrlStep("19dcff841796ee7b0fca422bf6fc0f6eb5d91a9185d065463541dc2b494cf9fbcdcc8ecb88a9f023b5eb9c1c28b549ff97ef4684d689494f93fb8ec93ca9740d9000", sim, transport);
}));

test('[NANO S] Commission rates only', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da7190380', 'hex');
    await transport.send(0xe0, 0x18, 0x00, 0x00, data);
    await configureBakerCommissionStep(true, true, true, "commission_rates", "33418b5501f85a4e5d1820b2eb7f72ddbd65fcbf81e7c342ec7351df1ea2c7f95119c955dacc4e985d439ea03065954c4fdf24837eb06c7057be22c1d9248c009000", sim, transport);
}));

test('[NANO S] Single commission rate', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da7190100', 'hex');
    await transport.send(0xe0, 0x18, 0x00, 0x00, data);
    await configureBakerCommissionStep(false, true, false, "single_commission_rate" ,"eca675c1f30619e15c33f6b86c26d786588edc61a761d91efac13eb74fceb69268d3b4da04d1d8c3332ee30a400ee253321db4d57a63c08e32fba00f9f1ff9079000", sim, transport);
}));

// TODO: Add a test with onl keys.