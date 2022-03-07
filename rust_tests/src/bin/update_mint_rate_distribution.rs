mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("06").unwrap();
    let p2 = 1;

    let mut mint_rate_mantissa = hex::decode("00734B9F").unwrap();
    let mut mint_rate_exponent = hex::decode("10").unwrap();
   let mut baker_reward = hex::decode("0000EA60").unwrap();
    let mut finalization_reward = hex::decode("00007530").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    if p2 == 0 {
        command_data.append(&mut mint_rate_mantissa);
        command_data.append(&mut mint_rate_exponent);
    }
    command_data.append(&mut baker_reward);
    command_data.append(&mut finalization_reward);

    let command = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 37,    // Sign mint rate distribution update
        p1: 0,
        p2: p2,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateMintRateDistribution: {}", hex::encode(result.data));
}
