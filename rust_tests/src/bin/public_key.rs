mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {

    let mut key_derivation_path = path::generate_key_derivation_path();
    println!("{}", key_path_length);

    let mut command_data = Vec::new();
    command_data.push(key_path_length);
    command_data.append(&mut key_derivation_path);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 1,   // Transfer (Get public-key)
        p1: 0,
        p2: 0,
        length: 0,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).expect("Public-key packet failed.");
    println!("Public-key: {}", hex::encode(result.data));
}

