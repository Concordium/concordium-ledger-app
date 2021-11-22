use ledger::{ApduCommand, LedgerApp};

pub fn export_private_key(p1: u8, p2: u8, identity: Vec<u8>) -> String {
    let ledger = LedgerApp::new().unwrap();

    let command = ApduCommand {
        cla: 224,
        ins: 5,
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
    let identity = hex::decode("FFFFFFFF").unwrap();

    println!("prf_key (hex): {}", export_private_key(0, 2, identity.clone()));
    println!("prf_key (hex): {}", export_private_key(1, 2, identity.clone()));

    let result = export_private_key(2, 2, identity.clone());
    let prf_key = &result[..64];
    println!("prf_key (hex): {}", prf_key);
    let id_cred_sec = &result[64..128];
    println!("id_cred (hex): {}", id_cred_sec);
}
