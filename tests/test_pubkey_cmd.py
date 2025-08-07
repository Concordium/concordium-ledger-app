import pytest

from application_client.boilerplate_command_sender import (
    BoilerplateCommandSender,
    Errors,
)
from application_client.boilerplate_response_unpacker import (
    unpack_get_public_key_response,
)
from ragger.bip import calculate_public_key_and_chaincode, CurveChoice
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NavIns
from ragger.firmware import Firmware
from utils import navigate_until_text_and_compare

nano_accept_instructions = [
    NavInsID.BOTH_CLICK,
    NavInsID.RIGHT_CLICK,
    NavInsID.RIGHT_CLICK,
    NavInsID.BOTH_CLICK,
]

nano_refuse_instructions = [
    NavInsID.RIGHT_CLICK,
    NavInsID.BOTH_CLICK,
]

flex_stax_accept_instructions = [
    NavInsID.SWIPE_CENTER_TO_LEFT,
    NavInsID.USE_CASE_CHOICE_CONFIRM,
    NavInsID.SWIPE_CENTER_TO_LEFT,
    NavInsID.USE_CASE_CHOICE_CONFIRM,
    NavInsID.WAIT_FOR_HOME_SCREEN,
]

flex_stax_refuse_instructions = [
    NavInsID.SWIPE_CENTER_TO_LEFT,
    NavInsID.USE_CASE_CHOICE_REJECT,
    NavInsID.WAIT_FOR_HOME_SCREEN,
]

# In this test we check that the GET_PUBLIC_KEY works in confirmation mode
@pytest.mark.active_test_scope
def test_get_legacy_public_key_confirm_accepted(
    backend, navigator, firmware, default_screenshot_path, test_name, scenario_navigator
):
    client = BoilerplateCommandSender(backend)
    path = "m/1105/0/0/0/0/2/0/0"
    if firmware.is_nano:
        instructions = nano_accept_instructions
    else:
        instructions = flex_stax_accept_instructions

    with client.get_public_key_with_confirmation(path=path):
        navigator.navigate_and_compare(
            default_screenshot_path,
            test_name,
            instructions,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=False,
        )

    response = client.get_async_response().data
    print("km------------------|response:", response.hex())
    assert (
        response.hex()
        == "87e16c8269270b1c75b930224df456d2927b80c760ffa77e57dbd738f6399492"
    )


# In this test we check that the GET_PUBLIC_KEY works in confirmation mode with signing
@pytest.mark.active_test_scope
def test_get_signed_legacy_public_key_confirm_accepted(
    backend, navigator, firmware, default_screenshot_path, test_name
):
    client = BoilerplateCommandSender(backend)
    path = "m/1105/0/0/0/0/2/0/0"
    if firmware.is_nano:
        instructions = nano_accept_instructions
    else:
        instructions = flex_stax_accept_instructions

    with client.get_public_key_with_confirmation(path=path, signPublicKey=True):
        navigator.navigate_and_compare(
            default_screenshot_path,
            test_name,
            instructions,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=False,
        )

    response = client.get_async_response().data
    print("km------------------|response:", response.hex())
    assert (
        response.hex()
        == "87e16c8269270b1c75b930224df456d2927b80c760ffa77e57dbd738f63994923f499aa9fe41f0b911a3cccde3080143f10d108b2ba72343ad70fa03458333a328c542ce0685632b16636cc579fcfe715743c332eff416589631057eb0e08d04"
    )


# In this test we check that the GET_PUBLIC_KEY works in confirmation mode with signing for governance key
@pytest.mark.active_test_scope
def test_get_signed_legacy_governance_public_key_confirm_accepted(
    backend, navigator, firmware, default_screenshot_path, test_name
):
    client = BoilerplateCommandSender(backend)
    path = "m/1105/0/1/0/0"
    if firmware.is_nano:
        instructions = nano_accept_instructions
    else:
        instructions = flex_stax_accept_instructions

    with client.get_public_key_with_confirmation(path=path, signPublicKey=True):
        navigator.navigate_and_compare(
            default_screenshot_path,
            test_name,
            instructions,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=False,
        )

    response = client.get_async_response().data
    print("km------------------|response:", response.hex())
    assert (
        response.hex()
        == "91fcf639f03a8e1c00ab0837383728c9a105df9d44c293b2436dddd7213bee1c62cf20d6c17d1971e66808d325ce1fed188b26b0d543de9f25e5a1c5e46d979cbd2ab98bc4213159883837b9fffa67d43dc5bcbc7b694d164feea777abc4a30d"
    )


# In this test we check that the GET_PUBLIC_KEY works in confirmation mode with signing
@pytest.mark.active_test_scope
def test_get_signed_new_public_key_confirm_accepted(
    backend, navigator, firmware, default_screenshot_path, test_name
):
    client = BoilerplateCommandSender(backend)
    path = "m/44/919/0/0/0"
    if firmware.is_nano:
        instructions = nano_accept_instructions
    else:
        instructions = flex_stax_accept_instructions

    with client.get_public_key_with_confirmation(path=path, signPublicKey=True):
        navigator.navigate_and_compare(
            default_screenshot_path,
            test_name,
            instructions,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=False,
        )

    response = client.get_async_response().data
    print("km------------------|response:", response.hex())
    assert (
        response.hex()
        == "e31d69e500b0f83983fb6080aaa46129cf7c70e27d59b1aae9820b1d03f9840252c415c2552d81fde03a9aef6bba24325711a5924b417d79324f60ef67466a017542c6423387fd0d7679cab784d8178bf15e10eb4cb2eef944d47611682c930c"
    )

# In this test we check that the GET_PUBLIC_KEY in confirmation mode replies an error if the user refuses
@pytest.mark.active_test_scope
def test_get_public_key_confirm_refused(backend, firmware, navigator, default_screenshot_path, test_name):
    client = BoilerplateCommandSender(backend)
    path = "m/44'/919'/0'/0/0"
    if firmware.is_nano:
            instructions = nano_refuse_instructions
    else:
        instructions = flex_stax_refuse_instructions

    with pytest.raises(ExceptionRAPDU) as e:
        with client.get_public_key_with_confirmation(path=path):
            navigator.navigate_and_compare(
            default_screenshot_path,
            test_name,
            instructions,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=False,
        )

    # Assert that we have received a refusal
    assert e.value.status == Errors.SW_DENY
    assert len(e.value.data) == 0
