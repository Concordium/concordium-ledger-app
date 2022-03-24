mod path;
mod update_transaction_header;

use hex;
use ledger::{ApduCommand, LedgerApp};

fn main() {
    let mut key_derivation_path = path::generate_key_derivation_path();
    let mut update_type = hex::decode("0F").unwrap();

    let mut transaction_fee_l_pool = hex::decode("0000EA30").unwrap();
    let mut baking_reward_l_pool = hex::decode("00000200").unwrap();
    let mut finalization_reward_l_pool = hex::decode("0000EA04").unwrap();
    let mut transaction_fee_min = hex::decode("00001001").unwrap();
    let mut transaction_fee_max = hex::decode("0000EA64").unwrap();
    let mut baking_reward_min = hex::decode("0000EA63").unwrap();
    let mut baking_reward_max = hex::decode("0000EA65").unwrap();
    let mut finalization_reward_min = hex::decode("00001000").unwrap();
    let mut finalization_reward_max = hex::decode("0000EA62").unwrap();

    let mut min_capital = hex::decode("00000A00C60D5000").unwrap();
    let mut capital_bound = hex::decode("C60D5000").unwrap();
    let mut leverage_bound = hex::decode("00000A00C60D50000000EA600000EA60").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut key_derivation_path);
    command_data.append(&mut update_transaction_header::generate_update_transaction_header());
    command_data.append(&mut update_type);
    command_data.append(&mut transaction_fee_l_pool);
    command_data.append(&mut baking_reward_l_pool);
    command_data.append(&mut finalization_reward_l_pool);

    let mut bounds = Vec::new();
    bounds.append(&mut transaction_fee_min);
    bounds.append(&mut transaction_fee_max);
    bounds.append(&mut baking_reward_min);
    bounds.append(&mut baking_reward_max);
    bounds.append(&mut finalization_reward_min);
    bounds.append(&mut finalization_reward_max);

    min_capital.append(&mut capital_bound);
    min_capital.append(&mut leverage_bound);

    let ledger = LedgerApp::new().unwrap();

    let command_init = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 65,    // Sign time parameters update
        p1: 0,
        p2: 0,
        length: command_data.len() as u8,
        data: command_data
    };

    ledger.exchange(command_init).unwrap();

    let command_bounds = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 65,    // Sign time parameters update
        p1: 1,
        p2: 0,
        length: bounds.len() as u8,
        data: bounds
    };

    ledger.exchange(command_bounds).unwrap();

    let command_final = ApduCommand {
        cla: 224,   // Has to be this value for all commands.
        ins: 65,    // Sign time parameters update
        p1: 2,
        p2: 0,
        length: min_capital.len() as u8,
        data: min_capital
    };

    let result = ledger.exchange(command_final).unwrap();
    println!("Signature: {}", hex::encode(result.data));
}
