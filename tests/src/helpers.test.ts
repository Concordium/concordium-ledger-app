import { toHex } from "./helpers";

test('toHex gives correct value 1', () => {
    expect(toHex(1)).toEqual('01');
});

test('toHex gives correct value 2', () => {
    expect(toHex(255)).toEqual('ff');
});

test('toHex prefixes with 0', () => {
    expect(toHex(10)).toEqual('0a');
});

test('toHex result is even length', () => {
    expect(toHex(531)).toEqual('0213')
});
