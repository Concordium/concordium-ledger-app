import Transport from "@ledgerhq/hw-transport";
import Zemu from "@zondax/zemu";
import { setupZemu } from "./options";

test('[NANO S] Create PLT - Phase 1 (Initial)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command with derivation path and update header
    const data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918', 'hex');
    const response = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response).toEqual(Buffer.from('9000', 'hex'));
}));

test('[NANO S] Create PLT - Phase 1 & 2 (Initial + Payload)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

    // Phase 2: Payload + init params length
    data = Buffer.from('03545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000001', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);

        // This should succeed if our state machine is working
        expect(response2).toEqual(Buffer.from("9000", "hex"));
    })
);

test('[NANO S] Create PLT - Wrong P1 order (should fail)', setupZemu('nanos', async (sim, transport) => {
    // Phase 1: Initial command
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from('9000', 'hex'));

        // Skip Phase 2, try Phase 3 directly (should fail with INVALID_STATE)
        data = Buffer.from("ff", "hex");
        const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

        await expect(tx).rejects.toMatchObject({
            message: expect.stringContaining("6b01"),
        });
    })
);

async function createPlt(
    sim: Zemu,
    transport: Transport,
    images: string,
    device?: "nanos" | "nanosp"
) {
    // Phase 1: Initial
    let data = Buffer.from('080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918', 'hex');
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from("9000", "hex"));

    // Phase 2: Payload + init params length
    data = Buffer.from('03545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000001', 'hex');
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from("9000", "hex"));

    // Phase 3: Init params (1 byte)
    data = Buffer.from("ff", "hex");
    const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    switch (device) {
        case "nanos":
            await sim.navigateAndCompareSnapshots(".", images, [8, 0]);
            break;
        case "nanosp":
            await sim.navigateAndCompareSnapshots(".", images, [6, 0]);
            break;
    }
    await expect(tx).resolves.toEqual(
        Buffer.from('4a623bcb4b21f7aa79eed1d443e7835c6079d43a414be67a96e21064a31d5b8d09643841754199f75d92595ee4a0a6e7220ce47fab75a1906e944fcb9cd8e3059000', 'hex'),
    );
}

function chunkBuffer(buffer: Buffer, chunkSize: number): Buffer[] {
    if (chunkSize <= 0) {
        throw new Error("Chunk size has to be a positive number.");
    }
    const chunks: Buffer[] = [];
    for (let i = 0; i < buffer.length; i += chunkSize) {
        chunks.push(buffer.slice(i, i + chunkSize));
    }
    return chunks;
}

async function createPltPaginated(
    sim: Zemu,
    transport: Transport,
    images: string
) {
    // Phase 1: Initial
    let data = Buffer.from(
        "080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918",
        "hex"
    );
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from("9000", "hex"));

    // Phase 2: Payload + init params length (48 bytes)
    data = Buffer.from(
        "03545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000030",
        "hex"
    );
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from("9000", "hex"));

    // Phase 3: Init params (48 bytes)
    data = Buffer.from(
        "a8646e616d65637a6273686275726e61626c65f56864656ea8646e616d65637a6273686275726e61626c65f56864656e",
        "hex"
    );
    const tx = transport.send(0xe0, 0x48, 0x02, 0x00, data);

    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots(".", images, [7, 0]);

    await expect(tx).resolves.toEqual(
        Buffer.from(
            "40a983b0b25c0da8899361bd0c4e20931b600842389d743fdcf204ad424ff186a8b7188271aefe743a797fd68a6552cac6ae9bf59e17899229a44b161eb1f6089000",
            "hex"
        )
    );
}

async function createPltTruncated(
    sim: Zemu,
    transport: Transport,
    images: string
) {
    // Phase 1: Initial
    let data = Buffer.from(
        "080000045100000000000000000000000000000000000000020000000000000000000000000000000a00000000000000640000000063de5da70000002918",
        "hex"
    );
    const response1 = await transport.send(0xe0, 0x48, 0x00, 0x00, data);
    expect(response1).toEqual(Buffer.from("9000", "hex"));

    // Phase 2: Payload + init params length (0x0258 = 600 bytes)
    data = Buffer.from(
        "03545259af5684e70c1438e442066d017e4410af6da2b53bfa651a07d81efa2aa668db200600000258",
        "hex"
    );
    const response2 = await transport.send(0xe0, 0x48, 0x01, 0x00, data);
    expect(response2).toEqual(Buffer.from("9000", "hex"));

    // Phase 3: Init params (600 bytes)
    data = Buffer.from(
        "a8646e616d65637a6273686275726e61626c65f56864656e794c697374f4686d65746164617461a16375726c7068747470733a2f2f746573742e636f6d686d696e7461626c65f569616c6c6f774c697374f46d696e697469616c537570706c79a26576616c756519271068646563696d616c730271676f7665726e616e63654163636f756e74a26474797065676163636f756e746761646472657373a3665f5f74797065736363645f6163636f756e745f61646472657373676164647265737378323379624a363673705a327864574633617667785162326d656f755961376d70764d574e506d556e637a5538466f46386347426e6465636f64656441646472657373582087e3bec61b8db2fb7389b57d2be4f7dd95d1088dfeb6ef7352c13d2b2d27bb49a8646e616d65637a6273686275726e61626c65f56864656e794c697374f4686d65746164617461a16375726c7068747470733a2f2f746573742e636f6d686d696e7461626c65f569616c6c6f774c697374f46d696e697469616c537570706c79a26576616c756519271068646563696d616c730271676f7665726e616e63654163636f756e74a26474797065676163636f756e746761646472657373a3665f5f74797065736363645f6163636f756e745f61646472657373676164647265737378323379624a363673705a327864574633617667785162326d656f755961376d70764d574e506d556e637a5538466f46386347426e6465636f64656441646472657373582087e3bec61b8db2fb7389b57d2be4f7dd95d1088dfeb6ef7352c13d2b2d27bb49c13d2b2d27bb49c13d2b2d27bb49",
        "hex"
    );
    const chunks = chunkBuffer(data, 255);
    let tx;
    for (let i = 0; i < chunks.length; i++) {
        data = chunks[i];
        const res = transport.send(0xe0, 0x48, 0x02, 0x00, data);
        if (i < chunks.length - 1) {
            expect(await res).toEqual(Buffer.from("9000", "hex"));
        } else {
            tx = res;
        }
    }

    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.navigateAndCompareSnapshots(".", images, [27, 0]);

    await expect(tx).resolves.toEqual(
        Buffer.from(
            "3f87227970e04e0a79a83f87436c01e9398477067786cda2abf355a1e99d466c0a399d38e18b9cb1084ca1f9579b5ddf25a36721f33484f437646dd3b8c713099000",
            "hex"
        )
    );
}

test(
    "[NANO S] Create PLT",
    setupZemu("nanos", async (sim, transport) => {
        await createPlt(sim, transport, "nanos_create_plt/small", "nanos");
    })
);

test(
    "[NANO SP] Create PLT",
    setupZemu("nanosp", async (sim, transport) => {
        await createPlt(sim, transport, "nanosp_create_plt/small", "nanosp");
    })
);

test(
    "[NANO SP] Create PLT paginated params",
    setupZemu("nanosp", async (sim, transport) => {
        await createPltPaginated(sim, transport, "nanosp_create_plt/paginated");
    })
);

test(
    "[NANO SP] Create PLT truncated params (>512 bytes)",
    setupZemu("nanosp", async (sim, transport) => {
        await createPltTruncated(sim, transport, "nanosp_create_plt/truncated");
    })
);
