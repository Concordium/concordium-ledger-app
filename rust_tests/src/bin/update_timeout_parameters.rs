mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("12").unwrap();

    let mut base = hex::decode("00000F0000000001").unwrap();
    let mut increase_numerator = hex::decode("0000000000000001").unwrap();
    let mut increase_denominator = hex::decode("00000000000000F2").unwrap();

    let mut decrease_numerator = hex::decode("000000000000C001").unwrap();
    let mut decrease_denominator = hex::decode("000000000C0000F2").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);

    command_data.append(&mut base);
    command_data.append(&mut increase_numerator);
    command_data.append(&mut increase_denominator);
    command_data.append(&mut decrease_numerator);
    command_data.append(&mut decrease_denominator);

    println!("{}", hex::encode(&command_data));

    let command = ApduCommand {
        cla: 224,
        ins: 67,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateTimeoutParameters: {}", hex::encode(result.data));
}
