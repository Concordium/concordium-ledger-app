import pytest

from application_client.boilerplate_transaction import Transaction
from application_client.boilerplate_command_sender import BoilerplateCommandSender, Errors
from application_client.boilerplate_response_unpacker import unpack_get_public_key_response, unpack_sign_tx_response
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID
from utils import check_signature_validity

# In this tests we check the behavior of the device when asked to sign a transaction


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
def test_sign_tx_simple_transfer(backend, scenario_navigator):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/1105/0/0/0/0/2/0/0"

    # First we need to get the public key of the device in order to build the transaction
    # rapdu = client.get_public_key(path=path)
    # _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    # Create the transaction that will be sent to the device for signing
    transaction = b"20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71620a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d70005"

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_tx(path=path, transaction=transaction):
        # Validate the on-screen request by performing the navigation appropriate for this device
        scenario_navigator.review_approve()

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    print('response', response.hex())
    _, der_sig, _ = unpack_sign_tx_response(response)
    print('der_sig', der_sig.hex())
    assert der_sig.hex() == "12afcc203c73075ae3e4d89e01844b3fb1b2ecef26565d8c1220e04bddfb7ced0fe38b06a6df22669a20eea4b180ea3d1e1e4ad28a1d2bea29e518ad53f1550d"
    # assert check_signature_validity(public_key, der_sig, transaction)


# # In this test we send to the device a transaction to trig a blind-signing flow
# # The transaction is short and will be sent in one chunk
# # We will ensure that the displayed information is correct by using screenshots comparison
# def test_sign_tx_short_tx_blind_sign(firmware, navigator, backend, scenario_navigator, test_name, default_screenshot_path):
#     if firmware.is_nano:
#         pytest.skip("Not supported on Nano devices")

#     # Use the app interface instead of raw interface
#     client = BoilerplateCommandSender(backend)
#     # The path used for this entire test
#     path: str = "m/44'/919'/0'/0/0"

#     # First we need to get the public key of the device in order to build the transaction
#     rapdu = client.get_public_key(path=path)
#     _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

#     # Create the transaction that will be sent to the device for signing
#     transaction = Transaction(
#         nonce=1,
#         to="0x0000000000000000000000000000000000000000",
#         value=0,
#         memo="Blind-sign"
#     ).serialize()

#     # Send the sign device instruction.
#     # As it requires on-screen validation, the function is asynchronous.
#     # It will yield the result when the navigation is done
#     with client.sign_tx(path=path, transaction=transaction):
#         navigator.navigate_and_compare(default_screenshot_path,
#                                         test_name+"/part1",
#                                         [NavInsID.USE_CASE_CHOICE_REJECT],
#                                         screen_change_after_last_instruction=False)

#         # Validate the on-screen request by performing the navigation appropriate for this device
#         scenario_navigator.review_approve()

#     # The device as yielded the result, parse it and ensure that the signature is correct
#     response = client.get_async_response().data
#     _, der_sig, _ = unpack_sign_tx_response(response)
#     assert check_signature_validity(public_key, der_sig, transaction)

# # In this test se send to the device a transaction to sign and validate it on screen
# # This test is mostly the same as the previous one but with different values.
# # In particular the long memo will force the transaction to be sent in multiple chunks
# def test_sign_tx_long_tx(backend, scenario_navigator):
#     # Use the app interface instead of raw interface
#     client = BoilerplateCommandSender(backend)
#     path: str = "m/44'/919'/0'/0/0"

#     rapdu = client.get_public_key(path=path)
#     _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

#     transaction = Transaction(
#         nonce=1,
#         to="0xde0b295669a9fd93d5f28d9ec85e40f4cb697bae",
#         value=666,
#         memo=("This is a very long memo. "
#               "It will force the app client to send the serialized transaction to be sent in chunk. "
#               "As the maximum chunk size is 255 bytes we will make this memo greater than 255 characters. "
#               "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed non risus. Suspendisse lectus tortor, dignissim sit amet, adipiscing nec, ultricies sed, dolor. Cras elementum ultrices diam.")
#     ).serialize()

#     with client.sign_tx(path=path, transaction=transaction):
#         scenario_navigator.review_approve()

#     response = client.get_async_response().data
#     _, der_sig, _ = unpack_sign_tx_response(response)
#     assert check_signature_validity(public_key, der_sig, transaction)


# # Transaction signature refused test
# # The test will ask for a transaction signature that will be refused on screen
# def test_sign_tx_refused(backend, scenario_navigator):
#     # Use the app interface instead of raw interface
#     client = BoilerplateCommandSender(backend)
#     path: str = "m/44'/919'/0'/0/0"

#     rapdu = client.get_public_key(path=path)
#     _, pub_key, _, _ = unpack_get_public_key_response(rapdu.data)

#     transaction = Transaction(
#         nonce=1,
#         to="0xde0b295669a9fd93d5f28d9ec85e40f4cb697bae",
#         value=666,
#         memo="This transaction will be refused by the user"
#     ).serialize()

#     with pytest.raises(ExceptionRAPDU) as e:
#         with client.sign_tx(path=path, transaction=transaction):
#             scenario_navigator.review_reject()

#     # Assert that we have received a refusal
#     assert e.value.status == Errors.SW_DENY
#     assert len(e.value.data) == 0
