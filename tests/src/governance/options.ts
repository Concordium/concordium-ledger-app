import { resolve } from 'path';
import Transport from '@ledgerhq/hw-transport';
import { ConcordiumZemu, optionsNanoS } from '../options';

export const NANOS_ELF_PATH = resolve('bin/governance-nanos/ccdGovernance_nanos.elf');

export function setupZemu(device: 'nanos', func: (sim: ConcordiumZemu, transport: Transport, device: 'nanos') => Promise<void>) {
    return async () => {
        let sim;
        let simOptions;
        if (device === 'nanos') {
            sim = new ConcordiumZemu(NANOS_ELF_PATH);
            simOptions = optionsNanoS;
        } else {
            throw new Error('Not supported device');
        }

        try {
            await sim.start(simOptions);
            await func(sim, sim.getTransport(), device);
        } finally {
            await sim.close();
        }
    };
}
