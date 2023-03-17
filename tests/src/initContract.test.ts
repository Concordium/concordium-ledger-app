import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

const signatureEnd = "9000"

const header = "08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da701";
const amount = "0000FFFFFFFFFFFF";
const moduleRef = "69d48cea644389f65be2cd807df746abc8b97d888dc98ae531030c2a3bffeee0"
const nameLength = "0009";
const name = "Test Name"
const parameterLength = "000E";
const parameter = "Parameter Name"

const initContract = (initialSteps: number, parameterSteps: number, signature: string) => async (sim: Zemu, transport: Transport, device: string) =>  {
    let data = Buffer.from(header + amount + moduleRef + nameLength + parameterLength, 'hex');
    await transport.send(0xe0, 0x37, 0x00, 0x00, data);
    transport.send(0xe0, 0x37, 0x01, 0x00, Buffer.from(name));
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', `${device}_init_contract/standard_initial`, [initialSteps]);
    const continueScreen = await sim.snapshot();
    sim.clickBoth();
    const tx = transport.send(0xe0, 0x37, 0x02, 0x00, Buffer.from(parameter));
    await sim.waitUntilScreenIsNot(continueScreen);
    await sim.navigateAndCompareSnapshots('.', `${device}_init_contract/standard_parameter`, [parameterSteps]);
    sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from(signature + signatureEnd, 'hex'),
    );
}

const standardSignature = "321ca85d32926cc48f7dcd5d60ceda1d9ddf5eeb2bb7f411d5f0ee9dfc6573c5401f430ed1429f1f968f24dfffd2caf88bda58a4da2dcd9311bb5b0144d48d04";

test('[NANO S] Sign an init contract transaction', setupZemu('nanos', initContract(12, 1, standardSignature)));
test('[NANO SP] Sign an init contract transaction', setupZemu('nanosp', initContract(7,1, standardSignature)));
test('[NANO X] Sign an init contract transaction', setupZemu('nanox', initContract(7,1, standardSignature)));


const initContractNoParameters = (initialSteps: number, parameterSteps: number, signature: string) => async (sim: Zemu, transport: Transport, device: string) =>  {
    const zeroLength = "0000"
    let data = Buffer.from(header + amount + moduleRef + nameLength + zeroLength, 'hex');
    await transport.send(0xe0, 0x37, 0x00, 0x00, data);
    const tx = transport.send(0xe0, 0x37, 0x01, 0x00, Buffer.from(name));
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots('.', `${device}_init_contract/no_parameter`, [initialSteps]);
    sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from(signature + signatureEnd, 'hex'),
    );
}

const noParameterSignature = "55f0b8ad93f9d61bf69e32c8824f1df6672fe4397b0359a0d058bcb267f117f06b43151a516ff6ba2d51b0d4f14ba1f5d9eac5737a5f4a8e188fb8cfaf6efa07";

test('[NANO S] Sign an init contract transaction with no parameters', setupZemu('nanos', initContractNoParameters(13, 1, noParameterSignature)));
test('[NANO SP] Sign an init contract transaction with no parameters', setupZemu('nanosp', initContractNoParameters(8,1, noParameterSignature)));
test('[NANO X] Sign an init contract transaction with no parameters', setupZemu('nanox', initContractNoParameters(8,1, noParameterSignature)));
