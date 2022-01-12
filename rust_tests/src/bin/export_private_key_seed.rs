mod export_private_key;

fn main() {
    let identity = hex::decode("FFFFFFFF").unwrap();

    println!("prf_key seed (hex): {}", export_private_key::export_private_key(0, 1, identity.clone()));
    println!("prf_key seed (hex): {}", export_private_key::export_private_key(1, 1, identity.clone()));

    let result = export_private_key::export_private_key(2, 1, identity.clone());
    let prf_seed = &result[..64];
    println!("prf_key seed (hex): {}", prf_seed);
    let id_cred_seed = &result[64..128];
    println!("id_cred seed (hex): {}", id_cred_seed);
}
