use ledger::{ApduCommand, LedgerApp};

fn main() {
    let ledger = LedgerApp::new().unwrap();
    // [Identity, credCounter]
    let data = hex::decode(format!("{}{}", "00000000", "00000000")).unwrap();

    let command = ApduCommand {
        cla: 224,
        ins: 0,
        p1: 0,
        p2: 0,
        length: 0,
        data: data.clone()
    };

    ledger.exchange(command).unwrap();
}
