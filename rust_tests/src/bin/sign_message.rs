mod path;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

fn main() {
    let ins = 0x38;

    let message = "Test".as_bytes().to_vec();
    let signer_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut signer_address = signer_address.from_base58check().unwrap().1;

    // Build transaction payload bytes.
    let mut initial_payload = path::generate_key_derivation_path();
    initial_payload.append(&mut signer_address);
    initial_payload.append(&mut (message.len() as u16).to_be_bytes().to_vec());

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins,
        p1: 0,
        p2: 0,
        length: initial_payload.len() as u8,
        data: initial_payload
    };

    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(command).expect("Sign message initial packet failed.");

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins,
        p1: 1,
        p2: 0,
        length: message.len() as u8,
        data: message
    };

    let result = ledger.exchange(command).expect("Sign message message packet failed.");
    println!("Signature: {}", hex::encode(result.data));

}
