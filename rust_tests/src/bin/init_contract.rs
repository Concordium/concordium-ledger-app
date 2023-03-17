mod account_transaction_header;
mod path;
use std::convert::TryFrom;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let ins = 55;
    let transaction_kind =  "01";

    let mut amount = hex::decode("0000FFFFFFFFFFFF").unwrap();
    let mut module_ref = hex::decode("69d48cea644389f65be2cd807df746abc8b97d888dc98ae531030c2a3bffeee0").unwrap();
    let name = b"myContractName".to_vec();
    let param = b"mySpicyParam".to_vec();

    // Build transaction payload bytes.
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();
    transaction_payload.append(&mut amount);
    transaction_payload.append(&mut module_ref);
    transaction_payload.append(&mut u16::try_from(name.len()).unwrap().to_be_bytes().to_vec());
    transaction_payload.append(&mut u16::try_from(param.len()).unwrap().to_be_bytes().to_vec());

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_payload);

    let ledger = LedgerApp::new().unwrap();

     let command1 = ApduCommand {
         cla: 224,
         ins,
         p1: 0,
         p2: 0,
         length: command_data.len() as u8,
         data: command_data
     };
    ledger.exchange(command1).expect("init contract, initial packet failed.");

    for name_part in name.chunks(255) {
            let name_command = ApduCommand {
                cla: 224, // Has to be this value for all commands.
                ins,
                p1: 1,
                p2: 0,
                length: name_part.len() as u8,
                data: name_part.to_vec()
            };
            ledger.exchange(name_command).expect("init contract, name packet failed.");
        }


    for param_part in param.chunks(255) {
        let param_command = ApduCommand {
            cla: 224, // Has to be this value for all commands.
            ins,
            p1: 2,
            p2: 0,
            length: param_part.len() as u8,
            data: param_part.to_vec()
        };
        let result = ledger.exchange(param_command).expect("init contract, parameter packet failed.");
        println!("Signature: {}", hex::encode(result.data));
    }
}
