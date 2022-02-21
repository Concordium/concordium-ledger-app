import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import chunkBuffer from './helpers';
import { setupZemu } from './options';

const header = "08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da719";
const capital = "0000FFFFFFFFFFFF";
const signVerifyKey = "7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b";
const signVerifyKeyProof = "a47cdf9133572e9ad5c02c3a7ffd1d05db7bb98860d918092454146153d62788f224c0157c65853ed4a0245ab3e0a593a3f85fa81cc4cb99eeaa643bfc793eab";
const electionVerifyKey = "32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828";
const electionVerifyKeyProof = "01fc695a8c51d4599cbe032a39832ad49bab900d88105b01d025b760b0d0d555b8c828f2d8fe29cc78c6307d979e6358b8bba9cf4d8200f272cc85b2a3813eff";
const aggregationVerifyKey = "7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082832f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828";
const aggregationVerifyKeyProof = "957aec4b2b7ed979ba2079d62246d135aefd61e7f46690c452fec8bcbb593481e229f6f1968194a09cf612490887e71d96730e2d852201e53fec9c89d36f8a90";
const fullP1 = capital + "01" + "02" + signVerifyKey + signVerifyKeyProof + electionVerifyKey + electionVerifyKeyProof;
const url = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

function encodeWord16(value: number): Buffer {
    const arr = new ArrayBuffer(2); // an Int16 takes 2 bytes
    const view = new DataView(arr);
    view.setUint16(0, value, false); // byteOffset = 0; litteEndian = false
    return Buffer.from(new Uint8Array(arr));
}

function configureBakerStep0(bitmap: string, transport: Transport) {
    const data = Buffer.from(header + bitmap, 'hex');
    return transport.send(0xe0, 0x18, 0x00, 0x00, data);
}

async function configureBakerStep1(transaction: string, aggregationKey: string | undefined, sim: Zemu, transport: Transport, handleUi: () => Promise<void>) {
    const data = Buffer.from(transaction, "hex");
    let tx = transport.send(0xe0, 0x18, 0x01, 0x00, data);
    if (aggregationKey) {
        await tx;
        const aggregationData = Buffer.from(aggregationKey, "hex");
        tx = transport.send(0xe0, 0x18, 0x02, 0x00, aggregationData);
    }
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();
    return tx;
}

async function configureBakerUrlStep(url: string, sim: Zemu, transport: Transport, handleUi: (index: number) => Promise<void>) {
    const serializedUrl = Buffer.from(url, 'utf-8');
    const serializedUrlLength = encodeWord16(serializedUrl.length);
    let previousSnapshot = await sim.snapshot();
    await transport.send(0xe0, 0x18, 0x03, 0x00, serializedUrlLength);

    // Batch the URL into at most 255 byte batches.
    const chunkedUrl = chunkBuffer(serializedUrl, 255);
    let tx;
    let i = 0;
    for (const serializedUrlChunk of chunkedUrl) {
        tx = transport.send(0xe0, 0x18, 0x04, 0x00, serializedUrlChunk);
        await handleUi(i);
        i += 1;
    }
    if (!tx) {
        // Should have entered the loop once and initialized the variable;
        throw new Error();
    }
    return tx;
}

async function configureBakerCommissionStep(transactionFee: boolean, bakingReward: boolean, finalizationReward: boolean, stepsThroughHeader: boolean, navigationDir: string, signature: string, sim: Zemu, transport: Transport) {
    let serializedCommissionRates = Buffer.alloc(0);
    let navigationSteps = stepsThroughHeader ? 6 : 0;

    if (transactionFee) {
        serializedCommissionRates = Buffer.from("F010B001", "hex")
        navigationSteps += 1;
    }

    if (bakingReward) {
        const bakingRewardCommission = Buffer.from("0000C001", "hex");
        serializedCommissionRates = Buffer.concat([serializedCommissionRates, bakingRewardCommission]);
        navigationSteps += 1;
    }

    if (finalizationReward) {
        const finalizationRewardCommission = Buffer.from("00000B11", "hex");
        serializedCommissionRates = Buffer.concat([serializedCommissionRates, finalizationRewardCommission]);
        navigationSteps += 1;
    }

    let tx = transport.send(0xe0, 0x18, 0x05, 0x00, serializedCommissionRates);
    await sim.waitScreenChange();
    await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/' + navigationDir, [navigationSteps, 0]);

    await expect(tx).resolves.toEqual(
        Buffer.from(signature, "hex")
    );
}

