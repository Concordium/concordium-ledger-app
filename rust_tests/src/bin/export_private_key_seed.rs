use ledger::{ApduCommand, LedgerApp};

fn main() {
    let ledger = LedgerApp::new().unwrap();

    let identity = hex::decode("00000007").unwrap();
    let id_cred_sec_command = ApduCommand {
        cla: 224,
        ins: 5,
        p1: 0,
        p2: 0,
        length: 0,
        data: identity.clone()
    };

    let result = ledger.exchange(id_cred_sec_command).unwrap();
    let id_cred_sec_key_seed = hex::encode(result.data);
    println!("id_cred_sec private-key (hex): {}", id_cred_sec_key_seed);

    let prf_key_command = ApduCommand {
        cla: 224,
        ins: 5,
        p1: 1,
        p2: 0,
        length: 0,
        data: identity
    };

    let result = ledger.exchange(prf_key_command).unwrap();
    let prf_key_seed = hex::encode(result.data);
    println!("prf_key private-key (hex): {}", prf_key_seed);

    let mut idp = hex::decode("00000008").unwrap();
    let mut ar_index = hex::decode("00FC0010").unwrap();
    let mut data = Vec::new();
    data.append(&mut idp);
    data.append(&mut ar_index);

    let ar_decryption_key_command = ApduCommand {
        cla: 224,
        ins: 5,
        p1: 2,
        p2: 0,
        length: 0,
        data
    };

    let result = ledger.exchange(ar_decryption_key_command).unwrap();
    let ar_decryption_key_seed = hex::encode(result.data);
    println!("ar_decryption_key_seed (hex): {}", ar_decryption_key_seed);
}