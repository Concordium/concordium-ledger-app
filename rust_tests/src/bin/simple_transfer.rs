mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

fn main() {
    let transaction_kind = "03";
    
    let mut amount = hex::decode("FFFFFFFFFFFFFFFF").unwrap();
    let sender_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut receiver_address = sender_address.from_base58check().unwrap().1;

    // Build transaction payload bytes.
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();
    transaction_payload.append(&mut receiver_address);
    transaction_payload.append(&mut amount);

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_payload);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 2,   // Transfer (Simple one time transfer)
        p1: 0,
        p2: 0,
        length: 60,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).expect("Simpler transfer packet failed.");
    println!("Signature: {}", hex::encode(result.data));
}