import { ConcordiumHdWallet } from '@concordium/node-sdk';
import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { SEED_PHRASE, setupZemu } from './options';

const end = '9000';

const exportDataTest = (p1: number, data: Buffer, expectedResult: string[]) => (async (sim: Zemu, transport: Transport) => {
    let tx = transport.send(0xe0, 0x07, p1, 0, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth(undefined, false);
    await expect(tx).resolves.toEqual(
        Buffer.from(expectedResult[0] + end, 'hex'),
    );

    let index = 1;
    const emptyData = Buffer.alloc(0);
    while (index < expectedResult.length) {
        tx = transport.send(0xe0, 0x07, 3, 0, emptyData);
        await expect(tx).resolves.toEqual(
            Buffer.from(expectedResult[index] + end, 'hex'),
        );
        index += 1;
    }

    await sim.waitUntilScreenIs(sim.getMainMenuSnapshot());
});

const testEach = test.each<'nanos' | 'nanosp' | 'nanox'>(['nanos', 'nanox', 'nanosp']);

function createDataBuffer(...data: number[]) {
    const result = Buffer.alloc(data.length * 4);
    let i = 0;
    for (const d of data) {
        result.writeUint32BE(d, i * 4)
        i += 1;
    }
    return result;
}

const P1_NO_ATTRIBUTES = 0x00;
const P1_ONLY_ATTRIBUTES = 0x01;
const P1_ALL = 0x02;
const MAINNET_COIN_TYPE = 919;
const TESTNET_COIN_TYPE = 1;

const identityProvider = 2;
const identity = 115;
const credential = 44;

const attributes = ['01', '02', '03', '04', '05', '06', '07', '08', '09', '10', '01', '02', '03', '04', '05', '06', '07', '08', '09', '10']
const attributeInput = "0014" + attributes.join('');
const mainnetDataWithAttributes = Buffer.concat([createDataBuffer(MAINNET_COIN_TYPE, identityProvider, identity, credential), Buffer.from(attributeInput, 'hex')]);

const mainnetHdWallet = ConcordiumHdWallet.fromSeedPhrase(SEED_PHRASE, 'Mainnet');
const mainnetInitialResponse = mainnetHdWallet.getIdCredSec(identityProvider, identity).toString('hex') + mainnetHdWallet.getPrfKey(identityProvider, identity).toString('hex') + mainnetHdWallet.getSignatureBlindingRandomness(identityProvider, identity).toString('hex');
const mainnetAttributeResponse = (start: number, end: number) => attributes.slice(start, end).map((att) => mainnetHdWallet.getAttributeCommitmentRandomness(identityProvider, identity, credential, parseInt(att, 16)).toString('hex')).join('');

testEach('[%s] Export identity data "Without attributes" Mainnet', (device) => setupZemu(device, exportDataTest(P1_NO_ATTRIBUTES, createDataBuffer(MAINNET_COIN_TYPE, identityProvider, identity), [mainnetInitialResponse]))());
testEach('[%s] Export identity data "All types" Mainnet', (device) => setupZemu(device, exportDataTest(P1_ALL, mainnetDataWithAttributes, [mainnetInitialResponse, mainnetAttributeResponse(0,7), mainnetAttributeResponse(7,14), mainnetAttributeResponse(14, 20)]))());
testEach('[%s] Export identity data "Only attributes" Mainnet', (device) => setupZemu(device, exportDataTest(P1_ONLY_ATTRIBUTES, mainnetDataWithAttributes, ['', mainnetAttributeResponse(0,7), mainnetAttributeResponse(7,14), mainnetAttributeResponse(14, 20)]))());

const testnetDataWithAttributes = Buffer.concat([createDataBuffer(TESTNET_COIN_TYPE, identityProvider, identity, credential), Buffer.from(attributeInput, 'hex')]);

const testnetHdWallet = ConcordiumHdWallet.fromSeedPhrase(SEED_PHRASE, 'Testnet');
const testnetInitialResponse = testnetHdWallet.getIdCredSec(identityProvider, identity).toString('hex') + testnetHdWallet.getPrfKey(identityProvider, identity).toString('hex') + testnetHdWallet.getSignatureBlindingRandomness(identityProvider, identity).toString('hex');
const testnetAttributeResponse = (start: number, end: number) => attributes.slice(start, end).map((att) => testnetHdWallet.getAttributeCommitmentRandomness(identityProvider, identity, credential, parseInt(att, 16)).toString('hex')).join('');

testEach('[%s] Export identity data "Without attributes" Testnet', (device) => setupZemu(device, exportDataTest(P1_NO_ATTRIBUTES, createDataBuffer(TESTNET_COIN_TYPE, identityProvider, identity), [testnetInitialResponse]))());
testEach('[%s] Export identity data "All types" Testnet', (device) => setupZemu(device, exportDataTest(P1_ALL, testnetDataWithAttributes, [testnetInitialResponse, testnetAttributeResponse(0,7), testnetAttributeResponse(7,14), testnetAttributeResponse(14, 20)]))());
testEach('[%s] Export identity data "Only attributes" Testnet', (device) => setupZemu(device, exportDataTest(P1_ONLY_ATTRIBUTES, testnetDataWithAttributes, ['', testnetAttributeResponse(0,7), testnetAttributeResponse(7,14), testnetAttributeResponse(14, 20)]))());
