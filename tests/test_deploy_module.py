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
from utils import navigate_until_text_and_compare, instructions_builder


@pytest.mark.active_test_scope
def test_credential_deployment(
    backend, firmware, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    path = "m/1105/0/0/0/0/2/0/0"
    header_and_type = bytes.fromhex(
        "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da700"
    )
    version = 1
    source = b"source"

    with client.credential_deployment(
        path=path, header_and_type=header_and_type, version=version, source=source
    ):
        navigate_until_text_and_compare(
            firmware, navigator, "Sign", default_screenshot_path, test_name
        )
