use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {

    let mut key_derivation_path = build_key_derivation_path();
    let key_path_length: u8 = (key_derivation_path.len() / 4) as u8;
    println!("{}", key_path_length);

    let mut command_data = Vec::new();
    command_data.push(key_path_length);
    command_data.append(&mut key_derivation_path);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 1,   // Transfer (Get public-key)
        p1: 0,
        p2: 0,
        length: 0,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(command).expect("Public-key packet failed.");
    println!("Public-key: {}", hex::encode(result.data));
}

fn build_key_derivation_path() -> Vec<u8> {
    // Purpose = 583.
    let mut purpose = hex::decode("00000247").unwrap();

    // Coin type = 691.
    let mut coin_type = hex::decode("000002B3").unwrap();

    // Subtree
    let mut account_subtree = hex::decode("00000000").unwrap();

    // Normal accounts
    let mut account_type = hex::decode("00000000").unwrap();

    // Identity
    let mut identity = hex::decode("00000000").unwrap();

    // Accounts
    let mut accounts = hex::decode("00000002").unwrap();

    // Account index
    let mut account_index = hex::decode("00000000").unwrap();

    // Signature index
    let mut sig_index = hex::decode("00000000").unwrap();

    let mut key_derivation_path = Vec::new();
    key_derivation_path.append(&mut purpose);
    key_derivation_path.append(&mut coin_type);
    key_derivation_path.append(&mut account_subtree);
    key_derivation_path.append(&mut account_type);
    key_derivation_path.append(&mut identity);
    key_derivation_path.append(&mut accounts);
    key_derivation_path.append(&mut account_index);
    key_derivation_path.append(&mut sig_index);

    return key_derivation_path;
}