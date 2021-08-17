mod account_transaction_header;
mod path;

use std::env;

use hex;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

fn main() {
    let args: Vec<String> = env::args().collect();
    let with_memo = args.len() > 1;

    let transaction_kind = if with_memo { "16" } else { "03" };

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

    let p1 = if with_memo { 1 } else { 0 };

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 2,   // Transfer (Simple one time transfer)
        p1: p1,
        p2: 0,
        length: 60,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let mut result = ledger.exchange(command).expect("Simpler transfer packet failed.");

    if with_memo {
        let memo  = hex::decode(&args[1]).unwrap();
        for memo_part in memo.chunks(255) {
            let memo_command = ApduCommand {
                cla: 224, // Has to be this value for all commands.
                ins: 2,   // Transfer (Simple one time transfer)
                p1: 2,
                p2: 0,
                length: memo_part.len() as u8,
                data: memo_part.to_vec() // test assumes the memo fits
            };
            result = ledger.exchange(memo_command).expect("Simpler transfer packet failed.");
        }
    }

    println!("Signature: {}", hex::encode(result.data));
}
