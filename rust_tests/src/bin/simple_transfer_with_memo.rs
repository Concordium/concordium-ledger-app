mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

fn main() {
    let ins_simple_with_memo = 50;
    let transaction_kind =  "16";

    let amount = hex::decode("FFFFFFFFFFFFFFFF").unwrap();
    let sender_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut receiver_address = sender_address.from_base58check().unwrap().1;

    let memo  = hex::decode("6474657374").unwrap();

    // Build transaction payload bytes.
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();
    transaction_payload.append(&mut receiver_address);

    transaction_payload.append(&mut (memo.len() as u16).to_be_bytes().to_vec());

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_payload);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: ins_simple_with_memo,
        p1: 1,
        p2: 0,
        length: 60,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(command).expect("Simpler transfer packet failed.");

        for memo_part in memo.chunks(255) {
            let memo_command = ApduCommand {
                cla: 224, // Has to be this value for all commands.
                ins: ins_simple_with_memo,
                p1: 2,
                p2: 0,
                length: memo_part.len() as u8,
                data: memo_part.to_vec() // test assumes the memo fits
            };
            ledger.exchange(memo_command).expect("Simpler transfer packet failed.");
        }

    let amount_command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: ins_simple_with_memo,
        p1: 3,
        p2: 0,
        length: 8,
        data: amount
    };

    let result = ledger.exchange(amount_command).expect("Simpler transfer packet failed.");

    println!("Signature: {}", hex::encode(result.data));
}
