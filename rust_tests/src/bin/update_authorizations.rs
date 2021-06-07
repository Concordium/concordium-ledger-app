mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("0A").unwrap();
    let mut prefix = hex::decode("02").unwrap();
    let mut public_key_list_length = hex::decode("0002").unwrap();

    let ledger = LedgerApp::new().unwrap();
    
    let mut cdata = Vec::new();
    cdata.append(&mut key_derivation_path);
    cdata.append(&mut update_transaction_header::generate_update_transaction_header());
    cdata.append(&mut update_type);
    cdata.append(&mut prefix);
    cdata.append(&mut public_key_list_length);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 0x2A,
        p1: 0,
        p2: 0,
        length: cdata.len() as u8,
        data: cdata
    };
    ledger.exchange(command).unwrap();

    // Send public-keys
    let public_key_1 = hex::decode(format!("{}{}", "00", "32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828")).unwrap();
    let public_key_2 = hex::decode(format!("{}{}", "00", "7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b")).unwrap();
    let public_key_list = [public_key_1, public_key_2];

    for public_key in public_key_list.iter() {
        let command = ApduCommand {
            cla: 224,
            ins: 0x2A,
            p1: 1,
            p2: 0,
            length: 0,
            data: public_key.clone()
        };
        ledger.exchange(command).unwrap();
    }

    for i in 0..12 {
        // Access structure
        let mut size = hex::decode("0003").unwrap();

        let mut command_data = Vec::new();
        command_data.append(&mut size);

        let command = ApduCommand {
            cla: 224, // Has to be this value for all commands.
            ins: 0x2A,   // Sign update authorizations
            p1: 2,
            p2: 0,
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
            ins: 0x2A,
            p1: 3,
            p2: 0,
            length: 0,
            data: access_structure_cdata
        };
        ledger.exchange(command).unwrap();

        let mut threshold = hex::decode("0002").unwrap();

        let mut threshold_cdata = Vec::new();
        threshold_cdata.append(&mut threshold);

        let command = ApduCommand {
            cla: 224,
            ins: 0x2A,
            p1: 4,
            p2: 0,
            length: 0,
            data: threshold_cdata
        };
        let result = ledger.exchange(command).unwrap();
        if i == 11 {
            println!("Signature: {}", hex::encode(result.data));
        }
    }
}
