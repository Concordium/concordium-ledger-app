from hashlib import sha256

# from sha3 import keccak_256  # type: ignore
from typing import List

from ecdsa.curves import SECP256k1  # type: ignore
from ecdsa.keys import VerifyingKey  # type: ignore
from ecdsa.util import sigdecode_der  # type: ignore

from ragger.navigator import NavInsID


def split_message(message: bytes, max_size: int) -> List[bytes]:
    """
    Splits a bytes message into a list of chunks, each with a maximum size of `max_size`.

    Args:
        message (bytes): The message to be split.
        max_size (int): The maximum size of each chunk.

    Returns:
        List[bytes]: A list of byte chunks, each of length at most `max_size`.

    Example:
        >>> split_message(b'abcdefgh', 3)
        [b'abc', b'def', b'gh']
    """
    return [message[x : x + max_size] for x in range(0, len(message), max_size)]


# Check if a signature of a given message is valid
# def check_signature_validity(
#     public_key: bytes, signature: bytes, message: bytes
# ) -> bool:
#     pk: VerifyingKey = VerifyingKey.from_string(
#         public_key, curve=SECP256k1, hashfunc=sha256
#     )
#     return pk.verify(
#         signature=signature, data=message, hashfunc=keccak_256, sigdecode=sigdecode_der
#     )


def build_tx_with_payload(
    payload: str,
    sender: str = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7",
):
    # Create the transaction that will be sent to the device for signing
    _payload = bytes.fromhex(payload)
    if len(sender) != 64:
        raise Exception("The sender address should be 64characters")

    HEADER = sender
    HEADER += "000000000000000a"  # SequenceNumber
    HEADER += "0000000000000064"  # EnergyAmount
    HEADER += len(_payload).to_bytes(4, "big").hex()  # PayloadSize
    HEADER += "0000000063de5da7"  # Expiry

    transaction = bytes.fromhex(HEADER) + _payload
    return transaction


def instructions_builder(
    number_of_screens_until_confirm: int,
    backend,
    confirm_instruction: NavInsID = NavInsID.USE_CASE_REVIEW_CONFIRM,
) -> list[NavInsID]:
    if backend.device.is_nano:
        go_right_instruction = NavInsID.RIGHT_CLICK
        temp_confirm_instruction = NavInsID.BOTH_CLICK
    else:
        go_right_instruction = NavInsID.SWIPE_CENTER_TO_LEFT
        temp_confirm_instruction = confirm_instruction

    # Add the go right instruction for the number of screens needed
    instructions = [go_right_instruction] * number_of_screens_until_confirm
    # Add the confirm instruction
    instructions.append(temp_confirm_instruction)
    return instructions


def navigate_until_text_and_compare(
    backend,
    navigator,
    text: str,
    screenshot_path: str,
    test_name: str,
    screen_change_before_first_instruction: bool = True,
    screen_change_after_last_instruction: bool = True,
    nav_ins_confirm_instruction: NavInsID = NavInsID.USE_CASE_REVIEW_CONFIRM,
):
    """Navigate through device screens until specified text is found and compare screenshots.

    This function handles navigation through device screens differently based on the device type (touch screen devices vs others).
    It will navigate through screens until the specified text is found, taking screenshots for comparison along the way.

    Args:
        backend: The backend object containing device information
        navigator: The navigator object used to control device navigation
        text: The text string to search for on device screens
        screenshot_path: Path where screenshot comparison files will be saved
        test_name: The name of the test that is being run
        screen_change_before_first_instruction: Whether to wait for screen change before first instruction
        screen_change_after_last_instruction: Whether to wait for screen change after last instruction
    Returns:
        None

    Note:
        For touch screen devices:
        - Uses swipe left gesture for navigation
        - Uses review confirm for confirmation
        For other devices:
        - Uses right click for navigation
        - Uses both click for confirmation
    """
    if backend.device.is_nano:
        go_right_instruction = NavInsID.RIGHT_CLICK
        confirm_instructions = [NavInsID.BOTH_CLICK]
    else:
        go_right_instruction = NavInsID.SWIPE_CENTER_TO_LEFT
        confirm_instructions = [nav_ins_confirm_instruction]

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
