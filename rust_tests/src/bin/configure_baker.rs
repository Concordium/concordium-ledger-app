mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let transaction_kind = "19";
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();

    let mut bitmap = hex::decode("0100").unwrap();
    transaction_payload.append(&mut bitmap);

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_payload);

    println!("{}", hex::encode(&command_data));

    let command = ApduCommand {
        cla: 224, 
        ins: 0x18,
        p1: 0,
        p2: 0,
        length: 0,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    ledger.exchange(command).expect("Configure baker signing failed");

    let mut capital = b"".to_vec();

    // let mut capital = hex::decode("0000FFFFFFFFFFFF").unwrap();
    // let mut restake = hex::decode("01").unwrap();
    // let mut open_status = hex::decode("02").unwrap();

    // capital.append(&mut restake);
    // capital.append(&mut open_status);

    // let mut sign_verify_key = hex::decode("7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b").unwrap();
    // let mut election_verify_key = hex::decode("32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828").unwrap();
    // let mut aggregation_verify_key = hex::decode("7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082832f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828").unwrap();

    // capital.append(&mut sign_verify_key);
    // capital.append(&mut election_verify_key);
    // capital.append(&mut aggregation_verify_key);

    // println!("{}", hex::encode(&capital));

    // let command2 = ApduCommand {
    //     cla: 224, 
    //     ins: 0x18,
    //     p1: 1,
    //     p2: 0,
    //     length: 0,
    //     data: capital
    // };
    // ledger.exchange(command2).expect("Configure baker signing failed");

    // let mut url_length = hex::decode("0800").unwrap();
    // let command3 = ApduCommand {
    //     cla: 224, 
    //     ins: 0x18,
    //     p1: 2,
    //     p2: 0,
    //     length: 0,
    //     data: url_length
    // };
    // ledger.exchange(command3).expect("Configure baker signing failed");

    // for n in 1..9 {
    //     let mut url = b"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa".to_vec();
    //     let command4 = ApduCommand {
    //         cla: 224, 
    //         ins: 0x18,
    //         p1: 3,
    //         p2: 0,
    //         length: 0,
    //         data: url
    //     };
    //     ledger.exchange(command4).expect("Configure baker signing failed");
    //     println!("Sent!");
    // }

    // let mut url = b"cccccccc".to_vec();
    // println!("{}", url.len());
    // let command4 = ApduCommand {
    //     cla: 224, 
    //     ins: 0x18,
    //     p1: 3,
    //     p2: 0,
    //     length: 0,
    //     data: url
    // };

    // let result = ledger.exchange(command4).expect("Configure baker signing failed");
    // println!("Signature: {}", hex::encode(result.data));

    let mut numerator_transaction_fee = hex::decode("0000000000000001").unwrap();
    let mut denominator_transaction_fee = hex::decode("00000000000000F2").unwrap();

    let mut numerator_baking_reward = hex::decode("00000000000000F1").unwrap();
    let mut denominator_baking_reward = hex::decode("00000000000000B2").unwrap();

    let mut numerator_finalization_reward = hex::decode("00000000000D0001").unwrap();
    let mut denominator_finalization_reward = hex::decode("00000000000C00F2").unwrap();

    let mut commission_data = Vec::new();
    commission_data.append(&mut numerator_transaction_fee);
    commission_data.append(&mut denominator_transaction_fee);
    commission_data.append(&mut numerator_baking_reward);
    commission_data.append(&mut denominator_baking_reward);
    commission_data.append(&mut numerator_finalization_reward);
    commission_data.append(&mut denominator_finalization_reward);

    println!("{}", hex::encode(&commission_data));

    let command7 = ApduCommand {
        cla: 224, 
        ins: 0x18,
        p1: 4,
        p2: 0,
        length: 0,
        data: commission_data
    };
    let result = ledger.exchange(command7).expect("Configure baker signing failed");
    println!("Signature: {}", hex::encode(result.data));
}
