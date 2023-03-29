import { resolve } from 'path';
import Zemu, { DEFAULT_START_OPTIONS, IStartOptions } from '@zondax/zemu';
import Transport from '@ledgerhq/hw-transport';

const SEED_PHRASE = 'vendor sphere crew wise puppy wise stand wait tissue boy fortune myself hamster intact window garment negative dynamic permit genre limb work dial guess';

const sharedOptions: Omit<IStartOptions, 'model'> = {
    ...DEFAULT_START_OPTIONS,
    caseSensitive: true,
    logging: true,
    startDelay: 3000,
    startTimeout: DEFAULT_START_OPTIONS.startTimeout,
    custom: `-s "${SEED_PHRASE}" `,
    startText: 'is ready',
};

export type LedgerModel = 'nanos' | 'nanosp' | 'nanox';

export const optionsNanoS: IStartOptions = {
    ...sharedOptions,
    model: 'nanos',
};

export const optionsNanoSPlus: IStartOptions = {
    ...sharedOptions,
    model: 'nanosp',
    custom: sharedOptions.custom + `-a 1`,
};

export const optionsNanoX: IStartOptions = {
    ...sharedOptions,
    model: 'nanox',
    custom: sharedOptions.custom + `-a 1`,
};

export const NANOS_ELF_PATH = resolve('bin/nanos/concordium_nanos.elf');
export const NANOX_ELF_PATH = resolve('bin/nanox/concordium_nanox.elf');
export const NANOS_PLUS_ELF_PATH = resolve('bin/nanosplus/concordium_nanosplus.elf');

export function setupZemu(device: 'nanos' | 'nanosp' | 'nanox', func: (sim: Zemu, transport: Transport, device: 'nanos' | 'nanosp' | 'nanox') => Promise<void>) {
    return async () => {
        let sim;
        let simOptions;
        if (device === 'nanos') {
            sim = new Zemu(NANOS_ELF_PATH);
            simOptions = optionsNanoS;
        } else if (device === 'nanosp') {
            sim = new Zemu(NANOS_PLUS_ELF_PATH);
            simOptions = optionsNanoSPlus;
        } else {
            sim = new Zemu(NANOX_ELF_PATH);
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
