#[path = "path.rs"] mod path;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

pub fn sign_message(message: Vec<u8>, p2: u8) {
    let ins = 0x38;
    let signer_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";

    let mut signer_address = signer_address.from_base58check().unwrap().1;
    println!("{}", message.len());

    // Build transaction payload bytes.
    let mut initial_payload = path::generate_testnet_key_derivation_path();
    initial_payload.append(&mut signer_address);
    initial_payload.append(&mut (message.len() as u16).to_be_bytes().to_vec());
    println!("{:?}", initial_payload);

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
    let message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam turpis magna, ultricies elementum suscipit sed, accumsan ut ex. Phasellus non tempus erat. Praesent fermentum turpis vel arcu tempus placerat. Aenean sed elit et erat vulputate aliquet. Nunc eu ultrices tortor, ut dignissim nisl. Nunc congue urna non efficitur laoreet. Aenean sit amet augue id purus consequat molestie. Quisque aliquet purus id enim auctor, non aliquet justo cursus. Donec consequat, nibh rutrum varius porta, urna mi.".as_bytes().to_vec();
    let p2 = 0;
    sign_message(message, p2)
}
