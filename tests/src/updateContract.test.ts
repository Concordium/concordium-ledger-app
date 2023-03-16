import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

const signatureEnd = "9000"

const header = "08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da702";
const amount = "0000FFFFFFFFFFFF";
const index = "0000FFFFFFFFFFFF"
const subindex = "0000FFFFFFFFFFFF";
const nameLength = "0009";
const name = "Test Name"
const parameterLength = "000E";
const parameter = "Parameter Name"

const updateContract = (initialSteps: number, parameterSteps: number, signature: string) => async (sim: Zemu, transport: Transport, device: string) =>  {
    let data = Buffer.from(header + amount + index + subindex + nameLength + parameterLength, 'hex');
    await transport.send(0xe0, 0x36, 0x00, 0x00, data);
    transport.send(0xe0, 0x36, 0x01, 0x00, Buffer.from(name));
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', `${device}_update_contract/standard_initial`, [initialSteps]);
    const continueScreen = await sim.snapshot();
    sim.clickBoth();
    const tx = transport.send(0xe0, 0x36, 0x02, 0x00, Buffer.from(parameter));
    await sim.waitUntilScreenIsNot(continueScreen);
    await sim.navigateAndCompareSnapshots('.', `${device}_update_contract/standard_parameter`, [parameterSteps]);
    sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from(signature + signatureEnd, 'hex'),
    );
}

const standardSignature = "34f35879db0cc50898ad9f8346554b6ab607410d02208a77dcd5a1a4d778491ffe45a1fd8381ac4e3f78e2badbbb0320b21264d391731234de85a226107e2a06";

test('[NANO S] Sign an update contract transaction', setupZemu('nanos', updateContract(10, 1, standardSignature)));
test('[NANO SP] Sign an update contract transaction', setupZemu('nanosp', updateContract(7,1, standardSignature)));
test('[NANO X] Sign an update contract transaction', setupZemu('nanox', updateContract(7,1, standardSignature)));


const updateContractNoParameters = (initialSteps: number, parameterSteps: number, signature: string) => async (sim: Zemu, transport: Transport, device: string) =>  {
    const zeroLength = "0000"
    let data = Buffer.from(header + amount + index + subindex + nameLength + zeroLength, 'hex');
    await transport.send(0xe0, 0x36, 0x00, 0x00, data);
    const tx = transport.send(0xe0, 0x36, 0x01, 0x00, Buffer.from(name));
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', `${device}_update_contract/no_parameter`, [initialSteps]);
    sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from(signature + signatureEnd, 'hex'),
    );
}

const noParameterSignature = "76c78f469429f2b5b061a46ce0d2c395cf2cbc7889b9efa709b2027cdd9a4095d8a38f33ec434a650e98d2b556fbf676514195a8f8b7c4c6eba6c75ac105e508";

test('[NANO S] Sign an update contract transaction with no parameters', setupZemu('nanos', updateContractNoParameters(11, 1, noParameterSignature)));
test('[NANO SP] Sign an update contract transaction with no parameters', setupZemu('nanosp', updateContractNoParameters(8,1, noParameterSignature)));
test('[NANO X] Sign an update contract transaction with no parameters', setupZemu('nanox', updateContractNoParameters(8,1, noParameterSignature)));
