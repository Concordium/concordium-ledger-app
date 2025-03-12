import pytest
from bip32 import BIP32, HARDENED_INDEX
from bip_utils import Bip32Slip10Ed25519
from cryptography.hazmat.primitives.asymmetric.ed25519 import Ed25519PrivateKey
from cryptography.hazmat.primitives import serialization
from application_client.boilerplate_command_sender import (
    BoilerplateCommandSender,
    Errors,
)
from application_client.boilerplate_response_unpacker import (
    unpack_get_public_key_response,
)
from ragger.bip import calculate_public_key_and_chaincode, CurveChoice
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns, NavigateWithScenario


# In this test we check that the VERIFY ADDRESS works in confirmation mode
@pytest.mark.active_test_scope
def test_verify_address_confirm_legacy_path_accepted(
    backend,
    scenario_navigator: NavigateWithScenario,
    test_name: str,
    default_screenshot_path: str,
):
    client = BoilerplateCommandSender(backend)
    with client.verify_address(identity_index=0, credential_counter=0):
        scenario_navigator.address_review_approve()

    response = client.get_async_response().status
    assert response == 0x9000


# In this test we check that the VERIFY ADDRESS works in confirmation mode
@pytest.mark.active_test_scope
def test_verify_address_confirm_new_path_accepted(
    backend, scenario_navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.verify_address(identity_index=0, credential_counter=0, idp_index=0):
        scenario_navigator.address_review_approve()

    response = client.get_async_response().status
    assert response == 0x9000


# In this test we check that the VERIFY ADDRESS in confirmation mode replies an error if the user refuses
@pytest.mark.active_test_scope
def test_verify_address_confirm_refused(
    backend, scenario_navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    try:
        with client.verify_address(identity_index=0, credential_counter=0, idp_index=0):
            scenario_navigator.address_review_reject()

    except ExceptionRAPDU as e:
        response = e.status

    assert response == 0x6985
