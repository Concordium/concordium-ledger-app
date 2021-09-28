mod path;
mod update_transaction_header;

use hex;

use ledger::{ApduCommand, LedgerApp};

fn create_command(cdata: Vec<u8>, p1: u8) -> ApduCommand {
    ApduCommand{
        cla: 224, // Has to be this value for all commands.
        ins: 0x2C,
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
    let mut update_type = hex::decode("0C").unwrap();

    let name = "Test name";
    let name_length = name.as_bytes().len() as u32;

    let url = "http://concordium.com";
    let url_length = url.as_bytes().len() as u32;

    let description = "Test description";
    let description_length = description.as_bytes().len() as u32;

    let mut ar_identity = hex::decode("00000001").unwrap();

    let mut public_key = hex::decode("993fdc40bb8af4cb75caf8a53928d247be6285784b29578a06df312c28854c1bfac2fd0183967338b578772398d412018a4afcfaae1ba3ccd63a5b0868a8a9c49deec35a8817d35d0082761b39c0c6bd2357ef997c0f319fefd5b336e6667b7b").unwrap();

    let ar_info_length: u32 = 4 + 4 + name_length + 4 + url_length + 4 + description_length + (public_key.len() as u32);
    println!("Payload length {}", ar_info_length);

    let ledger = LedgerApp::new().unwrap();

    let mut cdata = Vec::new();
    cdata.append(&mut key_derivation_path);
    cdata.append(&mut update_transaction_header::generate_update_transaction_header());
    cdata.append(&mut update_type);
    cdata.append(&mut ar_info_length.to_be_bytes().to_vec());
    cdata.append(&mut ar_identity);

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

    // Send public key
    let mut cdata = Vec::new();
    cdata.append(&mut public_key);
    let result = ledger.exchange(create_command(cdata, 3)).unwrap();

    if !result.data.is_empty() {
        println!("Signature: {}", hex::encode(result.data));
    }
}
