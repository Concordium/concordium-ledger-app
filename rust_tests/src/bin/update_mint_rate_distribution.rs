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

    let mut update_type = hex::decode("06").unwrap();

    let mut mint_rate_mantissa = hex::decode("00734B9F").unwrap();
    let mut mint_rate_exponent = hex::decode("10").unwrap();
    let mut baker_reward = hex::decode("0000EA60").unwrap();
    let mut finalization_reward = hex::decode("00007530").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_header_blob_bytes);
    command_data.append(&mut update_type);
    command_data.append(&mut mint_rate_mantissa);
    command_data.append(&mut mint_rate_exponent);
    command_data.append(&mut baker_reward);
    command_data.append(&mut finalization_reward);

    let command = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 37,    // Sign mint rate distribution update
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateMintRateDistribution: {}", hex::encode(result.data));
}
