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
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID
from utils import navigate_until_text_and_compare

# In this tests we check the behavior of the device when asked to sign a transaction


# # In this test we send to the device a transaction to sign and validate it on screen
# # The transaction is short and will be sent in one chunk
# # We will ensure that the displayed information is correct by using screenshots comparison
# @pytest.mark.active_test_scope
# def test_sign_tx_simple_transfer_legacy_path(
#     backend, firmware, navigator, default_screenshot_path, test_name
# ):
#     # Use the app interface instead of raw interface
#     client = BoilerplateCommandSender(backend)
#     # The path used for this entire test
#     path: str = "m/1105/0/0/0/0/2/0/0"

#     # Create the transaction that will be sent to the device for signing
#     transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7ffffffffffffffff"
#     transaction = bytes.fromhex(transaction)

#     # Send the sign device instruction.
#     # As it requires on-screen validation, the function is asynchronous.
#     # It will yield the result when the navigation is done
#     with client.sign_simple_transfer(path=path, transaction=transaction):
#         # Validate the on-screen request by performing the navigation appropriate for this device
#         navigate_until_text_and_compare(
#             firmware, navigator, "Sign", default_screenshot_path, test_name
#         )

#     # The device as yielded the result, parse it and ensure that the signature is correct
#     response = client.get_async_response().data
#     response_hex = response.hex()
#     print("response", response_hex)
#     assert (
#         response_hex
#         == "d1617ee706805c0bc6a43260ece93a7ceba37aaefa303251cf19bdcbbe88c0a3d3878dcb965cdb88ff380fdb1aa4b321671f365d7258e878d18fa1b398a1a10f"
#     )
#     # assert check_signature_validity(public_key, der_sig, transaction)


# # In this test we send to the device a transaction to sign and validate it on screen
# # The transaction is short and will be sent in one chunk
# # We will ensure that the displayed information is correct by using screenshots comparison
# @pytest.mark.active_test_scope
# def test_sign_tx_simple_transfer_new_path(
#     backend, firmware, navigator, default_screenshot_path, test_name
# ):
#     # Use the app interface instead of raw interface
#     client = BoilerplateCommandSender(backend)
#     # The path used for this entire test
#     path: str = "m/44/919/0/0/0/0"

#     # Create the transaction that will be sent to the device for signing
#     transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da70320a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7ffffffffffffffff"
#     transaction = bytes.fromhex(transaction)

#     # Send the sign device instruction.
#     # As it requires on-screen validation, the function is asynchronous.
#     # It will yield the result when the navigation is done
#     with client.sign_simple_transfer(path=path, transaction=transaction):
#         # Validate the on-screen request by performing the navigation appropriate for this device
#         navigate_until_text_and_compare(
#             firmware, navigator, "Sign", default_screenshot_path, test_name
#         )

#     # The device as yielded the result, parse it and ensure that the signature is correct
#     response = client.get_async_response().data
#     response_hex = response.hex()
#     print("response", response_hex)
#     assert (
#         response_hex
#         == "e5f112237d58f908c44385827e71048869db7e8f513e2ceb5da6a6370e2088f4371f93d6e08f9f6c1dd92c74fe565727b8f81600541e817d35cfeec4cc3bc408"
#     )


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
@pytest.mark.active_test_scope
def test_sign_tx_simple_transfer_with_memo_legacy_path(
    backend, firmware, navigator, default_screenshot_path, test_name
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
            firmware, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    # TODO: verify the signature
    assert (
        response_hex
        == "a588094eef4ed6053df2ab4b851bc5ec09b311c204d2fa94a9c7d7c8feebf74de26d2d2a547f18c4e959b24388394305ebd3dca99653de1cb1aa689bb6674207"
    )
    # assert check_signature_validity(public_key, der_sig, transaction)
