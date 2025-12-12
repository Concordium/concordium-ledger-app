import pytest

from application_client.boilerplate_transaction import Transaction
from application_client.boilerplate_command_sender import (
    BoilerplateCommandSender,
    Errors,
    InsType,
)
from application_client.boilerplate_response_unpacker import (
    unpack_get_public_key_response,
    unpack_sign_tx_response,
)
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID
from utils import (
    navigate_until_text_and_compare,
    instructions_builder,
    split_message,
    build_tx_with_payload,
)

MAX_SCHEDULE_PAIRS_IN_ONE_APDU: int = (250 // 16) * 16
MAX_APDU_LEN: int = 255


# This test contains a sequence of PLT operations
# 10 transfers with amount from 12.23 to 12.32 with an increase of 0.01 every transfer
# The display should be in normal format for Stax/Flex and JSON format for Nano
@pytest.mark.active_test_scope
def test_sign_plt_multiple_operations(
    backend, navigator, default_screenshot_path, test_name
):
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0754657374504c54"
        + "00000335"
        + "8AA1687472616E73666572A266616D6F756E74C482211904C769726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904C869726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904C969726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CA69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CB69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CC69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CD69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CE69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CF69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904D069726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7"
    )  # 821 bytes

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data

    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128  # 64 bytes signature


# This test contains a sequence of PLT operations
# 11 transfers with amount from 12.23 to 12.33 with an increase of 0.01 every transfer
# The display should be in JSON format for all devices
@pytest.mark.active_test_scope
def test_sign_plt_multiple_operations_JSON_display(
    backend, navigator, default_screenshot_path, test_name
):
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0754657374504c54"
        + "0000035B"
        + "8DA1687472616E73666572A266616D6F756E74C482211904C769726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904C869726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904C969726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CA69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CB69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CC69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CD69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CE69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1687472616E73666572A266616D6F756E74C482211904CF69726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7A1646D696E74A166616D6F756E74C482211904C7A1657061757365A0A167756E7061757365A0A1687472616E73666572A266616D6F756E74C482211904D069726563697069656E74D99D73A201D99D71A10119039703582020A845815BD43A1999E90FBF971537A70392EB38F89E6BD32B3DD70E1A9551D7"
    )  # 859 bytes

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data

    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128  # 64 bytes signature


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
@pytest.mark.active_test_scope
def test_sign_plt_single_transfer(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0754657374504c540000005381a1687472616e73666572a266616d6f756e74c482211904c769726563697069656e74d99d73a201d99d71a10119039703582020a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7"
    )

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_plt_transaction(path=path, transaction=transaction):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128  # 64 bytes signature

    # assert check_signature_validity(public_key, der_sig, transaction)


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
@pytest.mark.active_test_scope
def test_sign_plt_single_mint(backend, navigator, default_screenshot_path, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0754657374504c540000001581a1646d696e74a166616d6f756e74c482211904c7"
    )

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_plt_transaction(path=path, transaction=transaction):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128  # 64 bytes signature

    # assert check_signature_validity(public_key, der_sig, transaction)


# In this test we send to the device a transaction to sign and validate it on screen
# The transaction is short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison
@pytest.mark.active_test_scope
def test_sign_plt_single_deny(backend, navigator, default_screenshot_path, test_name):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    # The path used for this entire test
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0754657374504c540000004681A16B61646444656E794C697374A166746172676574D99D73A201D99D71A101190397035820C8D4BB7106A96BFA6F069438270BF9748049C24798B13B08F88FC2F46AFB435F"
    )

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_plt_transaction(path=path, transaction=transaction):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128  # 64 bytes signature


# PLT Operation Tests based on examples


