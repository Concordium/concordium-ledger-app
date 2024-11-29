import pytest

from application_client.boilerplate_command_sender import BoilerplateCommandSender, Errors
from application_client.boilerplate_response_unpacker import unpack_get_public_key_response
from ragger.bip import calculate_public_key_and_chaincode, CurveChoice
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns


# TODO: remove this test
# # In this test we check that the GET_PUBLIC_KEY works in non-confirmation mode
# def test_get_public_key_no_confirm(backend):
#     for path in ["m/44'/919'/0'/0/0", "m/44'/919'/0/0/0", "m/44'/919'/911'/0/0", "m/44'/919'/255/255/255", "m/44'/919'/2147483647/0/0/0/0/0/0/0",
#                  "m/1105'/0'/0'/0/0", "m/1105'/0'/0/0/0", "m/1105'/0'/911'/0/0", "m/1105'/0'/255/255/255", "m/1105'/0'/2147483647/0/0/0/0/0/0/0"]:
#         client = BoilerplateCommandSender(backend)
#         response = client.get_public_key(path=path).data
#         _, public_key, _, chain_code = unpack_get_public_key_response(response)

#         ref_public_key, ref_chain_code = calculate_public_key_and_chaincode(CurveChoice.Secp256k1, path=path)
#         assert public_key.hex() == ref_public_key
#         assert chain_code.hex() == ref_chain_code

# TODO: Write the same 2 tests for New derivation path

# In this test we check that the GET_PUBLIC_KEY works in confirmation mode
def test_get_legacy_public_key_confirm_accepted(backend, scenario_navigator):
    client = BoilerplateCommandSender(backend)
    path = "m/1105/0/0/0/0/2/0/0"
    with client.get_public_key_with_confirmation(path=path):
        scenario_navigator.address_review_approve()

    response = client.get_async_response().data
    print("km------------------|response:", response.hex())
    assert(response.hex() == "2087e16c8269270b1c75b930224df456d2927b80c760ffa77e57dbd738f6399492")


def test_get_signed_legacy_public_key_confirm_accepted(backend, scenario_navigator):
    client = BoilerplateCommandSender(backend)
    path = "m/1105/0/0/0/0/2/0/0"
    with client.get_public_key_with_confirmation(path=path, signPublicKey=True):
        scenario_navigator.address_review_approve()

    response = client.get_async_response().data
    print("km------------------|response:", response.hex())
    assert(response.hex() == "2087e16c8269270b1c75b930224df456d2927b80c760ffa77e57dbd738f6399492403f499aa9fe41f0b911a3cccde3080143f10d108b2ba72343ad70fa03458333a328c542ce0685632b16636cc579fcfe715743c332eff416589631057eb0e08d04")


# # In this test we check that the GET_PUBLIC_KEY in confirmation mode replies an error if the user refuses
# def test_get_public_key_confirm_refused(backend, scenario_navigator):
#     client = BoilerplateCommandSender(backend)
#     path = "m/44'/919'/0'/0/0"

#     with pytest.raises(ExceptionRAPDU) as e:
#         with client.get_public_key_with_confirmation(path=path):
#             scenario_navigator.address_review_reject()

#     # Assert that we have received a refusal
#     assert e.value.status == Errors.SW_DENY
#     assert len(e.value.data) == 0
