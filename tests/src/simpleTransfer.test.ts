import { setupZemu } from './options';

test('[NANO S] Sign a valid simple transfer with memo', setupZemu('nanos', async (sim, transport) => {
    let data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71620a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d70005', 'hex');
    transport.send(0xe0, 0x32, 0x01, 0x00, data);

    console.log('Go for it');
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    const snapshot1 = await sim.clickBoth(undefined, false);

    data = Buffer.from('6474657374', 'hex');
    transport.send(0xe0, 0x32, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot1);
    const snapshot2 = await sim.clickBoth(undefined, false);

    data = Buffer.from('ffffffffffffffff', 'hex');
    const tx = transport.send(0xe0, 0x32, 0x03, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot2);
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('542b8448df7579b94337cea6e169d981c755b2bde8cbd01ea1698b2098b8295a3a401e784664068ec5da74ffe8554aabe2c01ba0f70923f43440f60eba669c0d9000', 'hex'),
    );
}));

test('[NANO X] Sign a valid simple transfer with memo', setupZemu('nanox', async (sim, transport) => {
    let data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71620a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d70005', 'hex');
    transport.send(0xe0, 0x32, 0x01, 0x00, data);

    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    const snapshot1 = await sim.clickBoth(undefined, false);

    data = Buffer.from('6474657374', 'hex');
    transport.send(0xe0, 0x32, 0x02, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot1);
    const snapshot2 = await sim.clickBoth(undefined, false);

    data = Buffer.from('ffffffffffffffff', 'hex');
    const tx = transport.send(0xe0, 0x32, 0x03, 0x00, data);
    await sim.waitUntilScreenIsNot(snapshot2);
    await sim.clickRight();
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('542b8448df7579b94337cea6e169d981c755b2bde8cbd01ea1698b2098b8295a3a401e784664068ec5da74ffe8554aabe2c01ba0f70923f43440f60eba669c0d9000', 'hex'),
    );
}));

test('[NANO S] Sign a valid simple transfer', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7ffffffffffffffff', 'hex');
    const tx = transport.send(0xe0, 0x02, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('12afcc203c73075ae3e4d89e01844b3fb1b2ecef26565d8c1220e04bddfb7ced0fe38b06a6df22669a20eea4b180ea3d1e1e4ad28a1d2bea29e518ad53f1550d9000', 'hex'),
    );
}));

test('[NANO X] Sign a valid simple transfer', setupZemu('nanox', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7ffffffffffffffff', 'hex');
    const tx = transport.send(0xe0, 0x02, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();

    await expect(tx).resolves.toEqual(
        Buffer.from('12afcc203c73075ae3e4d89e01844b3fb1b2ecef26565d8c1220e04bddfb7ced0fe38b06a6df22669a20eea4b180ea3d1e1e4ad28a1d2bea29e518ad53f1550d9000', 'hex'),
    );
}));

test('[NANO S] Decline to sign a valid simple transfer', setupZemu('nanos', async (sim, transport) => {
    expect.assertions(1);
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7ffffffffffffffff', 'hex');
    transport.send(0xe0, 0x02, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27013));
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
}));

test('[NANO X] Decline to sign a valid simple transfer', setupZemu('nanox', async (sim, transport) => {
    expect.assertions(1);
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7ffffffffffffffff', 'hex');
    transport.send(0xe0, 0x02, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27013));
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();
}));
