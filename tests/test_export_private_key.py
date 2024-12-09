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


def test_export_private_key(
    backend, firmware, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key(export_type="standard", identity_index=0):
        navigate_until_text_and_compare(
            firmware,
            navigator,
            "Approve",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=False,
            screen_change_after_last_instruction=False,
        )