test('[NANO S] Configure-baker: Configure baker (none)', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0000';
    configureBakerStep0(bitmap, transport).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO S] Configure-baker: Fail if trying to update incomplete set of keys', setupZemu('nanos', async (sim, transport) => { 
    const bitmap = '0008';
    configureBakerStep0(bitmap, transport).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO S] Configure-baker: Capital, restake, open status and keys', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '003f';
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerStep1(fullP1, aggregationVerifyKey + aggregationVerifyKeyProof, sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/capital_restake_openstatus_keys', [9, 0]);
    })).resolves.toEqual(Buffer.from("5a9e9347eaef0997ef26c3de16a0f3ed433830c789fa9a7e6ef722a72990cd317136c6ae0aa42456ca561424563fc1dce4f01f1bdd86ab3756fb44537bf4a7029000", "hex"));
}));

test('[NANO S] Configure-baker: Capital, restake, open status, without keys', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0007';
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerStep1(capital + "01" + "02", undefined, sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/capital_restake_openstatus', [9, 0]);
    })).resolves.toEqual(Buffer.from("15102d4e6361a26fc03ae7866987f6253f20f3763602aa009b0f8318c0029c323b26c2f7c9a7fdd08c67719565cb680986e64d40faffdbae4ea876f4c6576d049000", "hex"));
}));

test('[NANO S] Configure-baker: only keys', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0038';
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerStep1(signVerifyKey + signVerifyKeyProof + electionVerifyKey + electionVerifyKeyProof, aggregationVerifyKey + aggregationVerifyKeyProof, sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/keys_only', [6, 0]);
    })).resolves.toEqual(Buffer.from("1e80af7f60f6d7a98c9a27068b46ba26fc5a524193b9c9dc2d4634436537a5c0d71ab68f9507380994c73f5a1b4f01f6fff235c051aaa27da60bce26f5ec22069000", "hex"));
}));

test('[NANO S] Configure-baker: URL only', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0040';
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerUrlStep(url, sim, transport, async (i: number) => {
        await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/url', [20, 0]);
    })).resolves.toEqual(Buffer.from("19dcff841796ee7b0fca422bf6fc0f6eb5d91a9185d065463541dc2b494cf9fbcdcc8ecb88a9f023b5eb9c1c28b549ff97ef4684d689494f93fb8ec93ca9740d9000", "hex"));
}));

test('[NANO S] Configure-baker: big URL only', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0040';
    const chunkCount = 3;
    const bigUrl = url.repeat(chunkCount);
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerUrlStep(bigUrl, sim, transport, async (i: number) => {
        await sim.waitScreenChange();
        if (i == 0) {
            await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/big_url_init', [20, 0]);
        } else if (i == chunkCount - 1)  {
            await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/big_url', [12]);
            await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/big_url_finish', [2, 0]);
        } else {
            await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/big_url', [14, 0]);
        }
    })).resolves.toEqual(Buffer.from("eca605c0cdbd851d978cd714d915419dd1f60ac3d8995a98189b9a14acfea7edac27261ac690e1d38c1325c9fd419565734a00bf2c8deceba5aa7c778a7b59069000", "hex"));
}));

test('[NANO S] Configure-baker: Commission rates only', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0380';
    await configureBakerStep0(bitmap, transport);
    await configureBakerCommissionStep(true, true, true, true, "commission_rates", "c6b8b327f944c2098b1bdc61f3ba54935d9ee17a0a8229683632577286a33bd38c686f67ee812441dd7e32b748fe8f62dd65df563892ab7c46a654ad50ffc20b9000", sim, transport);
}));

test('[NANO S] Configure-baker: Single commission rate', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0100';
    await configureBakerStep0(bitmap, transport);
    await configureBakerCommissionStep(false, true, false, true, "single_commission_rate" ,"42a39dcd27d8c7c9b147b6962b81437a66b9b21bb6a8c44332fcde01aca5bb50fce5ae796e4c2b952ad54442f97d12bde31612d4892eda4f47ff6e91470fb6079000", sim, transport);
}));

test('[NANO S] Configure-baker: All parameters', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '03ff';
    await configureBakerStep0(bitmap, transport);
    await configureBakerStep1(fullP1, aggregationVerifyKey + aggregationVerifyKeyProof, sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/all_parameters_1', [9, 0]);
    });
    await configureBakerUrlStep(url, sim, transport, async () => {
        await sim.waitScreenChange();
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/all_parameters_url', [14, 0]);
    });
    await configureBakerCommissionStep(true, true, true, false, "all_parameters_commision","b2a2fe3eda423c5781fe08d4eee49454c478f6c359f5c60325f06c8a1ab7803dd2158ea8a9d9fd97d2ae90fc615a1c5047d05260f7d1cb9ca6e3a240631023039000", sim, transport);
}));
