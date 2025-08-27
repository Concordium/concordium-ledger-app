# Concordium Ledger App Tests

This directory contains end-to-end tests for the Concordium Ledger application. The tests use [Zemu](https://github.com/Zondax/zemu) to simulate a Ledger device and verify application behavior.

## Setup

Before running tests, ensure you have the required dependencies installed:

```bash
# Install dependencies
yarn install
```

## Running Tests

To run all tests:

```bash
yarn test
```

### Running a Specific Test

To run a specific test or test file:

```bash
# Run tests matching a specific name pattern
yarn test -t "test_name_pattern"

# Run a specific test file
yarn test src/simpleTransfer.test.ts
```

## Snapshot Testing

The tests use Jest's snapshot testing to verify the UI flow of the Ledger application. Snapshots are stored in the `snapshots` directory.

### Updating Snapshots

When you make intentional changes to the application UI, you'll need to update the snapshots:

```bash
# Update all snapshots
ZEMU_UPDATE_SNAPSHOTS=1 yarn test

# Update snapshots for a specific test spec
ZEMU_UPDATE_SNAPSHOTS=1 yarn test src/simpleTransfer.test.ts
```

### Reviewing Snapshots

Snapshots are stored as PNG images in the `snapshots` directory. When a test fails because of UI changes, compare the images in:

- `snapshots/`: Contains the expected (baseline) snapshots
- `snapshots-tmp/`: Contains the actual snapshots from the test run

## Test Structure

- `src/*.test.ts`: Test files for different application features
- `src/governance/*.test.ts`: Tests for governance application features
- `src/helpers.ts`: Common test helper functions
- `src/options.ts`: Test configuration options

## Troubleshooting

### Common Issues

1. **Test Timeouts**
   - Tests may timeout if the device simulation takes too long
   - Try increasing the timeout in `jest.config.js` if needed

2. **Snapshot Differences**
   - UI changes will cause snapshot tests to fail
   - Review the images in `snapshots-tmp/` to ensure changes are expected
   - Update snapshots if changes are intentional

3. **Binary Build Issues**
   - Make sure the app binaries are built properly before testing
   - Run `./build_binaries.sh` to rebuild the test binaries if needed
