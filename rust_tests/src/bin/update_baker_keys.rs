mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};
use base58check::*;

fn main() {
    // Base58 part of header, so we can't just use hex and map to bytes.
    let sender_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut transaction_header = sender_address.from_base58check().unwrap().1;

    // Hex part of header.
    let nonce = "000000000000000A";
    let energy = "0000000000000064";
    let payload_size = "00000029";
    let expiry = "0000000063DE5DA7";
    let transaction_header_blob = format!("{}{}{}{}", &nonce, &energy, &payload_size, &expiry);
    let mut transaction_header_blob_bytes = hex::decode(transaction_header_blob).unwrap();

    // The full transaction header (sender_address + blob)
    transaction_header.append(&mut transaction_header_blob_bytes);

    // Transaction payload
    let transaction_kind = "08";

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut transaction_header);
    command_data.append(&mut hex::decode(transaction_kind).unwrap());

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 19,
        p1: 0,
        p2: 1,
        length: 60,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(command).unwrap();

    // Verification keys command
    let election_verify_key = hex::decode("32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828").unwrap();
    let mut baker_sign_verify_key = hex::decode("7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b").unwrap();
    let mut baker_aggregation_verify_key = hex::decode("7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082832f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828").unwrap();

    let mut verification_keys_data = election_verify_key;
    verification_keys_data.append(&mut baker_sign_verify_key);
    verification_keys_data.append(&mut baker_aggregation_verify_key);

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 19,
        p1: 1,
        p2: 1,
        length: verification_keys_data.len() as u8,
        data: verification_keys_data
    };
    ledger.exchange(command).unwrap();

    // Proofs, amount and restake command
    // The proofs are 3x64 bytes, but here we just fake it as one blob.
    let proofs = hex::decode("a47cdf9133572e9ad5c02c3a7ffd1d05db7bb98860d918092454146153d62788f224c0157c65853ed4a0245ab3e0a593a3f85fa81cc4cb99eeaa643bfc793eab01fc695a8c51d4599cbe032a39832ad49bab900d88105b01d025b760b0d0d555b8c828f2d8fe29cc78c6307d979e6358b8bba9cf4d8200f272cc85b2a3813eff957aec4b2b7ed979ba2079d62246d135aefd61e7f46690c452fec8bcbb593481e229f6f1968194a09cf612490887e71d96730e2d852201e53fec9c89d36f8a90").unwrap();
    let proofs_amount_restake_command = proofs;

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 19,
        p1: 2,
        p2: 1,
        length: proofs_amount_restake_command.len() as u8,
        data: proofs_amount_restake_command
    };
    let result = ledger.exchange(command).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}
