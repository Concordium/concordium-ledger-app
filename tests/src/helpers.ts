export default function chunkBuffer(buffer: Buffer, chunkSize: number): Buffer[] {
    if (chunkSize <= 0) {
        throw new Error('Chunk size has to be a positive number.');
    }
    const chunks: Buffer[] = [];
    for (let i = 0; i < buffer.length; i += chunkSize) {
        chunks.push(buffer.slice(i, i + chunkSize));
    }
    return chunks;
}

export function toHex(input: number) {
    const unpadded = input.toString(16);
    if (unpadded.length % 2 === 1) {
        return '0' + unpadded;
    }
    return unpadded;
}
