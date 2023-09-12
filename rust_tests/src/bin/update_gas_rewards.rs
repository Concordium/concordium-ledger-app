mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("15").unwrap();

    let mut baker_gas = hex::decode("000061A8").unwrap();
    let mut account_creation_gas = hex::decode("000000C8").unwrap();
    let mut chain_update_gas = hex::decode("00000190").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut baker_gas);
    command_data.append(&mut account_creation_gas);
    command_data.append(&mut chain_update_gas);

    println!("{}", hex::encode(&command_data));

    let command = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 35,    // Sign GAS rewards update
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature for UpdateGasRewards: {}", hex::encode(result.data));
}
