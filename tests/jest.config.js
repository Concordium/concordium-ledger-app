module.exports = {
    preset: 'ts-jest',
    testEnvironment: 'node',
    moduleFileExtensions: ['js', 'ts', 'json'],
    moduleDirectories: ['node_modules'],
    transform: { '^.+\\.ts$': ['ts-jest', { tsconfig: 'tsconfig.jest.json' }]},
    testTimeout: 120000,
    globalSetup: '<rootDir>/globalSetup.js',
    bail: true
};
