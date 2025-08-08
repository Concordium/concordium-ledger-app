import Transport from "@ledgerhq/hw-transport";
import Zemu from "@zondax/zemu";
import chunkBuffer from "../helpers";
import { setupZemu } from "./options";

async function updateProtocol(
    sim: Zemu,
    transport: Transport,
    numMessageUi: number,
    numUrlUi: number,
    numHashUi: number
) {
    const name = `${sim.startOptions.model}_update_protocol`;
    let data = Buffer.from(
        "080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da7000000290100000000000001ad",
        "hex"
    );
    await transport.send(0xe0, 0x21, 0x00, 0x00, data);

    data = Buffer.from("0000000000000029", "hex");
    await transport.send(0xe0, 0x21, 0x01, 0x00, data);

    data = Buffer.from(
        "546869732069732061206272696566206d6573736167652061626f757420746865207570646174652e",
        "hex"
    );
    transport.send(0xe0, 0x21, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots(
        ".",
        `${name}/message`,
        [numMessageUi + 1],
        false
    ); // + 1 due to update type
    let snap = await sim.clickBoth(undefined, false);

    data = Buffer.from("0000000000000029", "hex");
    await transport.send(0xe0, 0x21, 0x01, 0x00, data);

    data = Buffer.from(
        "687474703a2f2f636f6e636f726469756d2e636f6d2f73706563696669636174696f6e2f762f313233",
        "hex"
    );
    transport.send(0xe0, 0x21, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snap);
    await sim.navigateAndCompareSnapshots(
        ".",
        `${name}/url`,
        [numUrlUi],
        false
    );
    snap = await sim.clickBoth(undefined, false);

    data = Buffer.from(
        "75e34b7235d488828961216c5b9fbd5b88fe74c76516eb635c57cc61632222de",
        "hex"
    );
    transport.send(0xe0, 0x21, 0x03, 0x00, data);
    await sim.waitUntilScreenIsNot(snap);
    await sim.navigateAndCompareSnapshots(
        ".",
        `${name}/hash`,
        [numHashUi],
        false
    );
    snap = await sim.clickBoth(undefined, false);

    const auxiliaryData = Buffer.from(
        "54686973207265616c6c792073686f756c6420626520736f6d6520617578696c69617279206461746120616e64206e6f74206120537472696e67206c696b6520746869732e2054686973206973206a75737420666f722074657374696e672e2054686973206d6573736167652073686f756c642062652061206c6f74206c6f6e6765722c20736f20746861742077652061637475616c6c792063726f7373203235352062797465732e205468657265206973207374696c6c2061206c6974746c652062697420746f20676f20746f2063726f737320746865203235352062797465732074686174207765206e6565642e20576520776f756c64206c696b65206d6f7265207468616e2032353520746f20656e737572652074686174206261746368696e6720776f726b732e",
        "hex"
    );
    const chunkedData = chunkBuffer(auxiliaryData, 255);
    for (let i = 0; i < chunkedData.length; i += 1) {
        const chunk = chunkedData[i];
        if (i === chunkedData.length - 1) {
            const tx = transport.send(0xe0, 0x21, 0x04, 0x00, chunk);
            await sim.waitUntilScreenIsNot(snap);
            await sim.navigateAndCompareSnapshots(
                ".",
                `${name}/aux`,
                [0],
                false
            );

            await expect(tx).resolves.toEqual(
                Buffer.from(
                    "48cd884f4cc25084ea7db8ba2912ca9163ef4a4ae9b847bbe7cdff90836c649d2df04e2102370c4a41b27a4e171664de1c009e850592996c19f7f2e3a8a2050f9000",
                    "hex"
                )
            );
            break;
        } else {
            await transport.send(0xe0, 0x21, 0x04, 0x00, chunk);
        }
    }
}

test(
    "[NANO S] Update protocol",
    setupZemu("nanos", async (sim, transport) => {
        await updateProtocol(sim, transport, 4, 1, 4);
    })
);

test(
    "[NANO SP] Update protocol",
    setupZemu("nanosp", async (sim, transport) => {
        await updateProtocol(sim, transport, 2, 1, 2);
    })
);
