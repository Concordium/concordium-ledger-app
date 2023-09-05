mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("09").unwrap();
    // Threshold is 10998439038976 in decimal (which will be shown on the device).
    let mut baker_threshold = hex::decode("00000A00C60D5000").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut baker_threshold);

    let command = ApduCommand {
        cla: 224,
        ins: 39,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateBakerStakeThreshold: {}", hex::encode(result.data));
}
