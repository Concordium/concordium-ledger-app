import Zemu from "@zondax/zemu";
import { ISnapshot } from "@zondax/zemu/dist/types";

export function chunkBuffer(buffer: Buffer, chunkSize: number): Buffer[] {
    if (chunkSize <= 0) {
        throw new Error('Chunk size has to be a positive number.');
    }
    const chunks: Buffer[] = [];
    for (let i = 0; i < buffer.length; i += chunkSize) {
        chunks.push(buffer.slice(i, i + chunkSize));
    }
    return chunks;
}

/**
 * Wait until the screen is no the supplied screen. Adds an additional sleep to ensure
 * that the screen that it changes to is fully rendered.
 */
export async function safeWaitUntilScreenIsNot(sim: Zemu, screen: ISnapshot): Promise<void> {
    await sim.waitUntilScreenIsNot(screen);
    await Zemu.sleep(1000);
}
