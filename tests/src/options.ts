/* eslint-disable lines-between-class-members */
/* eslint-disable no-dupe-class-members */
import { resolve } from 'node:path';
import process from 'node:process';

import Zemu, { DEFAULT_START_OPTIONS, IStartOptions } from '@zondax/zemu';
import Transport from '@ledgerhq/hw-transport';
import { ISnapshot } from '@zondax/zemu/dist/types';

const SEED_PHRASE =
    'vendor sphere crew wise puppy wise stand wait tissue boy fortune myself hamster intact window garment negative dynamic permit genre limb work dial guess';

const sharedOptions: Omit<IStartOptions, 'model'> = {
    ...DEFAULT_START_OPTIONS,
    caseSensitive: true,
    logging: true,
    startDelay: 3000,
    startTimeout: DEFAULT_START_OPTIONS.startTimeout,
    custom: `-s "${SEED_PHRASE}" `,
    startText: 'is ready',
    disablePool: true,
};

export type LedgerModel = 'nanos' | 'nanosp' | 'nanox';

export const optionsNanoS: IStartOptions = {
    ...sharedOptions,
    model: 'nanos',
};

export const optionsNanoSPlus: IStartOptions = {
    ...sharedOptions,
    model: 'nanosp',
};

export const optionsNanoX: IStartOptions = {
    ...sharedOptions,
    model: 'nanox',
};

export const NANOS_ELF_PATH = resolve('bin/nanos/concordium_nanos.elf');
export const NANOX_ELF_PATH = resolve('bin/nanox/concordium_nanox.elf');
export const NANOS_PLUS_ELF_PATH = resolve('bin/nanosplus/concordium_nanosplus.elf');

export class ConcordiumZemu extends Zemu {
    // This is a hack to get around the issue where a screen is not fully rendered, but
    // the screen has changed from the provided screenshot. This method waits a little bit
    // after the screen has changed in an attempt to ensure that it is fully rendered.
    async waitUntilScreenIsNot(screen: ISnapshot, timeout: number = 5000): Promise<void> {
        await super.waitUntilScreenIsNot(screen, timeout);
        await Zemu.sleep(100);
    }

    /**
     * Helper function for comparing tmp and golden snapshots for a testcase image.
     *
     * @param testcaseName - The name of the testcase, e.g. 'nanosp_update_auths'
     * @param imgName - The name of the image, e.g. 'tx_hash'
     * @returns true
     */
    // Implementation aligned with `Zemu.compareSnapshots`. Unfortunately, there is no
    // official helper function in the `zemu` library that does this.
    async compareSnapshot(testcaseName: string, imgName: string): Promise<true> {
        const snapshotGolden = resolve(`./snapshots/${testcaseName}/${imgName}.png`);
        const snapshotTmp = resolve(`./snapshots-tmp/${testcaseName}/${imgName}.png`);

        await this.snapshot(snapshotTmp); // Save the snapshot

        this.log(`golden      ${snapshotGolden}`);
        this.log(`tmp         ${snapshotTmp}`);
        this.log('Start comparison');
        this.log(`Checked     ${snapshotTmp}`);

        const img1 = Zemu.LoadPng2RGB(`${snapshotTmp}`);
        const img2 = Zemu.LoadPng2RGB(`${snapshotGolden}`);
        if (!img1.data.equals(img2.data)) {
            throw new Error(`Image [${imgName}] does not match!`);
        }
        return true;
    }
}

function getZemu(device: 'nanos' | 'nanosp' | 'nanox') {
    if (device === 'nanos') {
        return new ConcordiumZemu(NANOS_ELF_PATH);
    }
    if (device === 'nanosp') {
        return new ConcordiumZemu(NANOS_PLUS_ELF_PATH);
    }
    return new ConcordiumZemu(NANOX_ELF_PATH);
}

export function getSetupZemu(getter: typeof getZemu) {
    return (
            device: 'nanos' | 'nanosp' | 'nanox',
            func: (sim: ConcordiumZemu, transport: Transport, device: 'nanos' | 'nanosp' | 'nanox') => Promise<void>
        ) =>
        async () => {
            const sim = getter(device);
            let simOptions;
            if (device === 'nanos') {
                simOptions = optionsNanoS;
            } else if (device === 'nanosp') {
                simOptions = optionsNanoSPlus;
            } else {
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

export const setupZemu = getSetupZemu(getZemu);
