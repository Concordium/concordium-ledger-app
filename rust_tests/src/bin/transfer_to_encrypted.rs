mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

fn main() {

    // Base58 part of header, so we can't just use hex and map to bytes.
    let sender_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut transaction_header = sender_address.from_base58check().unwrap().1;

    // Hex part of header.
    let nonce = "000000000000000A";
    let energy = "0000000000000064";
    let payload_size = "00000029";
    let expiry = "0000000063DE5DA7";
    let transaction_header_blob = format!("{}{}{}{}", &nonce, &energy, &payload_size, &expiry);
    let mut transaction_header_blob_bytes = hex::decode(transaction_header_blob).unwrap();

    // The full transaction header (sender_address + blob)
    transaction_header.append(&mut transaction_header_blob_bytes);

    // Transaction payload
    let transaction_kind = "11";        // Transfer to encrypted is 17, i.e. 11 in hex.
    let mut amount = hex::decode("F0000000000F4240").unwrap();

    // Build transaction payload bytes.
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();
    transaction_payload.append(&mut amount);

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut transaction_header);
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