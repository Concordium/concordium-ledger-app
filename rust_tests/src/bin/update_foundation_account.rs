mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();

    // Hex part of update header.
    let update_sequence_number = "000000000000000A";
    let transaction_time = "0000000000000064";
    let transaction_expiry_time = "0000000063DE5DA7";
    let payload_size = "00000029";
    let update_header_blob = format!("{}{}{}{}", &update_sequence_number, &transaction_time, &transaction_expiry_time, &payload_size);
    let mut update_header_blob_bytes = hex::decode(update_header_blob).unwrap();

    let mut update_type = hex::decode("05").unwrap();

    // Base58 part of header, so we can't just use hex and map to bytes.
    let foundation_account_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut foundation_account_address_bytes = foundation_account_address.from_base58check().unwrap().1;

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_header_blob_bytes);
    command_data.append(&mut update_type);
    command_data.append(&mut foundation_account_address_bytes);

    let command = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 36,    // Sign update foundation account
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateFoundationAccount: {}", hex::encode(result.data));
}
