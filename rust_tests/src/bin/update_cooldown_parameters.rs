mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("0E").unwrap();

    let mut delegator_cooldown = hex::decode("000075300000EA60").unwrap();
    let mut pool_owner_cooldown = hex::decode("0000000000001000").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut delegator_cooldown);
    command_data.append(&mut pool_owner_cooldown);

    let command = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 64,    // Sign time parameters update
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}
