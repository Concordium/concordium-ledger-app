mod account_transaction_header;
mod path;

use std::env;

use base58check::*;
use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let args: Vec<String> = env::args().collect();
    let with_memo = args.len() > 1;
    let transaction_kind = if with_memo { "18" } else { "13" };

    let sender_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut receiver_address = sender_address.from_base58check().unwrap().1;

    // For scheduled amounts we have to split the transaction into multiple packages, as we can
    // only move 255 bytes at a time.
    let mut number_of_scheduled_amounts = hex::decode("0F").unwrap();

    // Send initial command.
    // let initial_command_data = format!("{}{}{}{}{}", &path_prefix, &number_of_scheduled_amounts, &transaction_header, &transaction_kind, &to_address);

    // Build transaction payload bytes which is in the initial command.
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();
    transaction_payload.append(&mut receiver_address);
    transaction_payload.append(&mut number_of_scheduled_amounts);

    let mut initial_command_data = path::generate_key_derivation_path();
    initial_command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    initial_command_data.append(&mut transaction_payload);

    let initial_command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 3,   // Scheduled transfer
        p1: 0,
        p2: 0,
        length: 0,
        data: initial_command_data
    };
    let ledger = LedgerApp::new().unwrap();
    let mut result = ledger.exchange(initial_command).expect("Initial packet failed.");

    // Generate 16 scheduled transfers to demonstrate that splitting by 15 transfers works.
    let mut command_data: Vec<u8> = Vec::new();

    for i in 1..16 {
        let mut scheduled_timestamp = hex::decode("0000017A396883D9").unwrap();
        let mut amount = hex::decode("0000000005F5E100").unwrap();

        command_data.append(&mut scheduled_timestamp);
        command_data.append(&mut amount);

        // Send APDU 15 pairs at a time, or when done iterating.
        // Note that the packets containing the scheduled amounts must set P1 = 0x01.
        if i % 15 == 0 {
            println!("{}", &command_data.len());
            let command = ApduCommand {
                cla: 224,
                ins: 3,
                p1: 1,
                p2: 0,
                length: 0,
                data: command_data
            };

            result = ledger.exchange(command).expect("Scheduled transfer command failed");
            command_data = Vec::new();
        }
    }

    if with_memo {
        let memo  = hex::decode(&args[1]).unwrap();
        for memo_part in memo.chunks(255) {
            let memo_command = ApduCommand {
                cla: 224, // Has to be this value for all commands.
                ins: 3,
                p1: 3,
                p2: 0,
                length: memo_part.len() as u8,
                data: memo_part.to_vec()
            };
            result = ledger.exchange(memo_command).unwrap();
        }
    }

    println!("{}", hex::encode(result.data));
}
