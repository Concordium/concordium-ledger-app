import type { StartOptions } from '@zondax/zemu';

const SEED_PHRASE = 'vendor sphere crew wise puppy wise stand wait tissue boy fortune myself hamster intact window garment negative dynamic permit genre limb work dial guess';

export const optionsNanoS: StartOptions = {
  model: 'nanos',
  X11: true,
  logging: true,
  startDelay: 2000,
  custom: `-s "${SEED_PHRASE}" `,
  pressDelay: 0,
  pressDelayAfter: 0,
};