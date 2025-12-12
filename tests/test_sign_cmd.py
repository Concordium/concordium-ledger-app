import pytest

from application_client.boilerplate_transaction import Transaction
from application_client.boilerplate_command_sender import (
    BoilerplateCommandSender,
    Errors,
    InsType,
)
from application_client.boilerplate_response_unpacker import (
    unpack_get_public_key_response,
    unpack_sign_tx_response,
)
from ledgered.devices import DeviceType
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID
from utils import navigate_until_text_and_compare, instructions_builder, split_message

MAX_SCHEDULE_PAIRS_IN_ONE_APDU: int = (250 // 16) * 16
MAX_APDU_LEN: int = 255


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
@pytest.mark.active_test_scope
def test_sign_tx_simple_transfer_legacy_path(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/1105/0/0/0/0/2/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7ffffffffffffffff"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_simple_transfer(path=path, transaction=transaction):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "d1617ee706805c0bc6a43260ece93a7ceba37aaefa303251cf19bdcbbe88c0a3d3878dcb965cdb88ff380fdb1aa4b321671f365d7258e878d18fa1b398a1a10f"
    )
    # assert check_signature_validity(public_key, der_sig, transaction)


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
@pytest.mark.active_test_scope
def test_sign_tx_simple_transfer_new_path(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/44/919/0/0/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7ffffffffffffffff"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_simple_transfer(path=path, transaction=transaction):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "e5f112237d58f908c44385827e71048869db7e8f513e2ceb5da6a6370e2088f4371f93d6e08f9f6c1dd92c74fe565727b8f81600541e817d35cfeec4cc3bc408"
    )


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
@pytest.mark.active_test_scope
def test_sign_tx_simple_transfer_with_memo_legacy_path(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/1105/0/0/0/0/2/0/0"

    # Create the transaction that will be sent to the device for signing
    header_and_to_address = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71620a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7"
    header_and_to_address = bytes.fromhex(header_and_to_address)
    memo = "6474657374"
    memo = bytes.fromhex(memo)
    amount = "ffffffffffffffff"
    amount = bytes.fromhex(amount)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_simple_transfer_with_memo(
        path=path,
        header_and_to_address=header_and_to_address,
        memo=memo,
        amount=amount,
    ):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)

    assert (
        response_hex
        == "a588094eef4ed6053df2ab4b851bc5ec09b311c204d2fa94a9c7d7c8feebf74de26d2d2a547f18c4e959b24388394305ebd3dca99653de1cb1aa689bb6674207"
    )
    # assert check_signature_validity(public_key, der_sig, transaction)


@pytest.mark.active_test_scope
def test_sign_tx_transfer_with_schedule_legacy_path(
    backend, navigator, default_screenshot_path, test_name
):
    # Initialize the command sender client
    client = BoilerplateCommandSender(backend)
    # Define the path for the transaction
    path = "m/1105/0/0/0/0/2/0/0"

    # Create the transaction that will be sent to the device for signing
    header_and_to_address = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7"
    header_and_to_address = bytes.fromhex(header_and_to_address)

    # Define the schedule pairs
    pairs = [
        "0000017a396883d90000000005f5e100",
        "0000017a396883d90000000005f5e100",
        "0000017a396883d90000000005f5e100",
        "0000017a396883d90000000005f5e100",
        "0000017a396883d90000000005f5e100",
    ]
    joined_pairs = bytes.fromhex("".join(pairs))

    # Ensure pairs are a multiple of 16 bytes
    if len(joined_pairs) % 16 != 0:
        raise ValueError("Pairs must be a multiple of 16 bytes")

    # Split the pairs into chunks for APDU transmission
    pairs_chunk = split_message(joined_pairs, MAX_SCHEDULE_PAIRS_IN_ONE_APDU)

    # Send the first part of the transaction signing request

    with client.sign_tx_with_schedule_part_1(
        path=path,
        header_and_to_address=header_and_to_address,
        num_pairs=len(pairs),
    ):
        # Navigate and compare screenshots for validation
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Continue",
            default_screenshot_path,
            test_name,
            True,
            False,
            NavInsID.USE_CASE_CHOICE_CONFIRM,
        )

    # Process each chunk of pairs
    screenshots_so_far = 3
    if backend.device.is_nano:
        screenshots_so_far = 6
    #    elif backend.device.type == DeviceType.APEX_P:
    #        screenshots_so_far = 4

    for chunk in pairs_chunk:
        nbgl_confirm_instruction = NavInsID.USE_CASE_CHOICE_CONFIRM
        number_of_pairs_in_chunk = len(chunk) // 16
        # Create the instructions to validate each pair
        instructions = []
        for _ in range(number_of_pairs_in_chunk):
            if _ == number_of_pairs_in_chunk - 1:
                nbgl_confirm_instruction = NavInsID.USE_CASE_REVIEW_CONFIRM
            instructions.extend(
                instructions_builder(2, backend, nbgl_confirm_instruction)
            )

        # Send the second part of the transaction signing request
        with client.sign_tx_with_schedule_part_2(data=chunk):
            # Navigate and compare screenshots for validation
            navigator.navigate_and_compare(
                default_screenshot_path,
                test_name,
                instructions,
                10,
                True,
                True,
                screenshots_so_far,
            )
        screenshots_so_far += number_of_pairs_in_chunk * 3

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()

    assert (
        response_hex
        == "e22fa38f78a79db71e84376c4eec2382166cdc412994207e7631b0ba3828f069b17b6f30351a64c50e5efacec3fe25161e9f7131e0235cd740739b24e0b06308"
    )


