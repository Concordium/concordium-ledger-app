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
    const bitmap = '000f';
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerStep1(fullP1, aggregationVerifyKey + aggregationVerifyKeyProof, sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/capital_restake_openstatus_keys', [10, 0]);
    })).resolves.toEqual(Buffer.from("60aba821cb44103d68aab00be87990a886a8caab5fe10f403b77cde7b24fa527a92bc43fd061138b044c787d673d4f92851f0cb989286d65011ca3b5c5f908089000", "hex"));
}));

test('[NANO S] Configure-baker: Capital, restake, open status, without keys', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0007';
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerStep1(capital + "01" + "02", undefined, sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/capital_restake_openstatus', [9, 0]);
    })).resolves.toEqual(Buffer.from("15102d4e6361a26fc03ae7866987f6253f20f3763602aa009b0f8318c0029c323b26c2f7c9a7fdd08c67719565cb680986e64d40faffdbae4ea876f4c6576d049000", "hex"));
}));

test('[NANO S] Configure-baker: only keys', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0008';
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerStep1(signVerifyKey + signVerifyKeyProof + electionVerifyKey + electionVerifyKeyProof, aggregationVerifyKey + aggregationVerifyKeyProof, sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/keys_only', [7, 0]);
    })).resolves.toEqual(Buffer.from("fc41c691fea3e94dcfcf129d0ab29156111face70a70ed55c297cdb492ab5ddc6dfe0403b5b8fcbfff2d39f175ed35c434277422909bf6abe7427c23a11d6d0c9000", "hex"));
}));

test('[NANO S] Configure-baker: URL only', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0010';
    await configureBakerStep0(bitmap, transport);
    await expect(configureBakerUrlStep(url, sim, transport, async (i: number) => {
        await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/url', [20, 0]);
    })).resolves.toEqual(Buffer.from("83221ba7a2b53559ae2dd419915be2946b193aaff16ed7fe2ca98d55ed882643dc9505b0501799fff8253c75263a56aeb3c0f16b228a4ca33fe1c74845f3aa049000", "hex"));
}));

test('[NANO S] Configure-baker: big URL only', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0010';
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
    })).resolves.toEqual(Buffer.from("1a79847b753b9f0e1d64feca4b43f5a3ae99f3a3e142822fad8858d0969eb8d3381885dc697a90fd37b9275f1ab4d85e3f0cd7122b574213ec532e222c15df0f9000", "hex"));
}));

test('[NANO S] Configure-baker: Commission rates only', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '00E0';
    await configureBakerStep0(bitmap, transport);
    await configureBakerCommissionStep(true, true, true, true, "commission_rates", "5340107b065590038af108984528ae3ed2a04c8017fe7ca6f7879f302b65375aae9b759d8842096bfef7562d7e7e716dd01f3c892183c5955e1fe5037af971089000", sim, transport);
}));

test('[NANO S] Configure-baker: Single commission rate', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '0040';
    await configureBakerStep0(bitmap, transport);
    await configureBakerCommissionStep(false, true, false, true, "single_commission_rate" ,"9b5f30ce60f17c137de0ff1135ebd899b9ee00418928ee2f4f986c651508637c8bad35487518e6e2284a1b17b28358a38c33a9c015f78e33c66e79dcf471160b9000", sim, transport);
}));

test('[NANO S] Configure-baker: All parameters', setupZemu('nanos', async (sim, transport) => {
    const bitmap = '00ff';
    await configureBakerStep0(bitmap, transport);
    await configureBakerStep1(fullP1, aggregationVerifyKey + aggregationVerifyKeyProof, sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/all_parameters_1', [10, 0]);
    });
    await configureBakerUrlStep(url, sim, transport, async () => {
        await sim.waitScreenChange();
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_baker/all_parameters_url', [14, 0]);
    });
    await configureBakerCommissionStep(true, true, true, false, "all_parameters_commision","2c4dec07ca746a9e68485d381161aae741d1ffaabbc54169e299a53b9228c072a73cc124ca10c3b111ab7391b1165fec47bf82676ee87a7ed5d63384e679f30d9000", sim, transport);
}));