mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {

    let mut id_cred_pub = hex::decode("8196e718f392ec8d07216b22b555cbb71bcee88037566d3f758b9786b945e3b614660f4bf954dbe57bc2304e5a863d2e").unwrap();
    let mut reg_id_cred = hex::decode("89a1f69196a1d0423f4936aa664da95de16f40a639dba085073c5a7c8e710c2a402136cc89a39c12ed044e1035649c0f").unwrap();

    let mut verification_keys = Vec::new();
    verification_keys.push(hex::decode("b6bc751f1abfb6440ff5cce27d7cdd1e7b0b8ec174f54de426890635b27e7daf").unwrap());
    verification_keys.push(hex::decode("46a3e38ddf8b493be6e979034510b91db5448da9cba48c106139c288d658a004").unwrap());
    verification_keys.push(hex::decode("71d5f16bc3be249043dc0f9e20b4872f5c3477bf2f285336609c5b0873ab3c9c").unwrap());

    let length = verification_keys.len() as u8;

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut id_cred_pub);
    command_data.append(&mut reg_id_cred);
    command_data.push(length);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 32,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(command).unwrap();

    for mut verification_key in verification_keys {
        println!("Sending verification key!");
        let mut key_blob = hex::decode("00").unwrap();
        key_blob.append(&mut verification_key);
        let command = ApduCommand {
            cla: 224, // Has to be this value for all commands.
            ins: 32,
            p1: 1,
            p2: 0,
            length: key_blob.len() as u8,
            data: key_blob
        };
        ledger.exchange(command).unwrap();
    }

    let threshold: u8 = 2;
    let mut command_data = Vec::new();
    command_data.push(threshold);
    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 32,
        p1: 2,
        p2: 0,
        length: 1,
        data: command_data
    };
    let result = ledger.exchange(command).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}