@pytest.mark.active_test_scope
def test_sign_tx_transfer_with_schedule_and_memo_legacy_path(
    backend, navigator, default_screenshot_path, test_name
):
    # Initialize the command sender client
    client = BoilerplateCommandSender(backend)
    # Define the path for the transaction
    path = "m/1105/0/0/0/0/2/0/0"

    # Create the transaction that will be sent to the device for signing
    header_and_to_address = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71820a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7"
    header_and_to_address = bytes.fromhex(header_and_to_address)

    # Define the memo
    memo = "6474657374"  # "test" in hex
    memo = bytes.fromhex(memo)

    memo_chunks = split_message(memo, MAX_APDU_LEN)

    # Define the schedule pairs
    pairs = [
        "0000017a396883d90000000005f5e100",
        "0000017a396883d90000000005f5e100",
        "0000017a396883d90000000005f5e100",
        "0000017a396883d90000000005f5e100",
        "0000017a396883d90000000005f5e100",
    ]
    joined_pairs = bytes.fromhex("".join(pairs))

    # Ensure pairs are a multiple of 16 bytes
    if len(joined_pairs) % 16 != 0:
        raise ValueError("Pairs must be a multiple of 16 bytes")

    # Split the pairs into chunks for APDU transmission
    pairs_chunk = split_message(joined_pairs, MAX_SCHEDULE_PAIRS_IN_ONE_APDU)

    # Send the first part of the transaction signing request
    response = client.sign_tx_with_schedule_and_memo_part_1(
        path=path,
        header_and_to_address=header_and_to_address,
        num_pairs=len(pairs),
        memo_length=len(memo),
    )

    assert response.status == 0x9000
    # Send the part with the memo
    for chunk in memo_chunks:
        with client.sign_tx_with_schedule_and_memo_part_2(memo_chunk=chunk):
            navigate_until_text_and_compare(
                backend,
                navigator,
                "Continue",
                default_screenshot_path,
                test_name,
                True,
                False,
                NavInsID.USE_CASE_CHOICE_CONFIRM,
            )

    # Process each chunk of pairs
    screenshots_so_far = 3
    if backend.device.is_nano:
        screenshots_so_far = 7
    elif backend.device.type == DeviceType.APEX_P:
        screenshots_so_far = 4

    for chunk in pairs_chunk:
        nbgl_confirm_instruction = NavInsID.USE_CASE_CHOICE_CONFIRM
        number_of_pairs_in_chunk = len(chunk) // 16
        # Create the instructions to validate each pair
        instructions = []
        for _ in range(number_of_pairs_in_chunk):
            if _ == number_of_pairs_in_chunk - 1:
                nbgl_confirm_instruction = NavInsID.USE_CASE_REVIEW_CONFIRM
            instructions.extend(
                instructions_builder(2, backend, nbgl_confirm_instruction)
            )

        # Send the second part of the transaction signing request
        with client.sign_tx_with_schedule_part_2(data=chunk):
            # Navigate and compare screenshots for validation
            navigator.navigate_and_compare(
                default_screenshot_path,
                test_name,
                instructions,
                10,
                True,
                True,
                screenshots_so_far,
            )
        screenshots_so_far += number_of_pairs_in_chunk * 3

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()

    assert (
        response_hex
        == "9056db36dfa7b0ba722660b2becb227ed490dcaff9e332a7fba4c6d534ff0ff3368b21da8e7ebb62891be561261abd7c0435dfb46e596b1116c9996269d2a70b"
    )
