mod account_transaction_header;
mod path;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let transaction_kind = "19";
    let mut transaction_payload = hex::decode(transaction_kind).unwrap();

    let mut bitmap = hex::decode("00FF").unwrap();
    transaction_payload.append(&mut bitmap);

    let mut command_data = path::generate_key_derivation_path();
    command_data.append(&mut account_transaction_header::generate_account_transaction_header());
    command_data.append(&mut transaction_payload);

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

    let mut capital = hex::decode("0000FFFFFFFFFFFF").unwrap();
    let mut restake = hex::decode("01").unwrap();
    let mut open_status = hex::decode("02").unwrap();

    capital.append(&mut restake);
    capital.append(&mut open_status);

    let mut sign_verify_key = hex::decode("7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b").unwrap();
    let mut sign_verify_key_proof = hex::decode("a47cdf9133572e9ad5c02c3a7ffd1d05db7bb98860d918092454146153d62788f224c0157c65853ed4a0245ab3e0a593a3f85fa81cc4cb99eeaa643bfc793eab").unwrap();
    let mut election_verify_key = hex::decode("32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828").unwrap();
    let mut election_verify_key_proof = hex::decode("01fc695a8c51d4599cbe032a39832ad49bab900d88105b01d025b760b0d0d555b8c828f2d8fe29cc78c6307d979e6358b8bba9cf4d8200f272cc85b2a3813eff").unwrap();
    capital.append(&mut sign_verify_key);
    capital.append(&mut sign_verify_key_proof);
    capital.append(&mut election_verify_key);
    capital.append(&mut election_verify_key_proof);

     let command1 = ApduCommand {
         cla: 224,
         ins: 0x18,
         p1: 1,
         p2: 0,
         length: capital.len() as u8,
         data: capital
     };
    ledger.exchange(command1).expect("Configure baker signing failed");

    let mut aggregation_verify_key = hex::decode("7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082832f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828").unwrap();
    let mut aggregation_verify_key_proof = hex::decode("957aec4b2b7ed979ba2079d62246d135aefd61e7f46690c452fec8bcbb593481e229f6f1968194a09cf612490887e71d96730e2d852201e53fec9c89d36f8a90").unwrap();

    aggregation_verify_key.append(&mut aggregation_verify_key_proof);

    let command2 = ApduCommand {
        cla: 224,
        ins: 0x18,
        p1: 2,
        p2: 0,
        length: aggregation_verify_key.len() as u8,
        data: aggregation_verify_key
    };
    ledger.exchange(command2).expect("Configure baker signing failed");

    let url_length = hex::decode("0008").unwrap();
    let command3 = ApduCommand {
        cla: 224,
        ins: 0x18,
        p1: 3,
        p2: 0,
        length: 0,
        data: url_length
    };
    ledger.exchange(command3).expect("Configure baker signing failed");

    let url = b"cccccccc".to_vec();
    let command4 = ApduCommand {
        cla: 224,
        ins: 0x18,
        p1: 4,
        p2: 0,
        length: url.len() as u8,
        data: url
    };
    ledger.exchange(command4).expect("Configure baker signing failed");

    let mut transaction_fee = hex::decode("00000001").unwrap();
    let mut baking_reward = hex::decode("000000F1").unwrap();
    let mut finalization_reward = hex::decode("0000D001").unwrap();

    let mut commission_data = Vec::new();
    commission_data.append(&mut transaction_fee);
    commission_data.append(&mut baking_reward);
    commission_data.append(&mut finalization_reward);

    let command7 = ApduCommand {
        cla: 224,
        ins: 0x18,
        p1: 5,
        p2: 0,
        length: commission_data.len() as u8,
        data: commission_data
    };
    let result = ledger.exchange(command7).expect("Configure baker signing failed");
    println!("Signature: {}", hex::encode(result.data));
}
