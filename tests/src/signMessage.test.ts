import { buildBasicAccountSigner, ConcordiumHdWallet, signMessage, AccountAddress } from '@concordium/node-sdk';
import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import chunkBuffer from './helpers';
import { SEED_PHRASE, setupZemu } from './options';

const signMessageTest = (message: Buffer, p2: number, signature: string, steps: number[]) => (async (sim: Zemu, transport: Transport) => {
    let snapshot = sim.getMainMenuSnapshot();
    const messageLength = Buffer.alloc(2);
    messageLength.writeUint16BE(message.length);
    let data = Buffer.concat([Buffer.from('060000002c000000010000000000000000000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7', 'hex'), messageLength]);
    await transport.send(0xe0, 0x38, 0x00, p2, data);

    let chunkIndex = 0;
    let tx;
    for (let chunk of chunkBuffer(message, 255)) {
        tx = transport.send(0xe0, 0x38, 0x01, p2, chunk);
        await sim.waitUntilScreenIsNot(snapshot);
        for (let i = 0; i < steps[chunkIndex]; i += 1) {
            await sim.clickRight();
        }
        snapshot = await sim.snapshot();
        await sim.clickBoth(undefined, false);
        chunkIndex += 1;
    }
    await expect(tx).resolves.toEqual(
        Buffer.from(signature + '9000', 'hex'),
    );
});

const testEach = test.each<'nanos' | 'nanosp' | 'nanox'>(['nanos', 'nanosp', 'nanox']);

const P2_ASCII = 0;
const P2_HEX = 1;

async function getSignature(message: string | Buffer) {
    const hdWallet = ConcordiumHdWallet.fromSeedPhrase(SEED_PHRASE, 'Testnet');
    const account = new AccountAddress('3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq');
    const signature = await signMessage(account, message, buildBasicAccountSigner(hdWallet.getAccountSigningKey(0,0,0).toString('hex')));
    return signature[0][0];
}

const shortASCIIMessage = 'Good test';
const longASCIIMessage = 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam turpis magna, ultricies elementum suscipit sed, accumsan ut ex. Phasellus non tempus erat. Praesent fermentum turpis vel arcu tempus placerat. Aenean sed elit et erat vulputate aliquet. Nunc eu ultrices tortor, ut dignissim nisl. Nunc congue urna non efficitur laoreet. Aenean sit amet augue id purus consequat molestie. Quisque aliquet purus id enim auctor, non aliquet justo cursus. Donec consequat, nibh rutrum varius porta, urna mi.';
const shortHEXMessage = '1234567890abcdef';
const longHEXMessage = '4c6f72656d20697073756d20646f6c6f722073697420616d65742c20636f6e73656374657475722061646970697363696e6720656c69742e204e756c6c616d20747572706973206d61676e612c20756c7472696369657320656c656d656e74756d207375736369706974207365642c20616363756d73616e2075742065782e2050686173656c6c7573206e6f6e2074656d70757320657261742e205072616573656e74206665726d656e74756d20747572706969676e697373696d206e69736c2e204e756e6320636f6e6775652075726e61206e6f6e20656666696369747572206c616f726565742e2041656e65616e2073697420616d657420';

testEach('[%s] Sign short ascii Message', async (device) => setupZemu(device, signMessageTest(Buffer.from(shortASCIIMessage, 'utf-8'), P2_ASCII, await getSignature(shortASCIIMessage), [device === "nanos" ? 7 : 4]))());
testEach('[%s] Sign long ascii Message', async (device) => setupZemu(device, signMessageTest(Buffer.from(longASCIIMessage, 'utf-8'), P2_ASCII, await getSignature(longASCIIMessage), device === "nanos" ? [21, 14] : [8, 5]))());
testEach('[%s] Sign short HEX Message', async (device) => setupZemu(device, signMessageTest(Buffer.from(shortHEXMessage, 'hex'), P2_HEX, await getSignature(Buffer.from(shortHEXMessage, 'hex')), [device === "nanos" ? 7 : 4]))());
testEach('[%s] Sign long HEX Message', async (device) => setupZemu(device, signMessageTest(Buffer.from(longHEXMessage, 'hex'), P2_HEX, await getSignature(Buffer.from(longHEXMessage, 'hex')), device === "nanos" ? [38, 14] : [14, 5]))());