# Test empty operations
@pytest.mark.active_test_scope
def test_sign_plt_empty_operations(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with empty operations"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload("1b0474504c540000000180")

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128  # 64 bytes signature


# Test pause operation
@pytest.mark.active_test_scope
def test_sign_plt_pause(backend, navigator, default_screenshot_path, test_name):
    """Test PLT transaction with pause operation"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload("1b0474504c540000000981a1657061757365a0")

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test unpause operation
@pytest.mark.active_test_scope
def test_sign_plt_unpause(backend, navigator, default_screenshot_path, test_name):
    """Test PLT transaction with unpause operation"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload("1b0474504c540000000b81a167756e7061757365a0")

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test add allow list with coininfo
@pytest.mark.active_test_scope
def test_sign_plt_add_allow_list_with_coininfo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with add allow list operation (with coininfo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000004781a16c616464416c6c6f774c697374a166746172676574d99d73a201d99d71a101190397035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test add allow list without coininfo
@pytest.mark.active_test_scope
def test_sign_plt_add_allow_list_no_coininfo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with add allow list operation (no coininfo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000003e81a16c616464416c6c6f774c697374a166746172676574d99d73a1035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test remove allow list with coininfo
@pytest.mark.active_test_scope
def test_sign_plt_remove_allow_list_with_coininfo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with remove allow list operation (with coininfo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000004a81a16f72656d6f7665416c6c6f774c697374a166746172676574d99d73a201d99d71a101190397035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test remove allow list without coininfo
@pytest.mark.active_test_scope
def test_sign_plt_remove_allow_list_no_coininfo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with remove allow list operation (no coininfo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000004181a16f72656d6f7665416c6c6f774c697374a166746172676574d99d73a1035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test add deny list with coininfo
@pytest.mark.active_test_scope
def test_sign_plt_add_deny_list_with_coininfo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with add deny list operation (with coininfo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c5400000046"
        + "81a16b61646444656e794c697374a166746172676574d99d73a201d99d71a101190397035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test add deny list without coininfo
@pytest.mark.active_test_scope
def test_sign_plt_add_deny_list_no_coininfo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with add deny list operation (no coininfo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000003d81a16b61646444656e794c697374a166746172676574d99d73a1035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test remove deny list with coininfo
@pytest.mark.active_test_scope
def test_sign_plt_remove_deny_list_with_coininfo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with remove deny list operation (with coininfo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000004981a16e72656d6f766544656e794c697374a166746172676574d99d73a201d99d71a101190397035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test remove deny list without coininfo
@pytest.mark.active_test_scope
def test_sign_plt_remove_deny_list_no_coininfo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with remove deny list operation (no coininfo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000004081a16e72656d6f766544656e794c697374a166746172676574d99d73a1035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test mint with maximum amount
@pytest.mark.active_test_scope
def test_sign_plt_mint_max_amount(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with mint operation (maximum amount: 18446744073.709551615)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000001b81a1646d696e74a166616d6f756e74c482281bffffffffffffffff"
        # 0xffffffffffffffff = 18446744073709551615
        # but get 5161559073704476448
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test mint with amount 1
@pytest.mark.active_test_scope
def test_sign_plt_mint_amount_one(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with mint operation (amount: 1)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000001381a1646d696e74a166616d6f756e74c4820001"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test mint with very small amount
@pytest.mark.active_test_scope
def test_sign_plt_mint_very_small_amount(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with mint operation (very small amount)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000001481a1646d696e74a166616d6f756e74c48238fe01"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test burn with maximum amount
@pytest.mark.active_test_scope
def test_sign_plt_burn_max_amount(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with burn operation (maximum amount: 18446744073.709551615)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000001c81a1646275726ea166616d6f756e74c48238fe1bffffffffffffffff"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test burn with amount 1
@pytest.mark.active_test_scope
def test_sign_plt_burn_amount_one(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with burn operation (amount: 1)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000001381a1646275726ea166616d6f756e74c4820001"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test burn with very small amount


@pytest.mark.active_test_scope
def test_sign_plt_burn_very_small_amount(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with burn operation (very small amount)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000001481a1646275726ea166616d6f756e74c48238fe01"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test transfer with maximum amount and no memo
@pytest.mark.active_test_scope
def test_sign_plt_transfer_max_amount_no_memo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with transfer operation (max amount, no memo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000005981a1687472616e73666572a266616d6f756e74c482281bffffffffffffffff69726563697069656e74d99d73a201d99d71a101190397035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test transfer with untagged memo
@pytest.mark.active_test_scope
def test_sign_plt_transfer_with_untagged_memo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with transfer operation (amount 50, untagged memo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000006781a1687472616e73666572a3646d656d6f4f6e546869732069732061207465737466616d6f756e74c48200183269726563697069656e74d99d73a201d99d71a101190397035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128


# Test transfer with CBOR memo
@pytest.mark.active_test_scope
def test_sign_plt_transfer_with_cbor_memo(
    backend, navigator, default_screenshot_path, test_name
):
    """Test PLT transaction with transfer operation (small amount, CBOR memo)"""
    client = BoilerplateCommandSender(backend)
    path: str = "m/1105/0/0/0/0/2/0/0"

    transaction = build_tx_with_payload(
        "1b0474504c540000006981a1687472616e73666572a3646d656d6fd8184f6e546869732069732061207465737466616d6f756e74c48229183269726563697069656e74d99d73a201d99d71a101190397035820a26c957377a2461b6d0b9f63e7c9504136181942145e16c926451bbce5502b15"
    )

    with client.sign_plt_transaction(path=path, transaction=transaction):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert len(response_hex) == 128
