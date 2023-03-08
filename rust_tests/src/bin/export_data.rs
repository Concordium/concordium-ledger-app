use ledger::{ApduCommand, LedgerApp};

use std::env;

pub fn export_data(p1: u8, p2: u8, identity: Vec<u8>) -> String {
    let ledger = LedgerApp::new().unwrap();

    let command = ApduCommand {
        cla: 224,
        ins: 7,
        p1: p1,
        p2: p2,
        length: 0,
        data: identity
    };

    let result = ledger.exchange(command).unwrap();
    return hex::encode(result.data);
}

#[allow(dead_code)]
fn main() {
    // use args to determine args
    let args: Vec<String> = env::args().collect();
    let p1: u8  = args[1].parse::<u8>().unwrap();
    let data  = hex::decode(&args[2]).unwrap();

    let result = export_data(p1, 1, data);
    println!("result: {}", result);
}
