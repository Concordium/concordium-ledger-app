from py_ecc.bls12_381 import G1, multiply, neg, curve_order
from typing import Tuple
import hmac
import pytest
import hashlib
import base58
from bip32 import BIP32, HARDENED_INDEX
from bip_utils import Bip32Slip10Ed25519
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives import serialization
from application_client.boilerplate_command_sender import BoilerplateCommandSender, Errors
from application_client.boilerplate_response_unpacker import unpack_get_public_key_response
from ragger.bip import calculate_public_key_and_chaincode, CurveChoice
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns


def hkdf_expand(prk: bytes, info: bytes, length: int) -> bytes:
    """HKDF-Expand as per RFC 5869 Section 2.3."""
    hash_len = hashlib.sha256().digest_size
    n = (length + hash_len - 1) // hash_len
    if n > 255:
        raise Exception("Cannot expand to more than 255 * HashLen bytes using HKDF")
    okm = b''
    t = b''
    for i in range(1, n + 1):
        t = hmac.new(prk, t + info + bytes([i]), hashlib.sha256).digest()
        okm += t
    return okm[:length]

def int_to_bytes(x: int, length: int) -> bytes:
    return x.to_bytes(length, 'big')

def bytes_to_int(b: bytes) -> int:
    return int.from_bytes(b, 'big')

def compress_g1(point: Tuple[int, int, int]) -> bytes:
    """Compress a G1 point to 48 bytes"""
    x, y, _ = point
    flag = 0x80  # Set compression flag
    if y > curve_order - y:  # if y > -y
        flag |= 0x20
    return int_to_bytes((x | (flag << (8 * 48))) % curve_order, 48)

# TODO: INSPECT IF THIS WORKS (come from generative model)
def get_cred_id(bls_private_key: bytes, cred_counter: int) -> bytes:
    """
    Calculate the credential ID from the PRF key and credential counter.
    
    Args:
        bls_private_key: 32-byte private key
        cred_counter: credential counter as integer
    
    Returns:
        48-byte compressed point representing the credential ID
    """
    # Convert inputs to integers
    prf = bytes_to_int(bls_private_key)
    
    # The base point G is already defined in py_ecc as G1
    
    # Calculate (prf + cred_counter)
    sum_mod_r = (prf + cred_counter) % curve_order
    
    # Calculate inverse modulo curve_order
    cred_id_exponent = pow(sum_mod_r, -1, curve_order)
    
    # Multiply the base point by the exponent
    cred_id_point = multiply(G1, cred_id_exponent)
    
    # Compress the resulting point
    compressed_cred_id = compress_g1(cred_id_point)
    
    return compressed_cred_id

# TODO: INSPECT IF THIS WORKS
def bls_key_gen(seed: bytes) -> bytes:
    """
    Implements the BLS key generation algorithm as per the IETF draft:
    https://datatracker.ietf.org/doc/html/draft-irtf-cfrg-bls-signature-04#section-2.3
    """
    BLS_KEY_LENGTH = 32  # Secret key length in bytes
    SEED_LENGTH = 32     # Seed length in bytes
    l_CONST = 48         # Length of the HKDF expand output in bytes

    if len(seed) != SEED_LENGTH:
        raise ValueError("Seed must be exactly 32 bytes long.")

    # Initialize salt as "BLS-SIG-KEYGEN-SALT-"
    salt = bytes([ord(c) for c in "BLS-SIG-KEYGEN-SALT-"])

    # IKM (Input Keying Material) is the seed concatenated with a zero byte
    ikm = seed + b'\x00'

    # l_bytes is the length in bytes represented as a 2-byte big-endian integer
    l_bytes = l_CONST.to_bytes(2, 'big')

    # The order 'r' of the BLS12-381 curve
    r = int("0x73eda753299d7d483339d80809a1d80553bd402fffe5bfeffff00000001", 16)
    
    while True:
        # Hash the salt
        salt = hashlib.sha256(salt).digest()

        # HKDF-Extract
        prk = hmac.new(salt, ikm, hashlib.sha256).digest()

        # HKDF-Expand
        okm = hkdf_expand(prk, l_bytes, l_CONST)

        # Convert the output keying material to an integer modulo r
        sk_int = int.from_bytes(okm, 'big') % r

        if sk_int != 0:
            break  # Valid secret key found

    # Convert the integer back to bytes (32 bytes long)
    sk_bytes = sk_int.to_bytes(BLS_KEY_LENGTH, 'big')

    return sk_bytes

