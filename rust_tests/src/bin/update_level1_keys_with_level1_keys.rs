mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("0B").unwrap();
    let mut level1_key_update_type = hex::decode("00").unwrap();
    let mut number_of_update_keys = hex::decode("0002").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut level1_key_update_type);
    command_data.append(&mut number_of_update_keys);

    let command = ApduCommand {
        cla: 224,
        ins: 0x29,
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(command).unwrap();

    // All the verification keys are pre-fixed by their type, 0 means ed25519.
    let mut verification_keys = Vec::new();
    verification_keys.push(hex::decode(format!("{}{}", "00", "b6bc751f1abfb6440ff5cce27d7cdd1e7b0b8ec174f54de426890635b27e7daf")).unwrap());
    verification_keys.push(hex::decode(format!("{}{}", "00", "46a3e38ddf8b493be6e979034510b91db5448da9cba48c106139c288d658a004")).unwrap());

    for verification_key in verification_keys {
        let command = ApduCommand {
            cla: 224,
            ins: 0x29,
            p1: 1,
            p2: 0,
            length: verification_key.len() as u8,
            data: verification_key
        };
        ledger.exchange(command).unwrap();
    }

    let threshold = hex::decode("0002").unwrap();
    let command = ApduCommand {
        cla: 224,
        ins: 0x29,
        p1: 2,
        p2: 0,
        length: threshold.len() as u8,
        data: threshold
    };

    let result = ledger.exchange(command).unwrap();
     println!("Signature for UpdateLevel1KeysWithLevel1Keys: {}", hex::encode(result.data));
}
