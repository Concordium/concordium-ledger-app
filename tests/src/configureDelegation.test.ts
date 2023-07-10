import Transport from '@ledgerhq/hw-transport';
import Zemu from '@zondax/zemu';
import { LedgerModel, setupZemu } from './options';

async function configureDelegation(transaction: string, signature: string, sim: Zemu, transport: Transport, handleUi: () => Promise<void>) {
    const data = Buffer.from(transaction, 'hex');
    const tx = transport.send(0xe0, 0x17, 0x00, 0x00, data);
    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
    await handleUi();
    await expect(tx).resolves.toEqual(
        Buffer.from(signature, 'hex'),
    );
}

test('[NANO S] Configure delegation (none)', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0000', 'hex');
    transport.send(0xe0, 0x17, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO SP] Configure delegation (none)', setupZemu('nanosp', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0000', 'hex');
    transport.send(0xe0, 0x17, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO X] Configure delegation (none)', setupZemu('nanox', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0000', 'hex');
    transport.send(0xe0, 0x17, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO S] Configure delegation (capital)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00010000ffffffffffff', '5236560d222f35a0801ff4b2a37bc2151c43fb15267ceab0f4d8a43be80e40b701e91046d94b40a40c21bd7c02afc477f0b9981a4470aaa756c3ba6693d7280c9000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_capital', [8]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationCapitalXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00010000ffffffffffff', '5236560d222f35a0801ff4b2a37bc2151c43fb15267ceab0f4d8a43be80e40b701e91046d94b40a40c21bd7c02afc477f0b9981a4470aaa756c3ba6693d7280c9000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/${device}_configure_delegation_capital`, [4]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (capital)', setupZemu('nanosp', configureDelegationCapitalXAndSP));

test('[NANO X] Configure delegation (capital)', setupZemu('nanox', configureDelegationCapitalXAndSP));

