use base58check::*;

/// Generates an account transaction header, which is required for all
/// account transactions. This serves as a helper method to build a default
/// account transaction header for testing purposes.
pub fn generate_account_transaction_header() -> Vec<u8> {
    let sender_address = "3C8N65hBwc2cNtJkGmVyGeWYxhZ6R3X77mLWTwAKsnAnyworTq";
    let mut transaction_header = sender_address.from_base58check().unwrap().1;

    let nonce = "000000000000000A";
    let energy = "0000000000000064";
    let payload_size = "00000029";
    let expiry = "0000000063DE5DA7";
    let transaction_header_blob = format!("{}{}{}{}", &nonce, &energy, &payload_size, &expiry);
    let mut transaction_header_blob_bytes = hex::decode(transaction_header_blob).unwrap();
    transaction_header.append(&mut transaction_header_blob_bytes);

    return transaction_header;
}

#[allow(dead_code)]
fn main() { }