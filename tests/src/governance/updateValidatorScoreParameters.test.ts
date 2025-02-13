import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function validatorScoreParameters(sim: Zemu, transport: Transport, images: string) {
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da700000029' + '17' + '000000000000001b', 'hex');
    const tx = transport.send(0xe0, 0x47, 0x00, 0x00, data);

    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();

    const result = (await tx).toString('hex');
    const expected = '41916aa64bcd7492607f7f3148b28b8b113b23b86ebd954a378d36e0e71b3592e0d5dacb446166417306693f50cf0333d51f6964ee2b05927b2a750c1473bc079000';

    expect(result).toEqual(expected);
}

test('[NANO S] Validator score parameters', setupZemu('nanos', async (sim, transport) => {
    await validatorScoreParameters(sim, transport, 'nanos_validator_score_parameters');
}));

test('[NANO SP] Validator score parameters', setupZemu('nanosp', async (sim, transport) => {
    await validatorScoreParameters(sim, transport, 'nanosp_validator_score_parameters');
}));
