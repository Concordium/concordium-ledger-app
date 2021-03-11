mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut transaction_kind = hex::decode("32").unwrap();

    // Initial packet contains the key path, account transaction header, transaction kind
    // and finally the number of credential deployments.
    let mut credential_deployments_count = hex::decode("00").unwrap();

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_kind);
    command_data.append(&mut credential_deployments_count);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 49,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(command).unwrap();


    // Now comes all the credential deployments if there were any. For this test
    // we do not have any credential deployments yet.

    // TODO Send credential deployments (call the other rust test method? Except the final part of 
    // it if I am not mistaken).


    let credential_id_1 = hex::decode("85d8a7aa296c162e4e2f0d6bfbdc562db240e28942f7f3ddef6979a1133b5c719ec3581869aaf88388824b0f6755e63c").unwrap();
    let credential_id_2 = hex::decode("aca024ce6083d4956edad825c3721da9b61e5b3712606ba1465f7818a43849121bdb3e4d99624e9a74b9436cc8948d18").unwrap();
    let credential_id_list = [credential_id_1, credential_id_2];

    // Send number of credential ID's to remove.
    command_data = (credential_id_list.len() as u8).to_be_bytes().to_vec();
    let command = ApduCommand {
        cla: 224,
        ins: 49,
        p1: 0,
        p2: 2,
        length: command_data.len() as u8,
        data: command_data
    };
    ledger.exchange(command).unwrap();

    // Send the credential ID's one at a time.
    for credential_id in credential_id_list.iter() {
        let command = ApduCommand {
            cla: 224,
            ins: 49,
            p1: 0,
            p2: 3,
            length: credential_id.len() as u8,
            data: credential_id.to_vec()
        };
        ledger.exchange(command).unwrap();
    }

    // Send the threshold.
    let threshold = hex::decode("02").unwrap();
    let command = ApduCommand {
        cla: 224,
        ins: 49,
        p1: 0,
        p2: 4,
        length: threshold.len() as u8,
        data: threshold
    };
    let result = ledger.exchange(command).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}
