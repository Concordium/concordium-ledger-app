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

    let mut update_type = hex::decode("08").unwrap();
    let mut baker_gas = hex::decode("000061A8").unwrap();
    let mut finalization_proof_gas = hex::decode("000001F4").unwrap();
    let mut account_creation_gas = hex::decode("000000C8").unwrap();
    let mut chain_update_gas = hex::decode("00000190").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_header_blob_bytes);
    command_data.append(&mut update_type);
    command_data.append(&mut baker_gas);
    command_data.append(&mut finalization_proof_gas);
    command_data.append(&mut account_creation_gas);
    command_data.append(&mut chain_update_gas);

    let command = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 35,    // Sign GAS rewards update
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateGasRewards: {}", hex::encode(result.data));
}
