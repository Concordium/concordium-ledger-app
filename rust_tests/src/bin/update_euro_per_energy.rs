mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("03").unwrap();

    let mut numerator = hex::decode("0000000000000001").unwrap();
    let mut denominator = hex::decode("00000000000000F2").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut numerator);
    command_data.append(&mut denominator);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 6,   // Sign euro per energy
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateEuroPerEnergy: {}", hex::encode(result.data));
}
