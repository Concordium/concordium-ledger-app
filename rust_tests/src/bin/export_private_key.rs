use ledger::{ApduCommand, LedgerApp};

fn main() {
    let ledger = LedgerApp::new().unwrap();

    let identity = hex::decode("FFFFFFFF").unwrap();

    let prf_command = ApduCommand {
        cla: 224,
        ins: 5,
        p1: 0,
        p2: 2,
        length: 0,
        data: identity.clone()
    };

    let result = ledger.exchange(prf_command).unwrap();
    let prf_key_seed = hex::encode(result.data);
    println!("prf_key seed (hex): {}", prf_key_seed);

    let prf_recovery_command = ApduCommand {
        cla: 224,
        ins: 5,
        p1: 1,
        p2: 2,
        length: 0,
        data: identity.clone()
    };

    let result = ledger.exchange(prf_recovery_command).unwrap();
    let prf_key_seed = hex::encode(result.data);
    println!("prf_key seed (hex): {}", prf_key_seed);

    let both_command = ApduCommand {
        cla: 224,
        ins: 5,
        p1: 2,
        p2: 2,
        length: 0,
        data: identity
    };

    let result = ledger.exchange(both_command).unwrap();
    let prf_seed = &result.data[..32];
    println!("prf_key seed (hex): {}", hex::encode(prf_seed));
    let id_cred_seed = &result.data[32..64];
    println!("id_cred seed (hex): {}", hex::encode(id_cred_seed));
}
