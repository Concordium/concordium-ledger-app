use hex;
use ledger::{ApduCommand, LedgerApp};

// TODO Test with multiple values for the ones that can be multiple (verification keys, anonymity
// identities, ...,

fn main() {

    let mut new_account = hex::decode("01").unwrap();
    let mut verification_key_list_length = hex::decode("01").unwrap();
    let mut verification_key = hex::decode("f78929ec8a9819f6ae2e10e79522b6b311949635fecc3d924d9d1e23f8e9e1c3").unwrap();

    // Choice of identity and account (identity=0, account=0).
    let mut path_prefix = hex::decode("0000").unwrap();

    let mut command_data = Vec::new();
    command_data.append(&mut path_prefix);
    command_data.append(&mut new_account);
    command_data.append(&mut verification_key_list_length);

    // Initial command
    let initial_command = ApduCommand {
        cla: 224, // Has to be this value for all commands.
        ins: 4,   // Credential deployment
        p1: 0,
        p2: 0,
        length: 60,
        data: command_data
    };

    let ledger = LedgerApp::new().unwrap();
    let result = ledger.exchange(initial_command).expect("Credential deployment packet failed.");

    let verification_key_command = ApduCommand {
        cla: 224,
        ins: 4,
        p1: 1,
        p2: 0,
        length: 0,
        data: verification_key
    };

    ledger.exchange(verification_key_command).unwrap();

    let mut signature_threshold = hex::decode("FF").unwrap();
    let mut reg_id_cred = hex::decode("85d8a7aa296c162e4e2f0d6bfbdc562db240e28942f7f3ddef6979a1133b5c719ec3581869aaf88388824b0f6755e63c").unwrap();
    let mut identity_provider_identity = hex::decode("0000F013").unwrap();
    let mut revocation_threshold = hex::decode("0F").unwrap();

    let mut anonymity_revocation_length = hex::decode("0001").unwrap();

    signature_threshold.append(&mut reg_id_cred);
    signature_threshold.append(&mut identity_provider_identity);
    signature_threshold.append(&mut revocation_threshold);
    signature_threshold.append(&mut anonymity_revocation_length);

    let command = ApduCommand {
        cla: 224,
        ins: 4,
        p1: 2,
        p2: 0,
        length: 0,
        data: signature_threshold
    };
    ledger.exchange(command).unwrap();


    let mut ar_identity = hex::decode("000F0301").unwrap();
    let mut enc_id_cred_pub_share = hex::decode("aca024ce6083d4956edad825c3721da9b61e5b3712606ba1465f7818a43849121bdb3e4d99624e9a74b9436cc8948d178b9b144122aa070372e3fadee4998e1cc21161186a3d19698ad245e10912810df1aaddda16a27f654716108e27758099").unwrap();
    ar_identity.append(&mut enc_id_cred_pub_share);

    let ar_identity_command = ApduCommand {
        cla: 224,
        ins: 4,
        p1: 3,
        p2: 0,
        length: 0,
        data: ar_identity
    };
    ledger.exchange(ar_identity_command).expect("Ar identity command failed somehow.");


    // Send command with valid to and created at
    let mut valid_to = hex::decode("07E40B").unwrap();
    let mut created_at = hex::decode("07E10C").unwrap();
    valid_to.append(&mut created_at);

    let dates_command = ApduCommand {
        cla: 224,
        ins: 4,
        p1: 4,
        p2: 0,
        length: 0,
        data: valid_to
    };
    ledger.exchange(dates_command).unwrap();

    // Attribute list commands
    let mut attribute_tag = hex::decode("01").unwrap();

    let mut attribute_value = b"John".to_vec();
    let mut attribute_length = attribute_value.len().to_be_bytes().to_vec();
    attribute_tag.append(&mut attribute_length);

    let attribute_command = ApduCommand {
        cla: 224,
        ins: 4,
        p1: 5,
        p2: 0,
        length: 0,
        data: attribute_tag
    };
    ledger.exchange(attribute_command).unwrap();


    // TODO How to display UTF-8 Strings on the device? ... :(
    let attribute_value_command = ApduCommand {
        cla: 224,
        ins: 4,
        p1: 6,
        p2: 0,
        length: 0,
        data: attribute_value
    };
    ledger.exchange(attribute_value_command).unwrap();
}
