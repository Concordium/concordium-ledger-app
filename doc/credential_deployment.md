# Credential deployment transaction

NOTE: Only supports new account presently.

A transaction used to deploy credentials for a user.

## Credential deployment (new account)

The following is the description for the new account transaction. Later we will add documentation for the initial
account transaction, which is quite similar.

| Number of bytes                    | Description | Display |
|------------------------------------|-------------|---------|
| `1`  | 0 for existing account, 1 for new account | No |
| `1`  | Describes length of account verification key list | No |
| `32 * length` | Account verification key(s) | Yes |
| `1`  | Signature threshold | Yes | 
| `48` | RegIdCred, registration id? | Yes |
| `4`  | Identity provider identity | Yes |
| `1`  | Anonymity revocation threshold | Yes | 
| `2`  | Length of anonymity revocation data | No |
| `(4 + 96)*` | ArIdentity and encIdCredPubShare | Yes |
| `3`  | Credential valid to (2 first bytes is the year, last byte is the month) | Yes |
| `3`  | Credential created at (2 first bytes is the year, last byte is the month) | Yes |
| `2 + (1 + [1..255])*` | 2 bytes for number of attributes, then attribute tag and attribute value list | Yes (SHA256 of it) |
| `4`  | Length of proofs | No | 
| `?`  | Proofs | No  |

Does it make sense to display the hash of a value? The user can verify that it is correct by hashing their data
themselves, but that is quite difficult and probably not something a user would do.

## Protocol description

The following sequence of bytes must be sent in the specified sequence, and with the given P1 value. Some of the commands
should be repeated until all data has been sent.

* Multiple commands.

| P1 | CDATA | Comment |
|--------|-------------|----|
| `0x00` | `identity account new/existing_account verification_key_length` | |
| `0x01` | `verification_key` | One verification key per command. |
| `0x02` | `signature_threshold reg_id_cred identity_provider_identity ar_threshold ar_list_length` | |
| `0x03` | `ar_identity enc_id_cred_pub_share` | One pair of values per command. |
| `0x04` | `valid_to create_at attribute_list_length` | |
| `0x05` | `attribute_tag attribute_value_length` | One pair of values per command. After one command move to next line. |
| `0x06` | `attribute_value` | One value per command. If more attributes are available goto `0x05` step |
| `0x07` | `length_of_proofs` | | 
| `0x08` | `proofs` | Maximum 255 bytes of the proofs per command, until the whole value has been sent | 
