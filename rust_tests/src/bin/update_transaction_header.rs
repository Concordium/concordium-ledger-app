
/// Generates an update transaction header, which is required for all
/// chain parameter update transactions. This serves as a helper method to build a default
/// header for testing purposes.
pub fn generate_update_transaction_header() -> Vec<u8> {
    let update_sequence_number = "000000000000000A";
    let transaction_time = "0000000000000064";
    let transaction_expiry_time = "0000000063DE5DA7";
    let payload_size = "00000029";
    let update_header_blob = format!("{}{}{}{}", &update_sequence_number, &transaction_time, &transaction_expiry_time, &payload_size);
    let update_header_blob_bytes = hex::decode(update_header_blob).unwrap();

    return update_header_blob_bytes;
}

#[allow(dead_code)]
fn main() { }