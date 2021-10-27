use ledger::{ApduCommand, LedgerApp};

fn main() {
    let ledger = LedgerApp::new().unwrap();
    let data = hex::decode(format!("0800000451000000000000000000000000{}00000002{}00000000", "00000000", "00000004")).unwrap();

    let command = ApduCommand {
        cla: 224,
        ins: 54,
        p1: 0,
        p2: 0,
        length: 0,
        data: data.clone()
    };

    ledger.exchange(command).unwrap();
}
