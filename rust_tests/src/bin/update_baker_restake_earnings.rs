mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let transaction_kind = "07";

    let mut restake_earnings = hex::decode("01").unwrap();

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut hex::decode(transaction_kind).unwrap());
    command_data.append(&mut restake_earnings);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 22,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}