# TODO: INSPECT IF THIS WORKS
def get_private_key_from_path(path: str) -> bytes:
    """
    Derives a raw Ed25519 private key from a BIP32 derivation path using SLIP-0010.

    This function takes a BIP32 path and derives an Ed25519 private key using SLIP-0010 derivation.
    It uses a fixed test seed for deterministic key generation. The derived key is returned as raw bytes.

    Args:
        path (str): BIP32 derivation path (e.g. "m/1105'/0'/0'/0/0")
                   Must be a valid BIP32 path with maximum 7 levels (including master 'm')

    Returns:
        bytes: Raw 32-byte Ed25519 private key

    Raises:
        ValueError: If the provided path has more than 7 levels (m/1/2/3/4/5/6)
        Bip32KeyError: If key derivation fails
    """
    # Validate path length - maximum 7 levels allowed (including master 'm')
    # This helps maintain reasonable derivation depth for security and performance
    if(len(path.split('/')) > 7):
        raise ValueError("The path is too long")

    # Initialize with test seed for deterministic testing
    # Note: In production, seed would come from secure random source or mnemonic
    seed = b"ed25519 seed"
    seed = hashlib.sha512(seed).digest()

    # Initialize SLIP-0010 context for Ed25519
    # SLIP-0010 provides deterministic key derivation for Ed25519
    bip32_ctx: Bip32Slip10Ed25519 = Bip32Slip10Ed25519.FromSeed(seed)
    # Perform hierarchical derivation using the provided path
    # Each path component triggers a child key derivation step
    derived_key = bip32_ctx.DerivePath(path)

    # Get the raw private key bytes from the derived key
    private_key_bytes = derived_key.PrivateKey().Raw().ToBytes()

    # Create standard Ed25519 key from raw bytes
    # This ensures compatibility with common Ed25519 operations
    private_key = Ed25519PrivateKey.from_private_bytes(private_key_bytes)

    # Convert to raw bytes format for return
    # Strips away any key formatting, returning just the core 32 bytes
    private_key_bytes = private_key.private_bytes_raw
    return private_key_bytes

# TODO: ADD THE CASE FOR THE NEW SCHEME (44')
def get_bls_private_key(path: str) -> bytes:
    """
    Calculate the BLS private key from a given derivation path.

    This function takes a BIP32 derivation path and generates a BLS private key.
    It supports two main derivation schemes:
    - Path starting with 1105' (legacy scheme)
    - Path starting with 44' (new scheme, not yet implemented)

    Args:
        path (str): BIP32 derivation path (e.g. "m/1105'/0'/0'/0/0")

    Returns:
        bytes: The 32-byte BLS private key

    Raises:
        ValueError: If the path is invalid (too short or unsupported scheme)
    """
    # Split the path into its components
    path_parts = path.split('/')

    # Check minimum path length (m/purpose'/coin'/account'/change/address)
    if(len(path_parts) < 6):
        raise ValueError("The path is too short")
    if(len(path_parts) > 6):
        raise ValueError("The path is too long")

    # Handle Legacy scheme (1105')
    if(path_parts[1] == "1105'"):
        # Get the regular private key and convert it to BLS
        private_key = get_private_key_from_path(path)
        print("km-Private key: ", private_key)
        return bls_key_gen(private_key)

    # New scheme (44') - not implemented yet
    elif(path_parts[1] == "44'"):
        raise ValueError("Not implemented yet")

    # Invalid derivation scheme
    else:

        raise ValueError("Invalid path")

# return a 32 bytes address
def get_address(path: str, cred_counter: int) -> bytes:
    bls_private_key = get_bls_private_key(path)
    cred_id = get_cred_id(bls_private_key, cred_counter)
    return base58.b58encode(hashlib.sha256(cred_id).digest())


# In this test we check that the VERIFY ADDRESS works in non-confirmation mode
# TODO: VERIFY THE ADDRESS WITH THE DEVICE
def test_verify_address_no_confirm(backend):
    path = "m/1105'/0'/0'/0'/0'/1'"
    # Get the expected address
    expected_address = get_address(path, 0)
    # Ask the device to ve
    assert expected_address == b"1234567890"
    # for path in ["m/44'/919'/0'/0/0", "m/44'/919'/0/0/0", "m/44'/919'/911'/0/0", "m/44'/919'/255/255/255", "m/44'/919'/2147483647/0/0/0/0/0/0/0",
    #              "m/1105'/0'/0'/0/0", "m/1105'/0'/0/0/0", "m/1105'/0'/911'/0/0", "m/1105'/0'/255/255/255", "m/1105'/0'/2147483647/0/0/0/0/0/0/0"]:
    #     client = BoilerplateCommandSender(backend)
    #     response = client.backend.exchange(cla=0xe0,ins=0x00,p1=0x00,p2=0x00,
    #                                        data=b"")

    #     ref_public_key, ref_chain_code = calculate_public_key_and_chaincode(CurveChoice.Secp256k1, path=path)
    #     assert public_key.hex() == ref_public_key
    #     assert chain_code.hex() == ref_chain_code


# # In this test we check that the VERIFY ADDRESS works in confirmation mode
# def test_verify_address_confirm_accepted(backend, scenario_navigator):
#     client = BoilerplateCommandSender(backend)
#     path = "m/44'/919'/0'/0/0"
#     with client.get_public_key_with_confirmation(path=path):
#         scenario_navigator.address_review_approve()

#     response = client.get_async_response().data
#     _, public_key, _, chain_code = unpack_get_public_key_response(response)

#     ref_public_key, ref_chain_code = calculate_public_key_and_chaincode(CurveChoice.Secp256k1, path=path)
#     assert public_key.hex() == ref_public_key
#     assert chain_code.hex() == ref_chain_code


# # In this test we check that the VERIFY ADDRESS in confirmation mode replies an error if the user refuses
# def test_verify_address_confirm_refused(backend, scenario_navigator):
#     client = BoilerplateCommandSender(backend)
#     path = "m/44'/919'/0'/0/0"

#     with pytest.raises(ExceptionRAPDU) as e:
#         with client.get_public_key_with_confirmation(path=path):
#             scenario_navigator.address_review_reject()

#     # Assert that we have received a refusal
#     assert e.value.status == Errors.SW_DENY
#     assert len(e.value.data) == 0
