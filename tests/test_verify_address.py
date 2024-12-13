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
from utils import instructions_builder


nbgl_instructions_address_confirmation = [
    NavInsID.SWIPE_CENTER_TO_LEFT,
    NavInsID.SWIPE_CENTER_TO_LEFT,
    NavInsID.USE_CASE_CHOICE_CONFIRM,
]
nbgl_instructions_address_confirmation_reject = [
    NavInsID.SWIPE_CENTER_TO_LEFT,
    NavInsID.SWIPE_CENTER_TO_LEFT,
    NavInsID.USE_CASE_CHOICE_REJECT,
    NavInsID.USE_CASE_CHOICE_CONFIRM,
]
bagl_instructions_address_confirmation = [
    NavInsID.RIGHT_CLICK,
    NavInsID.RIGHT_CLICK,
    NavInsID.RIGHT_CLICK,
    NavInsID.BOTH_CLICK,
]

bagl_instructions_address_confirmation_reject = [
    NavInsID.RIGHT_CLICK,
    NavInsID.RIGHT_CLICK,
    NavInsID.RIGHT_CLICK,
    NavInsID.BOTH_CLICK,
]


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
        instructions = []
        if backend.firmware.device.startswith(("stax", "flex")):
            instructions.extend(nbgl_instructions_address_confirmation)
        else:
            instructions.extend(bagl_instructions_address_confirmation)

        scenario_navigator.navigator.navigate_and_compare(
            default_screenshot_path,
            test_name,
            instructions,
        )

    response = client.get_async_response().status
    assert response == 0x9000


# In this test we check that the VERIFY ADDRESS works in confirmation mode
@pytest.mark.active_test_scope
def test_verify_address_confirm_new_path_accepted(
    backend, scenario_navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.verify_address(identity_index=0, credential_counter=0, idp_index=0):
        instructions = []
        if backend.firmware.device.startswith(("stax", "flex")):
            instructions.extend(nbgl_instructions_address_confirmation)
        else:
            instructions.extend(instructions_builder(2, backend))

        scenario_navigator.navigator.navigate_and_compare(
            default_screenshot_path, test_name, instructions
        )

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
            instructions = []
            if backend.firmware.device.startswith(("stax", "flex")):
                instructions.extend(nbgl_instructions_address_confirmation_reject)
            else:
                instructions.extend(bagl_instructions_address_confirmation_reject)

            scenario_navigator.navigator.navigate_and_compare(
                default_screenshot_path,
                test_name,
                instructions,
            )
    except ExceptionRAPDU as e:
        response = e.status

    assert response == 0x6985
