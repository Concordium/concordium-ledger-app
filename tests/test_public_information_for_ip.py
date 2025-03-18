import pytest

from application_client.boilerplate_command_sender import BoilerplateCommandSender
from utils import navigate_until_text_and_compare
from ragger.navigator import NavInsID


# In these tests we send to the device a transaction to sign and validate it on screen
# The transactions are short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison


def create_right_clicks_and_confirm(n: int) -> list:
    """Create a list of n right click instructions followed by both click"""
    instructions = [NavInsID.RIGHT_CLICK] * n
    instructions.append(NavInsID.BOTH_CLICK)
    return instructions


@pytest.mark.active_test_scope
def test_sign_public_information_for_ip(
    backend, firmware, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)

    # Create the transaction that will be sent to the device for signing
    data_1 = "0800000451000000000000000000000000000000000000000200000000000000008196e718f392ec8d07216b22b555cbb71bcee88037566d3f758b9786b945e3b614660f4bf954dbe57bc2304e5a863d2e89a1f69196a1d0423f4936aa664da95de16f40a639dba085073c5a7c8e710c2a402136cc89a39c12ed044e1035649c0f03"
    data_1 = bytes.fromhex(data_1)

    data_2 = "0000b6bc751f1abfb6440ff5cce27d7cdd1e7b0b8ec174f54de426890635b27e7daf"
    data_2 = bytes.fromhex(data_2)

    data_3 = "000146a3e38ddf8b493be6e979034510b91db5448da9cba48c106139c288d658a004"
    data_3 = bytes.fromhex(data_3)

    data_4 = "000271d5f16bc3be249043dc0f9e20b4872f5c3477bf2f285336609c5b0873ab3c9c"
    data_4 = bytes.fromhex(data_4)

    data_5 = "02"
    data_5 = bytes.fromhex(data_5)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_public_information_for_ip_part_1(chunks=[data_1, data_2]):
        if firmware.is_nano:
            navigate_until_text_and_compare(
                firmware,
                navigator,
                "Continue",
                default_screenshot_path,
                test_name + "_1",
                False,
                False,
            )
        else:
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
    with client.sign_public_information_for_ip_part_2(chunks=[data_3]):
        if firmware.is_nano:
            navigate_until_text_and_compare(
                firmware,
                navigator,
                "Continue",
                default_screenshot_path,
                test_name + "_2",
                False,
                False,
            )
        else:
            navigate_until_text_and_compare(
                firmware,
                navigator,
                "Continue",
                default_screenshot_path,
                test_name + "_2",
                True,
                False,
                NavInsID.USE_CASE_CHOICE_CONFIRM,
            )

    with client.sign_public_information_for_ip_part_3(chunks=[data_4, data_5]):
        navigate_until_text_and_compare(
            firmware,
            navigator,
            "Sign identity",
            default_screenshot_path,
            test_name + "_3",
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=False,
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "a35ed536e3473504011d9d13e0e86e0f02512c386809ad442d8d16cd6417e71f32e98c7973f52b580425173481a3ea4f216c775377cf82fc307c95215f64ac0d"
    )
