mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

/// Please note that non-ASCII UTF-8 characters will not be displayed on the Ledger device.
/// Therefore it is preferred if they are restricted to the ASCII character set in the UI.
fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();

    // Hex part of update header.
    let update_sequence_number = "000000000000000A";
    let transaction_time = "0000000000000064";
    let transaction_expiry_time = "0000000063DE5DA7";
    let payload_size = "00000029";
    let update_header_blob = format!("{}{}{}{}", &update_sequence_number, &transaction_time, &transaction_expiry_time, &payload_size);
    let mut update_header_blob_bytes = hex::decode(update_header_blob).unwrap();
    let mut update_type = hex::decode("03").unwrap();

    let message_text = "This is a brief message about the update.";
    let mut message_length = message_text.as_bytes().len() as u64;

    let specification_url = "http://concordium.com/specification/v/123";
    let mut specification_url_length = message_text.as_bytes().len() as u64;

    let mut specification_hash = hex::decode("75e34b7235d488828961216c5b9fbd5b88fe74c76516eb635c57cc61632222de").unwrap();

    let auxiliary_data = b"This really should be some auxiliary data and not a String like this. This is just for testing. This message should be a lot longer, so that we actually cross 255 bytes. There is still a little bit to go to cross the 255 bytes that we need. We would like more than 255 to ensure that batching works.".to_vec();

    let payload_length: u64 = 8 + message_length + 8 + specification_url_length + (specification_hash.len() as u64) + (auxiliary_data.len() as u64);
    println!("Payload length {}", payload_length);

    let ledger = LedgerApp::new().unwrap();

    let mut cdata = Vec::new();
    cdata.append(&mut key_derivation_path);
    cdata.append(&mut update_header_blob_bytes);
    cdata.append(&mut update_type);
    cdata.append(&mut payload_length.to_be_bytes().to_vec());

    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 33,
        p1: 0,
        p2: 0,
        length: cdata.len() as u8,
        data: cdata
    };
    ledger.exchange(command).unwrap();

    // Send message length
    let mut cdata = Vec::new();
    cdata.append(&mut message_length.to_be_bytes().to_vec());
    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 33,
        p1: 1,
        p2: 0,
        length: cdata.len() as u8,
        data: cdata
    };
    ledger.exchange(command).unwrap();

    // Send message
    let mut cdata = Vec::new();
    cdata.append(&mut message_text.as_bytes().to_vec());
    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 33,
        p1: 2,
        p2: 0,
        length: cdata.len() as u8,
        data: cdata
    };
    ledger.exchange(command).unwrap();



    // Send spec url length
    let mut cdata = Vec::new();
    cdata.append(&mut specification_url_length.to_be_bytes().to_vec());
    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 33,
        p1: 1,
        p2: 0,
        length: cdata.len() as u8,
        data: cdata
    };
    ledger.exchange(command).unwrap();

    // Send spec url
    let mut cdata = Vec::new();
    cdata.append(&mut specification_url.as_bytes().to_vec());
    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 33,
        p1: 2,
        p2: 0,
        length: cdata.len() as u8,
        data: cdata
    };
    ledger.exchange(command).unwrap();

    // Send specification hash.
    let mut cdata = Vec::new();
    cdata.append(&mut specification_hash);
    let command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 33,
        p1: 3,
        p2: 0,
        length: cdata.len() as u8,
        data: cdata
    };
    ledger.exchange(command).unwrap();


    // Batch send auxiliary data
    for auxiliary_partition in auxiliary_data.chunks(255) {
        println!("Sending batch");
        let command = ApduCommand {
            cla: 224, // Has to be this value for all commands.
            ins: 33,
            p1: 4,
            p2: 0,
            length: auxiliary_partition.len() as u8,
            data: auxiliary_partition.to_vec()
        };
        let result = ledger.exchange(command).unwrap();
        if !result.data.is_empty() {
            println!("Signature: {}", hex::encode(result.data));
        }
    }
}
