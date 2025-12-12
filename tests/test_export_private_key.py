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
from utils import navigate_until_text_and_compare

# Legacy Path


@pytest.mark.active_test_scope
def test_export_standard_private_key_legacy_path(
    backend, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key_legacy(export_type="standard", identity_index=0):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Accept",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=True,
        )
    result = client.get_async_response()

    assert result.data == bytes.fromhex(
        "48235b90248b6e552d59bf8b533292d25c5afd1f8e1ad5d1e00478794642ba38"
    )


@pytest.mark.active_test_scope
def test_export_recovery_private_key_legacy_path(
    backend, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key_legacy(export_type="recovery", identity_index=0):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Accept",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=True,
        )
    result = client.get_async_response()

    assert result.data == bytes.fromhex(
        "48235b90248b6e552d59bf8b533292d25c5afd1f8e1ad5d1e00478794642ba38"
    )


@pytest.mark.active_test_scope
def test_export_prfkey_and_idcredsed_private_key_legacy_path(
    backend, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key_legacy(
        export_type="prfkey_and_idcredsec", identity_index=0
    ):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Accept",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=True,
        )
    result = client.get_async_response()

    assert result.data == bytes.fromhex(
        "48235b90248b6e552d59bf8b533292d25c5afd1f8e1ad5d1e00478794642ba3802a5a44c0b2e0abcaf313c77fa05f6449c092ad449a081098bd48515bf95e947"
    )


# New Path


@pytest.mark.active_test_scope
def test_export_standard_private_key_new_path(
    backend, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key_new_path(
        "identity_credential_creation", idp_index=0, identity_index=1
    ):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Accept",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=True,
        )
    result = client.get_async_response()

    assert len(result.data) == 33 * 3


@pytest.mark.active_test_scope
def test_export_account_creation_private_key_new_path(
    backend, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key_new_path(
        "account_creation", idp_index=0, identity_index=1, account_index=2
    ):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Accept",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=True,
        )
    result = client.get_async_response()

    assert len(result.data) == 33 * 3


@pytest.mark.active_test_scope
def test_export_id_recovery_private_key_new_path(
    backend, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key_new_path(
        "id_recovery", idp_index=0, identity_index=1
    ):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Accept",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=True,
        )
    result = client.get_async_response()

    assert len(result.data) == 33 * 2


@pytest.mark.active_test_scope
def test_export_account_credential_discovery_private_key_new_path(
    backend, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key_new_path(
        "account_credential_discovery", idp_index=0, identity_index=1
    ):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Accept",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=True,
        )
    result = client.get_async_response()

    assert len(result.data) == 33 * 1


@pytest.mark.active_test_scope
def test_export_creation_of_zk_proof_private_key_new_path(
    backend, navigator, test_name, default_screenshot_path
):
    client = BoilerplateCommandSender(backend)
    with client.export_private_key_new_path(
        "creation_of_zk_proof", idp_index=0, identity_index=1, account_index=2
    ):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Accept",
            default_screenshot_path,
            test_name,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=True,
        )
    result = client.get_async_response()

    assert len(result.data) == 33 * 1
