mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("10").unwrap();

    let mut reward_period_length = hex::decode("000075300000EA60").unwrap();
    let mut mint_rate_mantissa = hex::decode("00734B9F").unwrap();
    let mut mint_rate_exponent = hex::decode("09").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut reward_period_length);
    command_data.append(&mut mint_rate_mantissa);
    command_data.append(&mut mint_rate_exponent);

    let command = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 66,    // Sign time parameters update
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}
