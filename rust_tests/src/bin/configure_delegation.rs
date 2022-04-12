mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let transaction_kind = "1A";
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();

    let mut bitmap = hex::decode("0003").unwrap();
    transaction_payload.append(&mut bitmap);

    let mut capital = hex::decode("0000FFFFFFFFFFFF").unwrap();
    transaction_payload.append(&mut capital);

    let mut restake = hex::decode("01").unwrap();
    transaction_payload.append(&mut restake);

    let mut delegation_type = hex::decode("01").unwrap();
    transaction_payload.append(&mut delegation_type);

    let mut baker_id =  hex::decode("00000000ABCDEFFF").unwrap();
    transaction_payload.append(&mut baker_id);

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_payload);

    println!("{}", hex::encode(&command_data));

    let command = ApduCommand {
        cla: 224, 
        ins: 0x17,
        p1: 0,
        p2: 0,
        length: 0,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).expect("Configure delegation signing failed");
    println!("Signature: {}", hex::encode(result.data));
}
