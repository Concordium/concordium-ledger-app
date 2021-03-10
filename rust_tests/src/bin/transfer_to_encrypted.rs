mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let transaction_kind = "11";        // Transfer to encrypted is 17, i.e. 11 in hex.

    let mut amount = hex::decode("F0000000000F4240").unwrap();

    // Build transaction payload bytes.
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();
    transaction_payload.append(&mut amount);

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_payload);

    let command = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 17,    // Transfer to encrypted
        p1: 0,
        p2: 0,
        length: 0,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}