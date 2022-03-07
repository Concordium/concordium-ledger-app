mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("0B").unwrap();
    let mut prefix = hex::decode("01").unwrap();
    let mut public_key_list_length = hex::decode("0002").unwrap();
    let p2 = 1;
    let ins = 0x2b;

    let ledger = LedgerApp::new().unwrap();

    let mut cdata = Vec::new();
    cdata.append(&mut key_derivation_path);
    cdata.append(&mut update_transaction_header::generate_update_transaction_header());
    cdata.append(&mut update_type);
    cdata.append(&mut prefix);
    cdata.append(&mut public_key_list_length);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: ins,
        p1: 0,
        p2: p2,
        length: cdata.len() as u8,
        data: cdata
    };
    ledger.exchange(command).unwrap();

    // Send public-keys
    let public_key_1 = hex::decode(format!("{}{}", "00", "b6bc751f1abfb6440ff5cce27d7cdd1e7b0b8ec174f54de426890635b27e7daf")).unwrap();
    let public_key_2 = hex::decode(format!("{}{}", "00", "46a3e38ddf8b493be6e979034510b91db5448da9cba48c106139c288d658a004")).unwrap();
    let public_key_list = [public_key_1, public_key_2];

    for public_key in public_key_list.iter() {
        let command = ApduCommand {
            cla: 224,
            ins: ins,
            p1: 1,
            p2: p2,
            length: 0,
            data: public_key.clone()
        };
        ledger.exchange(command).unwrap();
    }
    let mut result = None;

    let structure_count = if p2 == 0 {
        12
    } else {
        14
    };

    for _ in 0..structure_count {
        // Access structure
        let mut size = hex::decode("0003").unwrap();

        let mut command_data = Vec::new();
        command_data.append(&mut size);

        let command = ApduCommand {
            cla: 224, // Has to be this value for all commands.
            ins: ins,   // Sign update authorizations
            p1: 2,
            p2: p2,
            length: command_data.len() as u8,
            data: command_data
        };
        ledger.exchange(command).unwrap();

        // TODO: We should test one with > 127 values...
        let mut key_index_list1 = hex::decode("0001").unwrap();
        let mut key_index_list2 = hex::decode("00F1").unwrap();
        let mut key_index_list3 = hex::decode("000C").unwrap();

        let mut access_structure_cdata = Vec::new();
        access_structure_cdata.append(&mut key_index_list1);
        access_structure_cdata.append(&mut key_index_list2);
        access_structure_cdata.append(&mut key_index_list3);

        let command = ApduCommand {
            cla: 224,
            ins: ins,
            p1: 3,
            p2: p2,
            length: access_structure_cdata.len() as u8,
            data: access_structure_cdata
        };
        ledger.exchange(command).unwrap();

        let mut threshold = hex::decode("0002").unwrap();
        let mut threshold_cdata = Vec::new();
        threshold_cdata.append(&mut threshold);

        let command = ApduCommand {
            cla: 224,
            ins: ins,
            p1: 4,
            p2: p2,
            length: threshold_cdata.len() as u8,
            data: threshold_cdata
        };
        result = Some(ledger.exchange(command).unwrap());
    }
    if let Some(r) = result {
        println!("Signature: {}", hex::encode(r.data));
    }
}
