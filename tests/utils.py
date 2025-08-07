from hashlib import sha256
from sha3 import keccak_256  # type: ignore
from typing import List

from ecdsa.curves import SECP256k1  # type: ignore
from ecdsa.keys import VerifyingKey  # type: ignore
from ecdsa.util import sigdecode_der  # type: ignore

from ragger.navigator import NavInsID


def split_message(message: bytes, max_size: int) -> List[bytes]:
    return [message[x : x + max_size] for x in range(0, len(message), max_size)]


# Check if a signature of a given message is valid
def check_signature_validity(
    public_key: bytes, signature: bytes, message: bytes
) -> bool:
    pk: VerifyingKey = VerifyingKey.from_string(
        public_key, curve=SECP256k1, hashfunc=sha256
    )
    return pk.verify(
        signature=signature, data=message, hashfunc=keccak_256, sigdecode=sigdecode_der
    )


def instructions_builder(
    number_of_screens_until_confirm: int,
    backend,
    confirm_instruction: NavInsID = NavInsID.USE_CASE_REVIEW_CONFIRM,
) -> list[NavInsID]:
    if backend.firmware.device.startswith(("stax", "flex")):
        go_right_instruction = NavInsID.SWIPE_CENTER_TO_LEFT
        temp_confirm_instruction = confirm_instruction
    else:
        go_right_instruction = NavInsID.RIGHT_CLICK
        temp_confirm_instruction = NavInsID.BOTH_CLICK

    # Add the go right instruction for the number of screens needed
    instructions = [go_right_instruction] * number_of_screens_until_confirm
    # Add the confirm instruction
    instructions.append(temp_confirm_instruction)
    return instructions


def navigate_until_text_and_compare(
    firmware,
    navigator,
    text: str,
    screenshot_path: str,
    test_name: str,
    screen_change_before_first_instruction: bool = True,
    screen_change_after_last_instruction: bool = True,
    nav_ins_confirm_instruction: NavInsID = NavInsID.USE_CASE_REVIEW_CONFIRM,
):
    """Navigate through device screens until specified text is found and compare screenshots.

    This function handles navigation through device screens differently based on the device type (Stax/Flex vs others).
    It will navigate through screens until the specified text is found, taking screenshots for comparison along the way.

    Args:
        firmware: The firmware object containing device information
        navigator: The navigator object used to control device navigation
        text: The text string to search for on device screens
        screenshot_path: Path where screenshot comparison files will be saved
        test_name: The name of the test that is being run
        screen_change_before_first_instruction: Whether to wait for screen change before first instruction
        screen_change_after_last_instruction: Whether to wait for screen change after last instruction
    Returns:
        None

    Note:
        For Stax/Flex devices:
        - Uses swipe left gesture for navigation
        - Uses review confirm for confirmation
        For other devices:
        - Uses right click for navigation
        - Uses both click for confirmation
    """
    if firmware.device.startswith(("stax", "flex")):
        go_right_instruction = NavInsID.SWIPE_CENTER_TO_LEFT
        confirm_instructions = [nav_ins_confirm_instruction]
    else:
        go_right_instruction = NavInsID.RIGHT_CLICK
        confirm_instructions = [NavInsID.BOTH_CLICK]

    navigator.navigate_until_text_and_compare(
        go_right_instruction,
        confirm_instructions,
        text,
        screenshot_path,
        test_name,
        300,
        screen_change_before_first_instruction,
        screen_change_after_last_instruction,
    )


# The following functions might be useful someday, they are not tested, so some of them might not behave as expected

# def hkdf_expand(prk: bytes, info: bytes, length: int) -> bytes:
#     """HKDF-Expand as per RFC 5869 Section 2.3."""
#     hash_len = hashlib.sha256().digest_size
#     n = (length + hash_len - 1) // hash_len
#     if n > 255:
#         raise Exception("Cannot expand to more than 255 * HashLen bytes using HKDF")
#     okm = b''
#     t = b''
#     for i in range(1, n + 1):
#         t = hmac.new(prk, t + info + bytes([i]), hashlib.sha256).digest()
#         okm += t
#     return okm[:length]

# def int_to_bytes(x: int, length: int) -> bytes:
#     return x.to_bytes(length, 'big')

# def bytes_to_int(b: bytes) -> int:
#     return int.from_bytes(b, 'big')

# def compress_g1(point: Tuple[int, int, int]) -> bytes:
#     """Compress a G1 point to 48 bytes"""
#     x, y, _ = point
#     flag = 0x80  # Set compression flag
#     if y > curve_order - y:  # if y > -y
#         flag |= 0x20
#     return int_to_bytes((x | (flag << (8 * 48))) % curve_order, 48)

# # TODO: INSPECT IF THIS WORKS (come from generative model)
# def get_cred_id(bls_private_key: bytes, cred_counter: int) -> bytes:
#     """
#     Calculate the credential ID from the PRF key and credential counter.

#     Args:
#         bls_private_key: 32-byte private key
#         cred_counter: credential counter as integer

#     Returns:
#         48-byte compressed point representing the credential ID
#     """
#     # Convert inputs to integers
#     prf = bytes_to_int(bls_private_key)

#     # The base point G is already defined in py_ecc as G1

#     # Calculate (prf + cred_counter)
#     sum_mod_r = (prf + cred_counter) % curve_order

#     # Calculate inverse modulo curve_order
#     cred_id_exponent = pow(sum_mod_r, -1, curve_order)

#     # Multiply the base point by the exponent
#     cred_id_point = multiply(G1, cred_id_exponent)

#     # Compress the resulting point
#     compressed_cred_id = compress_g1(cred_id_point)

#     return compressed_cred_id