test('[NANO S] Configure delegation (stop delegation)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00010000000000000000', 'ec7c4f56cb26c512c69d37e0e5211c1c9908e5e739f4b43f7702e9d4fd460f270aa28eff4b5beec18b377ce21cadaf1181013fc0b9740fba70cf8f1f1608b6059000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/stop_delegation', [7]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationStopDelegationXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00010000000000000000', 'ec7c4f56cb26c512c69d37e0e5211c1c9908e5e739f4b43f7702e9d4fd460f270aa28eff4b5beec18b377ce21cadaf1181013fc0b9740fba70cf8f1f1608b6059000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/stop_delegation`, [4]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (stop delegation)', setupZemu('nanosp', configureDelegationStopDelegationXAndSP));

test('[NANO X] Configure delegation (stop delegation)', setupZemu('nanox', configureDelegationStopDelegationXAndSP));

test('[NANO S] Configure delegation (restake)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a000201', 'bae27a390f059e24239d2d0fbbf084cf35735aca08998c0cfa9123bc597fa2cf170ba447d071430b440fda9dcb4f591cc65d0c4b86c3fba83cbc5ba66d6dd90d9000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_restake', [7]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationRestakeXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a000201', 'bae27a390f059e24239d2d0fbbf084cf35735aca08998c0cfa9123bc597fa2cf170ba447d071430b440fda9dcb4f591cc65d0c4b86c3fba83cbc5ba66d6dd90d9000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/${device}_configure_delegation_restake`, [4]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (restake)', setupZemu('nanosp', configureDelegationRestakeXAndSP));

test('[NANO X] Configure delegation (restake)', setupZemu('nanox', configureDelegationRestakeXAndSP));

test('[NANO S] Configure delegation (target)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00040100000000abcdefff', 'fd86e63cf6c991d2cdda0a376d4f5879dcfa518b3009eb492bca171752070afb7161c12d1e7e6b5fc71f26a4b61cbd9b9edd9d21b9ec2d6ad93241b13d8f9f069000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_target', [7]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationTargetXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00040100000000abcdefff', 'fd86e63cf6c991d2cdda0a376d4f5879dcfa518b3009eb492bca171752070afb7161c12d1e7e6b5fc71f26a4b61cbd9b9edd9d21b9ec2d6ad93241b13d8f9f069000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/${device}_configure_delegation_target`, [4]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (target)', setupZemu('nanosp', configureDelegationTargetXAndSP));

test('[NANO X] Configure delegation (target)', setupZemu('nanox', configureDelegationTargetXAndSP));

test('[NANO S] Configure delegation (capital, target)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00050000ffffffffffff0100000000abcdefff', '4bd97df489f0a80d286ecc46a1706f8dd01011007c6c6daa1fbeca6b19e44948cc8ec2555a64ab74bd6647243af1a3e9b636b44f50ba01277fc3c88bd7266a019000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_capital_target', [9]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationCapitalTargetXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00050000ffffffffffff0100000000abcdefff', '4bd97df489f0a80d286ecc46a1706f8dd01011007c6c6daa1fbeca6b19e44948cc8ec2555a64ab74bd6647243af1a3e9b636b44f50ba01277fc3c88bd7266a019000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/${device}_configure_delegation_capital_target`, [5]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (capital, target)', setupZemu('nanosp', configureDelegationCapitalTargetXAndSP));

test('[NANO X] Configure delegation (capital, target)', setupZemu('nanox', configureDelegationCapitalTargetXAndSP));

test('[NANO S] Configure delegation (capital, restake)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00030000ffffffffffff01', '1fd6be04b8a1e3d953dbb01adc88bd2627fc82597b4d81620638b50761c21da1b4f9dddffea693b7c13b827a0c32e469ca9cc5c8f7dd9cd870e9c59c07b1f60b9000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_capital_restake', [9]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationCapitalRestakeXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00030000ffffffffffff01', '1fd6be04b8a1e3d953dbb01adc88bd2627fc82597b4d81620638b50761c21da1b4f9dddffea693b7c13b827a0c32e469ca9cc5c8f7dd9cd870e9c59c07b1f60b9000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/${device}_configure_delegation_capital_restake`, [5]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (capital, restake)', setupZemu('nanosp', configureDelegationCapitalRestakeXAndSP));

test('[NANO X] Configure delegation (capital, restake)', setupZemu('nanox', configureDelegationCapitalRestakeXAndSP));

test('[NANO S] Configure delegation (restake, target)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0006010100000000abcdefff', 'a2359b8c0c2cad87ed690db75d5f6292892a01f7086fced4b2c937b48f83b123cb7d160f7ad9061f494458ab32ee092b424962843d90453e12a3b8e8397521039000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_restake_target', [8]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationRestakeTargetXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a0006010100000000abcdefff', 'a2359b8c0c2cad87ed690db75d5f6292892a01f7086fced4b2c937b48f83b123cb7d160f7ad9061f494458ab32ee092b424962843d90453e12a3b8e8397521039000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/${device}_configure_delegation_restake_target`, [5]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (restake, target)', setupZemu('nanosp', configureDelegationRestakeTargetXAndSP));

test('[NANO X] Configure delegation (restake, target)', setupZemu('nanox', configureDelegationRestakeTargetXAndSP));

test('[NANO S] Configure delegation (capital, restake, target)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00070000ffffffffffff010100000000abcdefff', '8e9212e857a48c2cc3f991341ec63aefee293435679c11b091820436453cd56e37e96daed41216ad344d4f1b8ad5a3d65de4b25d0d6bb085db9a58208e4a52049000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_capital_restake_target', [10]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationCapitalRestakeTargetXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00070000ffffffffffff010100000000abcdefff', '8e9212e857a48c2cc3f991341ec63aefee293435679c11b091820436453cd56e37e96daed41216ad344d4f1b8ad5a3d65de4b25d0d6bb085db9a58208e4a52049000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/${device}_configure_delegation_capital_restake_target`, [6]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (capital, restake, target)', setupZemu('nanosp', configureDelegationCapitalRestakeTargetXAndSP));

test('[NANO X] Configure delegation (capital, restake, target)', setupZemu('nanox', configureDelegationCapitalRestakeTargetXAndSP));

test('[NANO S] Configure delegation (passive delegation)', setupZemu('nanos', async (sim, transport) => {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a000400', 'b8a1c21533e2af7d6d6c9966ca4d9770588198663ef2d29e0c94babbf6a3652f89238a784cc2a022e06e37c0d257ceade210ea050f515b79a7cad65b6c6da9059000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', 'nanos_configure_delegation/nanos_configure_delegation_passive', [7]);
        await sim.clickBoth(undefined, false);
    });
}));

async function configureDelegationPassiveDelegationXAndSP(sim: Zemu, transport: Transport, device: LedgerModel) {
    await configureDelegation('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a000400', 'b8a1c21533e2af7d6d6c9966ca4d9770588198663ef2d29e0c94babbf6a3652f89238a784cc2a022e06e37c0d257ceade210ea050f515b79a7cad65b6c6da9059000', sim, transport, async () => {
        await sim.navigateAndCompareSnapshots('.', `${device}_configure_delegation/${device}_configure_delegation_passive`, [4]);
        await sim.clickBoth(undefined, false);
    });
}

test('[NANO SP] Configure delegation (passive delegation)', setupZemu('nanosp', configureDelegationPassiveDelegationXAndSP));

test('[NANO X] Configure delegation (passive delegation)', setupZemu('nanox', configureDelegationPassiveDelegationXAndSP));

test('[NANO S] Configure delegation (bad datalength)', setupZemu('nanos', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00070000ffffffffffff010100000000abcdefff00', 'hex');
    transport.send(0xe0, 0x17, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO SP] Configure delegation (bad datalength)', setupZemu('nanosp', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00070000ffffffffffff010100000000abcdefff00', 'hex');
    transport.send(0xe0, 0x17, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));

test('[NANO X] Configure delegation (bad datalength)', setupZemu('nanox', async (sim, transport) => {
    const data = Buffer.from('08000004510000000000000000000000000000000000000002000000000000000020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00070000ffffffffffff010100000000abcdefff00', 'hex');
    transport.send(0xe0, 0x17, 0x00, 0x00, data).catch((e) => expect(e.statusCode).toEqual(27396));
}));
