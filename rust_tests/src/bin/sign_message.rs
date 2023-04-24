#[path = "path.rs"] mod path;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

pub fn sign_message(message: Vec<u8>, p2: u8) {
    let ins = 0x38;
    let signer_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";

    let mut signer_address = signer_address.from_base58check().unwrap().1;
    println!("{}", message.len());

    // Build transaction payload bytes.
    let mut initial_payload = path::generate_key_derivation_path();
    initial_payload.append(&mut signer_address);
    initial_payload.append(&mut (message.len() as u16).to_be_bytes().to_vec());

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins,
        p1: 0,
        p2,
        length: initial_payload.len() as u8,
        data: initial_payload
    };

    let ledger = LedgerApp::new().unwrap();
    let mut result = ledger.exchange(command).expect("Sign message initial packet failed.");

    for message_part in message.chunks(255) {
        let message_command = ApduCommand {
            cla: 224, // Has to be this value for all commands.
            ins,
            p1: 1,
            p2: 0,
            length: message_part.len() as u8,
            data: message_part.to_vec()
        };
        result = ledger.exchange(message_command).expect("Sign message message packet failed.");
    }
    println!("{}", hex::encode(result.data));
}

#[allow(dead_code)]
fn main() {
    let message = "057366ddef5d7edefe03fef5743012b995b5e3bcdae130b4e01b396853b65bb9b687bde11540c7dc5ed6e237de5a604b8f75b72a58d5f69f66abb5e52efd8c5d8381eaea64650ac1ecae6cca898e9677a061f8cb7e0501787ac6c659719f80d3d22d38d8f5aab812bd8013490230c8ce3781a14f8be3371111fkrmrkfkdmn12".as_bytes().to_vec();
    let p2 = 0;
    sign_message(message, p2)
}
