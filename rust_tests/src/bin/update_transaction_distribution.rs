mod update_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();

    let mut update_type = hex::decode("07").unwrap();
    let mut baker_fee = hex::decode("0000AFC8").unwrap();
    let mut gas_account_fee = hex::decode("000088B8").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut baker_fee);
    command_data.append(&mut gas_account_fee);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 34,   // Sign transaction distribution update
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateTransactionFeeDistribution: {}", hex::encode(result.data));
}
