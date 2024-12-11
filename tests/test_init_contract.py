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
def test_init_contract(
    backend, firmware, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    path = "m/1105/0/0/0/0/2/0/0"
    header_and_type = bytes.fromhex(
        "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da701"
    )
    amount = 0xFFFFFFFFFFFFFFFF
    module_ref = bytes.fromhex(
        "a00000000000000000000000000000000000000000000000000000000000000a"
    )
    name = bytes.fromhex("696e69745f5465737420436f6e7472616374")
    params = bytes.fromhex("0102030405060708090a")

    with client.init_contract(
        path=path,
        header_and_type=header_and_type,
        amount=amount,
        module_ref=module_ref,
        name=name,
        params=params,
    ):
        navigate_until_text_and_compare(
            firmware, navigator, "Sign", default_screenshot_path, test_name
        )
    response = client.get_async_response()
    print(response.data.hex())
    assert response.status == 0x9000
    assert response.data == bytes.fromhex(
        "ca0e947d521063cc40c3bf8a65dd05a6bb66c799957417f94123c0642162020fed8dd80cd9aadd51f24d697bb9bcce26b72115dbd80bfe893a8395b54f8bb10c"
    )
