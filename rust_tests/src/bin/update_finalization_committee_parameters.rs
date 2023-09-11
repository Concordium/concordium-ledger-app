mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("16").unwrap();
    
    let mut min_finalizers = hex::decode("00000003").unwrap();
    let mut max_finalizers = hex::decode("00000005").unwrap();
    let mut relative_stake_threshold_numerator = hex::decode("00008005").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut min_finalizers);
    command_data.append(&mut max_finalizers);
    command_data.append(&mut relative_stake_threshold_numerator);


    println!("{}", hex::encode(&command_data));

    let command = ApduCommand {
        cla: 224,
        ins: 70,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateFinalizationCommitteeParameters: {}", hex::encode(result.data));
}
