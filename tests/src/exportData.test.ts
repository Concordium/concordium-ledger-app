import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

const end = '9000';

const exportDataTest = (p1: number, p2: number, data: Buffer, expectedResult: string) => (async (sim: Zemu, transport: Transport) => {
    const tx = transport.send(0xe0, 0x07, p1, p2, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickBoth();
    await expect(tx).resolves.toEqual(
        Buffer.from(expectedResult + end, 'hex'),
    );
});

const testEach = test.each<'nanos' | 'nanosp' | 'nanox'>(['nanos']);

function createDataBuffer(...data: number[]) {
    const result = Buffer.alloc(data.length * 4);
    let i = 0;
    for (const d of data) {
        result.writeUint32BE(d, i * 4)
        i += 1;
    }
    return result;
}

testEach('[%s] Export Id Cred Sec on Mainnet', (device) => setupZemu(device, exportDataTest(0x02, 0x00, createDataBuffer(2,115), '33b9d19b2496f59ed853eb93b9d374482d2e03dd0a12e7807929d6ee54781bb1'))());
testEach('[%s] Export Prf Key on Mainnet', (device) => setupZemu(device, exportDataTest(0x03, 0x00, createDataBuffer(3,35), '4409e2e4acffeae641456b5f7406ecf3e1e8bd3472e2df67a9f1e8574f211bc5'))());
testEach('[%s] Export Blinding randomness on Mainnet', (device) => setupZemu(device, exportDataTest(0x04, 0x00, createDataBuffer(4, 5713), '1e3633af2b1dbe5600becfea0324bae1f4fa29f90bdf419f6fba1ff520cb3167'))());
testEach('[%s] Export attribute randomness on Mainnet', (device) => setupZemu(device, exportDataTest(0x05, 0x00, createDataBuffer(5,0,4,0), '6ef6ba6490fa37cd517d2b89a12b77edf756f89df5e6f5597440630cd4580b8f'))());

testEach('[%s] Export Id Cred Sec on Testnet', (device) => setupZemu(device, exportDataTest(0x02, 0x01, createDataBuffer(2,115), '33c9c538e362c5ac836afc08210f4b5d881ba65a0a45b7e353586dad0a0f56df'))());
testEach('[%s] Export Prf Key on Testnet', (device) => setupZemu(device, exportDataTest(0x03, 0x01, createDataBuffer(3,35), '41d794d0b06a7a31fb79bb76c44e6b87c63e78f9afe8a772fc64d20f3d9e8e82'))());
testEach('[%s] Export Blinding randomness on Testnet', (device) => setupZemu(device, exportDataTest(0x04, 0x01, createDataBuffer(4, 5713), '079eb7fe4a2e89007f411ede031543bd7f687d50341a5596e015c9f2f4c1f39b'))());
testEach('[%s] Export attribute randomness on Testnet', (device) => setupZemu(device, exportDataTest(0x05, 0x01, createDataBuffer(5,0,4,0), '409fa90314ec8fb4a2ae812fd77fe58bfac81765cad3990478ff7a73ba6d88ae'))());
