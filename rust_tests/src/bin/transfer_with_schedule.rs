mod account_transaction_header;
mod path;

use base58check::*;
use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    // Transaction kind is 19 -> 13 in hexadecimal.
    let mut transaction_kind = hex::decode("13").unwrap();

    let sender_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut receiver_address = sender_address.from_base58check().unwrap().1;

    // For scheduled amounts we have to split the transaction into multiple packages, as we can
    // only move 255 bytes at a time.
    let mut number_of_scheduled_amounts = hex::decode("03").unwrap();

    // Send initial command.
    // let initial_command_data = format!("{}{}{}{}{}", &path_prefix, &number_of_scheduled_amounts, &transaction_header, &transaction_kind, &to_address);

    let mut initial_command_data = path::generate_key_derivation_path();
    initial_command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    initial_command_data.append(&mut transaction_kind);
    initial_command_data.append(&mut receiver_address);
    initial_command_data.append(&mut number_of_scheduled_amounts);

    let initial_command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 3,   // Scheduled transfer
        p1: 0,
        p2: 0,
        length: 0,
        data: initial_command_data
    };
    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(initial_command).expect("Initial packet failed.");

    // Generate 17 scheduled transfers to demonstrate that splitting by 15 transfers works.
    let mut command_data: Vec<u8> = Vec::new();

    println!("The final result received contains the signature.");
    for i in 1..4 {
        let mut scheduled_timestamp = hex::decode("FFFFFFFFFFFFFFFF").unwrap();
        let mut amount = hex::decode("0000000005F5E100").unwrap();

        command_data.append(&mut scheduled_timestamp);
        command_data.append(&mut amount);

        // Send APDU 15 pairs at a time, or when done iterating.
        // Note that the packets containing the scheduled amounts must set P1 = 0x01.
        if i % 15 == 0 || i == 3 {
            println!("{}", &command_data.len());
            let command = ApduCommand {
                cla: 224,
                ins: 3,
                p1: 1,
                p2: 0,
                length: 0,
                data: command_data
            };

            let result = ledger.exchange(command).expect("Scheduled transfer command failed");
            println!("{}", hex::encode(result.data));
            command_data = Vec::new();
        }
    }
}
