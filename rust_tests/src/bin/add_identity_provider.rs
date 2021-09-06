mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};
use sha2::{Sha256, Digest};

fn create_command(cdata: Vec<u8>, p1: u8) -> ApduCommand {
    ApduCommand{
        cla: 224, // Has to be this value for all commands.
        ins: 0x2D,
        p1: p1,
        p2: 0,
        length: cdata.len() as u8,
        data: cdata
    }
}

/// Please note that non-ASCII UTF-8 characters will not be displayed on the Ledger device.
/// Therefore it is preferred if they are restricted to the ASCII character set in the UI.
fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("0D").unwrap();

    let name = "Test name";
    let name_length = name.as_bytes().len() as u32;

    let url = "http://concordium.com";
    let url_length = url.as_bytes().len() as u32;

    let description = "Test description";
    let description_length = description.as_bytes().len() as u32;

    let mut ip_identity = hex::decode("00000001").unwrap();

    let verify_key = hex::decode("00000001").unwrap();
    let mut cdi_verify_key = hex::decode("37efcc5b9180fc9c43a5a51a2f27d6581e63e4b2b3dad75b8510061b8c2db39f").unwrap();

    let ip_info_length: u32 = 4 + 4 + name_length + 4 + url_length + 4 + description_length + (verify_key.len() as u32) + (cdi_verify_key.len() as u32);
    println!("Payload length {}", ip_info_length);

    let ledger = LedgerApp::new().unwrap();

    let mut cdata = Vec::new();
    cdata.append(&mut key_derivation_path);
    cdata.append(&mut update_transaction_header::generate_update_transaction_header());
    cdata.append(&mut update_type);
    cdata.append(&mut ip_info_length.to_be_bytes().to_vec());
    cdata.append(&mut ip_identity);

    ledger.exchange(create_command(cdata, 0)).unwrap();

    // Send name length
    let mut cdata = Vec::new();
    cdata.append(&mut name_length.to_be_bytes().to_vec());
    ledger.exchange(create_command(cdata, 1)).unwrap();

    // Send name
    let mut cdata = Vec::new();
    cdata.append(&mut name.as_bytes().to_vec());
    ledger.exchange(create_command(cdata, 2)).unwrap();

    // Send url length
    let mut cdata = Vec::new();
    cdata.append(&mut url_length.to_be_bytes().to_vec());
    ledger.exchange(create_command(cdata, 1)).unwrap();

    // Send url
    let mut cdata = Vec::new();
    cdata.append(&mut url.as_bytes().to_vec());
    ledger.exchange(create_command(cdata, 2)).unwrap();

    // Send description length
    let mut cdata = Vec::new();
    cdata.append(&mut description_length.to_be_bytes().to_vec());
    ledger.exchange(create_command(cdata, 1)).unwrap();

    // Send description
    let mut cdata = Vec::new();
    cdata.append(&mut description.as_bytes().to_vec());
    ledger.exchange(create_command(cdata, 2)).unwrap();

    // create a Sha256 object
    let mut hasher = Sha256::new();
    hasher.update(&verify_key);
    let hash = hasher.finalize();
    println!("Verify key Hash {}", hex::encode(hash));

    // Batch send verify key
    for chunk in verify_key.chunks(255) {
        ledger.exchange(create_command(chunk.to_vec(), 3)).unwrap();
    }

    // Send cdi verify key
    let mut cdata = Vec::new();
    cdata.append(&mut cdi_verify_key);
    let result = ledger.exchange(create_command(cdata, 4)).unwrap();

    if !result.data.is_empty() {
        println!("Signature: {}", hex::encode(result.data));
    }
}
