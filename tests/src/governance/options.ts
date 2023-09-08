import { resolve } from 'path';
import { ConcordiumZemu, getSetupZemu } from '../options';

export const NANOS_ELF_PATH = resolve('bin/governance-nanos/ccdGovernance_nanos.elf');
export const NANOS_PLUS_ELF_PATH = resolve('bin/governance-nanosplus/ccdGovernance_nanosplus.elf');

function getZemu(device: 'nanos' | 'nanosp' | 'nanox') {
    if (device === 'nanos') {
        return new ConcordiumZemu(NANOS_ELF_PATH);
    } else {
        return new ConcordiumZemu(NANOS_PLUS_ELF_PATH);
    }
}

export const setupZemu = getSetupZemu(getZemu);
