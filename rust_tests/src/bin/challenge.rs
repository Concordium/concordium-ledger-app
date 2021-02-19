mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {

    let mut command_data = path::generate_key_derivation_path();

    let mut challenge = hex::decode("f78929ec8a9819f6ae2e10e79522b6b311949635fecc3d924d9d1e23f8e9e1c3").unwrap();
    command_data.append(&mut challenge);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 48,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}