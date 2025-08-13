import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function getPublicKey(sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000', 'hex');
    const tx = transport.send(0xe0, 0x01, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('da342689aac8704e9a23a9a0075adb6e3c935abf6c137f897c7b50cf27c4dfdd9000', 'hex')
    );
}

test('[NANO S] Extract an account public key', setupZemu('nanos', getPublicKey));

test('[NANO SP] Extract an account public key', setupZemu('nanosp', getPublicKey));

test('[NANO X] Extract an account public key', setupZemu('nanox', getPublicKey));

async function keyWithNoPrompt(_sim: Zemu, transport: Transport) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000', 'hex');
    const tx = transport.send(0xe0, 0x01, 0x01, 0x00, data);
    await expect(tx).resolves.toEqual(
        Buffer.from('da342689aac8704e9a23a9a0075adb6e3c935abf6c137f897c7b50cf27c4dfdd9000', 'hex')
    );
}

test('[NANO S] Extract a public key with no prompt', setupZemu('nanos', keyWithNoPrompt));

test('[NANO SP] Extract a public key with no prompt', setupZemu('nanosp', keyWithNoPrompt));

test('[NANO X] Extract a public key with no prompt', setupZemu('nanox', keyWithNoPrompt));

async function signedKey(sim: Zemu, transport: Transport) {
    const data = Buffer.from('050000045100000000000000010000000200000000', 'hex');
    const tx = transport.send(0xe0, 0x01, 0x00, 0x01, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from(
            '9b2184153e198fd0d3c9b2f330ed3cd17db5ab76d5a75a8f168b8ed6caede4f997a2604185c434ddd34c953c15e91ea2002a033412923c1732b7eaae562c6ffedf1929c26d2247abe583f956c527dfb8732a29387dff4cdcedda4c9a937e58099000',
            'hex'
        )
    );
}

test('[NANO S] Extract a signed key', setupZemu('nanos', signedKey));

test('[NANO SP] Extract a signed key', setupZemu('nanosp', signedKey));

test('[NANO X] Extract a signed key', setupZemu('nanox', signedKey));

async function rootGovernance(sim: Zemu, transport: Transport) {
    const data = Buffer.from('050000045100000000000000010000000000000000', 'hex');
    const tx = transport.send(0xe0, 0x01, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('0fae3081138d74fc3b02e9cc0744cb6d438df4d83d30b28eccb9da048e1fd55d9000', 'hex')
    );
}

test('[NANO S] Extract a root governance public key', setupZemu('nanos', rootGovernance));

test('[NANO SP] Extract a root governance public key', setupZemu('nanosp', rootGovernance));

test('[NANO X] Extract a root governance public key', setupZemu('nanox', rootGovernance));

async function level1Governance(sim: Zemu, transport: Transport) {
    const data = Buffer.from('050000045100000000000000010000000100000000', 'hex');
    const tx = transport.send(0xe0, 0x01, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('4068d1d0db2b0b569b5cadf54d2bec551846708b5961b2704d6f452776fe1ee79000', 'hex')
    );
}

test('[NANO S] Extract a level 1 governance public key', setupZemu('nanos', level1Governance));

test('[NANO SP] Extract a level 1 governance public key', setupZemu('nanosp', level1Governance));

test('[NANO X] Extract a level 1 governance public key', setupZemu('nanox', level1Governance));

async function level2Governance(sim: Zemu, transport: Transport) {
    const data = Buffer.from('050000045100000000000000010000000200000000', 'hex');
    const tx = transport.send(0xe0, 0x01, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('9b2184153e198fd0d3c9b2f330ed3cd17db5ab76d5a75a8f168b8ed6caede4f99000', 'hex')
    );
}

test('[NANO S] Extract a level 2 governance public key', setupZemu('nanos', level2Governance));

test('[NANO SP] Extract a level 2 governance public key', setupZemu('nanosp', level2Governance));

test('[NANO X] Extract a level 2 governance public key', setupZemu('nanox', level2Governance));
