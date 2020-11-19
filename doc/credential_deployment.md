# Credential deployment transaction

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
| `2 + (1 + [1..255])*` | 2 bytes for number of attributes, then attribute tag and attribute value list | Yes (SHA256 of all of it?) |
| `4`  | Length of proofs | No | 
| `?`  | Proofs | No  |