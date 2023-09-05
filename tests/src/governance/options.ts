import { resolve } from 'path';
import Transport from '@ledgerhq/hw-transport';
import { ConcordiumZemu, optionsNanoS, optionsNanoX } from '../options';

export const NANOS_ELF_PATH = resolve('bin/governance/nanos/ccdGovernance_nanos.elf');
export const NANOX_ELF_PATH = resolve('bin/governance/nanox/ccdGovernance_nanox.elf');

export function setupZemu(device: 'nanos' | 'nanox', func: (sim: ConcordiumZemu, transport: Transport, device: 'nanos' | 'nanox') => Promise<void>) {
    return async () => {
        let sim;
        let simOptions;
        if (device === 'nanos') {
            sim = new ConcordiumZemu(NANOS_ELF_PATH);
            simOptions = optionsNanoS;
        } else {
            sim = new ConcordiumZemu(NANOX_ELF_PATH);
            simOptions = optionsNanoX;
        }

        try {
            await sim.start(simOptions);
            await func(sim, sim.getTransport(), device);
        } finally {
            await sim.close();
        }
    };
}
