mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let ins_register_data = 53;
    let transaction_kind =  "15";

    let data  = hex::decode("6474657374").unwrap();

    // Build transaction payload bytes.
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();
    transaction_payload.append(&mut (data.len() as u16).to_be_bytes().to_vec());

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_payload);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: ins_register_data,
        p1: 0,
        p2: 0,
        length: 60,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(command).expect("register data, initial packet failed.");

        for data_part in data.chunks(255) {
            let data_command = ApduCommand {
                cla: 224, // Has to be this value for all commands.
                ins: ins_register_data,
                p1: 1,
                p2: 0,
                length: data_part.len() as u8,
                data: data_part.to_vec()
            };
            let result = ledger.exchange(data_command).expect("register data, data packet failed.");
            println!("{}", hex::encode(result.data));
        }
}
