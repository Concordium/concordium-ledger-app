use ledger::{ApduCommand, LedgerApp};

pub fn export_data(p1: u8, p2: u8, data: Vec<u8>) -> String {
    let ledger = LedgerApp::new().unwrap();

    let command = ApduCommand {
        cla: 224,
        ins: 7,
        p1: p1,
        p2: p2,
        length: data.len() as u8,
        data
    };

    let result = ledger.exchange(command).unwrap();
    return hex::encode(result.data);
}

#[allow(dead_code)]
fn main() {
    // 00000001 coin type + 00000000 idp + 00000000 id + 00000000 cred + 0015 attribute count + 21 bytes for 21 attributes
    let data  = hex::decode("000000010000000000000000000000000015010203040506070809100102030405060708091000").unwrap();
    let result = export_data(2, 0, data);
    println!("result: {}", result);

    let data  = hex::decode("").unwrap();
    let result = export_data(3, 0, data.clone());
    println!("result: {}", result);
    let result = export_data(3, 0, data.clone());
    println!("result: {}", result);
    let result = export_data(3, 0, data.clone());
    println!("result: {}", result);
    let result = export_data(3, 0, data);
    println!("result: {}", result);
}
