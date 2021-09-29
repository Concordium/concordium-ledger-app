import { resolve } from 'path';
import type { StartOptions } from '@zondax/zemu';
import Zemu from '@zondax/zemu';
import Transport from '@ledgerhq/hw-transport';

const SEED_PHRASE = 'vendor sphere crew wise puppy wise stand wait tissue boy fortune myself hamster intact window garment negative dynamic permit genre limb work dial guess';

export const optionsNanoS: StartOptions = {
    model: 'nanos',
    X11: true,
    logging: true,
    startDelay: 2000,
    custom: `-s "${SEED_PHRASE}" `,
    pressDelay: 100,
    pressDelayAfter: 100,
};

export const optionsNanoX: StartOptions = {
    model: 'nanox',
    X11: true,
    logging: true,
    startDelay: 2000,
    custom: `-s "${SEED_PHRASE}" `,
    pressDelay: 100,
    pressDelayAfter: 100,
};

export const NANOS_ELF_PATH = resolve('bin/nanos/concordium_nanos.elf');
export const NANOX_ELF_PATH = resolve('bin/nanox/concordium_nanox.elf');

export function setupZemu(device: 'nanos' | 'nanox', func: (sim: Zemu, transport: Transport) => Promise<void>) {
    return async () => {
        let sim;
        let simOptions;
        if (device === 'nanos') {
            sim = new Zemu(NANOS_ELF_PATH);
            simOptions = optionsNanoS;
        } else {
            sim = new Zemu(NANOX_ELF_PATH);
            simOptions = optionsNanoX;
        }

        try {
            await sim.start(simOptions);
            await func(sim, sim.getTransport());
        } finally {
            await sim.close();
        }
    };
}