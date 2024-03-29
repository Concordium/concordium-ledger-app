use hex;

pub fn generate_key_derivation_path() -> Vec<u8> {
    let mut key_derivation_path = build_key_derivation_path();
    let key_path_length: u8 = (key_derivation_path.len() / 4) as u8;

    let mut result = Vec::new();
    result.push(key_path_length);
    result.append(&mut key_derivation_path);

    return result;
}

fn build_key_derivation_path() -> Vec<u8> {
    // Purpose = 1105.
    let mut purpose = hex::decode("00000451").unwrap();

    // Coin type = 0.
    let mut coin_type = hex::decode("00000000").unwrap();

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

#[allow(dead_code)]
fn main() { }
