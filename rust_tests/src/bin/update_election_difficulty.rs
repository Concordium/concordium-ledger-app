mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();

    // Hex part of update header.
    let update_sequence_number = "000000000000000A";
    let transaction_time = "0000000000000064";
    let transaction_expiry_time = "0000000063DE5DA7";
    let payload_size = "00000029";
    let update_header_blob = format!("{}{}{}{}", &update_sequence_number, &transaction_time, &transaction_expiry_time, &payload_size);
    let mut update_header_blob_bytes = hex::decode(update_header_blob).unwrap();

    let mut update_type = hex::decode("02").unwrap();
    let mut numerator = hex::decode("0000F000").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_header_blob_bytes);
    command_data.append(&mut update_type);
    command_data.append(&mut numerator);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 38,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateElectionDifficulty: {}", hex::encode(result.data));
}
