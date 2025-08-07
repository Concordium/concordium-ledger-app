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
def test_register_data(
    backend, firmware, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    path = "m/1105/0/0/0/0/2/0/0"
    header_and_type = bytes.fromhex(
        "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da715"
    )
    data = bytes.fromhex("6474657374")

    # Send the first part of the data
    with client.register_data_part_1(
        path=path,
        header_and_type=header_and_type,
        data_length=len(data),
    ):
        navigate_until_text_and_compare(
            firmware,
            navigator,
            "Continue",
            default_screenshot_path,
            test_name + "_1",
            True,
            False,
            NavInsID.USE_CASE_CHOICE_CONFIRM,
        )

    response = client.get_async_response()
    print(response.data.hex())
    assert response.status == 0x9000

    # Send the second part of the data
    with client.register_data_part_2(data):
        if firmware.is_nano:
            navigator.navigate_and_compare(
                default_screenshot_path,
                test_name + "_2",
                instructions_builder(1, backend) + [NavInsID.BOTH_CLICK],
                10,  # Timeout
                False,
                True,
            )
        else:
            navigate_until_text_and_compare(
                firmware,
                navigator,
                "Sign transaction",
                default_screenshot_path,
                test_name + "_2",
                True,
                False,
            )
    response = client.get_async_response()
    print(response.data.hex())
    assert response.status == 0x9000
    assert response.data == bytes.fromhex(
        "a410e856c8942767e5af88c3992013a2e788584d9c69141271400222978b57f5b86c8d3a0127b9d521a00c8e8b68ca7c4937da0f1ace27860765d9b0de4ffe08"
    )