# # TODO: INSPECT IF THIS WORKS
# def bls_key_gen(seed: bytes) -> bytes:
#     """
#     Implements the BLS key generation algorithm as per the IETF draft:
#     https://datatracker.ietf.org/doc/html/draft-irtf-cfrg-bls-signature-04#section-2.3
#     """
#     BLS_KEY_LENGTH = 32  # Secret key length in bytes
#     SEED_LENGTH = 32     # Seed length in bytes
#     l_CONST = 48         # Length of the HKDF expand output in bytes

#     if len(seed) != SEED_LENGTH:
#         raise ValueError("Seed must be exactly 32 bytes long.")

#     # Initialize salt as "BLS-SIG-KEYGEN-SALT-"
#     salt = bytes([ord(c) for c in "BLS-SIG-KEYGEN-SALT-"])

#     # IKM (Input Keying Material) is the seed concatenated with a zero byte
#     ikm = seed + b'\x00'

#     # l_bytes is the length in bytes represented as a 2-byte big-endian integer
#     l_bytes = l_CONST.to_bytes(2, 'big')

#     # The order 'r' of the BLS12-381 curve
#     r = int("0x73eda753299d7d483339d80809a1d80553bd402fffe5bfeffff00000001", 16)

#     while True:
#         # Hash the salt
#         salt = hashlib.sha256(salt).digest()

#         # HKDF-Extract
#         prk = hmac.new(salt, ikm, hashlib.sha256).digest()

#         # HKDF-Expand
#         okm = hkdf_expand(prk, l_bytes, l_CONST)

#         # Convert the output keying material to an integer modulo r
#         sk_int = int.from_bytes(okm, 'big') % r

#         if sk_int != 0:
#             break  # Valid secret key found

#     # Convert the integer back to bytes (32 bytes long)
#     sk_bytes = sk_int.to_bytes(BLS_KEY_LENGTH, 'big')

#     return sk_bytes

# # TODO: INSPECT IF THIS WORKS
# def get_private_key_from_path(path: str) -> bytes:
#     """
#     Derives a raw Ed25519 private key from a BIP32 derivation path using SLIP-0010.

#     This function takes a BIP32 path and derives an Ed25519 private key using SLIP-0010 derivation.
#     It uses a fixed test seed for deterministic key generation. The derived key is returned as raw bytes.

#     Args:
#         path (str): BIP32 derivation path (e.g. "m/1105'/0'/0'/0/0")
#                    Must be a valid BIP32 path with maximum 7 levels (including master 'm')

#     Returns:
#         bytes: Raw 32-byte Ed25519 private key

#     Raises:
#         ValueError: If the provided path has more than 7 levels (m/1/2/3/4/5/6)
#         Bip32KeyError: If key derivation fails
#     """
#     # Validate path length - maximum 7 levels allowed (including master 'm')
#     # This helps maintain reasonable derivation depth for security and performance
#     if(len(path.split('/')) > 7):
#         raise ValueError("The path is too long")

#     # Initialize with test seed for deterministic testing
#     # Note: In production, seed would come from secure random source or mnemonic
#     seed = b"ed25519 seed"
#     seed = hashlib.sha512(seed).digest()

#     # Initialize SLIP-0010 context for Ed25519
#     # SLIP-0010 provides deterministic key derivation for Ed25519
#     bip32_ctx: Bip32Slip10Ed25519 = Bip32Slip10Ed25519.FromSeed(seed)
#     # Perform hierarchical derivation using the provided path
#     # Each path component triggers a child key derivation step
#     derived_key = bip32_ctx.DerivePath(path)

#     # Get the raw private key bytes from the derived key
#     private_key_bytes = derived_key.PrivateKey().Raw().ToBytes()

#     # Create standard Ed25519 key from raw bytes
#     # This ensures compatibility with common Ed25519 operations
#     private_key = Ed25519PrivateKey.from_private_bytes(private_key_bytes)

#     # Convert to raw bytes format for return
#     # Strips away any key formatting, returning just the core 32 bytes
#     private_key_bytes = private_key.private_bytes_raw
#     return private_key_bytes

# # TODO: ADD THE CASE FOR THE NEW SCHEME (44')
# def get_bls_private_key(path: str) -> bytes:
#     """
#     Calculate the BLS private key from a given derivation path.

#     This function takes a BIP32 derivation path and generates a BLS private key.
#     It supports two main derivation schemes:
#     - Path starting with 1105' (legacy scheme)
#     - Path starting with 44' (new scheme, not yet implemented)

#     Args:
#         path (str): BIP32 derivation path (e.g. "m/1105'/0'/0'/0/0")

#     Returns:
#         bytes: The 32-byte BLS private key

#     Raises:
#         ValueError: If the path is invalid (too short or unsupported scheme)
#     """
#     # Split the path into its components
#     path_parts = path.split('/')

#     # Check minimum path length (m/purpose'/coin'/account'/change/address)
#     if(len(path_parts) < 6):
#         raise ValueError("The path is too short")
#     if(len(path_parts) > 6):
#         raise ValueError("The path is too long")

#     # Handle Legacy scheme (1105')
#     if(path_parts[1] == "1105'"):
#         # Get the regular private key and convert it to BLS
#         private_key = get_private_key_from_path(path)
#         print("km-Private key: ", private_key)
#         return bls_key_gen(private_key)

#     # New scheme (44') - not implemented yet
#     elif(path_parts[1] == "44'"):
#         raise ValueError("Not implemented yet")

#     # Invalid derivation scheme
#     else:

#         raise ValueError("Invalid path")

# # return a 32 bytes address
# def get_address(path: str, cred_counter: int) -> bytes:
#     bls_private_key = get_bls_private_key(path)
#     cred_id = get_cred_id(bls_private_key, cred_counter)
#     return base58.b58encode(hashlib.sha256(cred_id).digest())
