import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { setupZemu } from './options';

async function configureDelegation(transaction: string, signature: string, sim: Zemu, transport: Transport, handleUi: () => Promise<void>) {
    const data = Buffer.from(transaction, "hex");
    const tx = transport.send(0xe0, 0x17, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();
    await expect(tx).resolves.toEqual(
        Buffer.from(signature, "hex")
    );
}

test('[NANO S] Configure delegation (none)', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0000', 'hex');
    transport.send(0xe0, 0x17, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO X] Configure delegation (none)', setupZemu('nanox', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0000', 'hex');
    transport.send(0xe0, 0x17, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO S] Configure delegation (capital)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00010000ffffffffffff", "5236560d222f35a0801ff4b2a37bc2151c43fb15267ceab0f4d8a43be80e40b701e91046d94b40a40c21bd7c02afc477f0b9981a4470aaa756c3ba6693d7280c9000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_capital', [7, 0]);
    });
}));

test('[NANO X] Configure delegation (capital)', setupZemu('nanox', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00010000ffffffffffff", "5236560d222f35a0801ff4b2a37bc2151c43fb15267ceab0f4d8a43be80e40b701e91046d94b40a40c21bd7c02afc477f0b9981a4470aaa756c3ba6693d7280c9000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanox_configure_delegation/nanox_configure_delegation_capital', [4, 0]);
    });
}));

test('[NANO S] Configure delegation (restake)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a000201", "bae27a390f059e24239d2d0fbbf084cf35735aca08998c0cfa9123bc597fa2cf170ba447d071430b440fda9dcb4f591cc65d0c4b86c3fba83cbc5ba66d6dd90d9000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_restake', [7, 0]);
    });
}));

test('[NANO X] Configure delegation (restake)', setupZemu('nanox', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a000201", "bae27a390f059e24239d2d0fbbf084cf35735aca08998c0cfa9123bc597fa2cf170ba447d071430b440fda9dcb4f591cc65d0c4b86c3fba83cbc5ba66d6dd90d9000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanox_configure_delegation/nanox_configure_delegation_restake', [4, 0]);
    });
}));

test('[NANO S] Configure delegation (pool)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00040100000000abcdefff", "fd86e63cf6c991d2cdda0a376d4f5879dcfa518b3009eb492bca171752070afb7161c12d1e7e6b5fc71f26a4b61cbd9b9edd9d21b9ec2d6ad93241b13d8f9f069000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_pool', [7, 0]);
    });
}));

test('[NANO X] Configure delegation (pool)', setupZemu('nanox', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00040100000000abcdefff", "fd86e63cf6c991d2cdda0a376d4f5879dcfa518b3009eb492bca171752070afb7161c12d1e7e6b5fc71f26a4b61cbd9b9edd9d21b9ec2d6ad93241b13d8f9f069000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanox_configure_delegation/nanox_configure_delegation_pool', [4, 0]);
    });
}));

test('[NANO S] Configure delegation (capital, pool)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00050000ffffffffffff0100000000abcdefff", "4bd97df489f0a80d286ecc46a1706f8dd01011007c6c6daa1fbeca6b19e44948cc8ec2555a64ab74bd6647243af1a3e9b636b44f50ba01277fc3c88bd7266a019000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_capital_pool', [8, 0]);
    });
}));

test('[NANO X] Configure delegation (capital, pool)', setupZemu('nanox', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00050000ffffffffffff0100000000abcdefff", "4bd97df489f0a80d286ecc46a1706f8dd01011007c6c6daa1fbeca6b19e44948cc8ec2555a64ab74bd6647243af1a3e9b636b44f50ba01277fc3c88bd7266a019000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanox_configure_delegation/nanox_configure_delegation_capital_pool', [5, 0]);
    });
}));

test('[NANO S] Configure delegation (capital, restake)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00030000ffffffffffff01", "1fd6be04b8a1e3d953dbb01adc88bd2627fc82597b4d81620638b50761c21da1b4f9dddffea693b7c13b827a0c32e469ca9cc5c8f7dd9cd870e9c59c07b1f60b9000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_capital_restake', [8, 0]);
    });
}));

test('[NANO X] Configure delegation (capital, restake)', setupZemu('nanox', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00030000ffffffffffff01", "1fd6be04b8a1e3d953dbb01adc88bd2627fc82597b4d81620638b50761c21da1b4f9dddffea693b7c13b827a0c32e469ca9cc5c8f7dd9cd870e9c59c07b1f60b9000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanox_configure_delegation/nanox_configure_delegation_capital_restake', [5, 0]);
    });
}));

test('[NANO S] Configure delegation (restake, pool)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0006010100000000abcdefff", "a2359b8c0c2cad87ed690db75d5f6292892a01f7086fced4b2c937b48f83b123cb7d160f7ad9061f494458ab32ee092b424962843d90453e12a3b8e8397521039000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_restake_pool', [8, 0]);
    });
}));

test('[NANO X] Configure delegation (restake, pool)', setupZemu('nanox', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0006010100000000abcdefff", "a2359b8c0c2cad87ed690db75d5f6292892a01f7086fced4b2c937b48f83b123cb7d160f7ad9061f494458ab32ee092b424962843d90453e12a3b8e8397521039000", sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanox_configure_delegation/nanox_configure_delegation_restake_pool', [5, 0]);
    });
}));

test('[NANO S] Configure delegation (capital, restake, pool)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00070000ffffffffffff010100000000abcdefff", "8e9212e857a48c2cc3f991341ec63aefee293435679c11b091820436453cd56e37e96daed41216ad344d4f1b8ad5a3d65de4b25d0d6bb085db9a58208e4a52049000" ,sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_capital_restake_pool', [9, 0]);
    });
}));

test('[NANO X] Configure delegation (capital, restake, pool)', setupZemu('nanox', async (sim, transport) => {
    await configureDelegation("08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00070000ffffffffffff010100000000abcdefff", "8e9212e857a48c2cc3f991341ec63aefee293435679c11b091820436453cd56e37e96daed41216ad344d4f1b8ad5a3d65de4b25d0d6bb085db9a58208e4a52049000" ,sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanox_configure_delegation/nanox_configure_delegation_capital_restake_pool', [6, 0]);
    });
}));
