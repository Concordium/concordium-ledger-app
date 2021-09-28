import Zemu from '@zondax/zemu';
import { NANOS_ELF_PATH, optionsNanoS } from './options';

test('Sign a valid simple transfer', async () => {
    const sim = new Zemu(NANOS_ELF_PATH);

    try {
        await sim.start(optionsNanoS);
        const transport = sim.getTransport();

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
    } finally {
        await sim.close();
    }
});
