import pytest

from application_client.boilerplate_command_sender import BoilerplateCommandSender
from utils import navigate_until_text_and_compare
from ragger.navigator import NavInsID

url = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"

signVerifyKey = "7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b"
signVerifyKeyProof = "a47cdf9133572e9ad5c02c3a7ffd1d05db7bb98860d918092454146153d62788f224c0157c65853ed4a0245ab3e0a593a3f85fa81cc4cb99eeaa643bfc793eab"
electionVerifyKey = "32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828"
electionVerifyKeyProof = "01fc695a8c51d4599cbe032a39832ad49bab900d88105b01d025b760b0d0d555b8c828f2d8fe29cc78c6307d979e6358b8bba9cf4d8200f272cc85b2a3813eff"
aggregationVerifyKey = "7873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96b32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082832f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c0828"
aggregationVerifyKeyProof = "957aec4b2b7ed979ba2079d62246d135aefd61e7f46690c452fec8bcbb593481e229f6f1968194a09cf612490887e71d96730e2d852201e53fec9c89d36f8a90"


# In these tests we send to the device a transaction to sign and validate it on screen
# The transactions are short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison


@pytest.mark.active_test_scope
def test_sign_configure_baker_capital_restake_open_status_and_keys(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)

    # Create the transaction that will be sent to the device for signing
    transaction = "0000FFFFFFFFFFFF01027873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96ba47cdf9133572e9ad5c02c3a7ffd1d05db7bb98860d918092454146153d62788f224c0157c65853ed4a0245ab3e0a593a3f85fa81cc4cb99eeaa643bfc793eab32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082801fc695a8c51d4599cbe032a39832ad49bab900d88105b01d025b760b0d0d555b8c828f2d8fe29cc78c6307d979e6358b8bba9cf4d8200f272cc85b2a3813eff"
    transaction = bytes.fromhex(transaction)

    bitmap = "000f"
    bitmap = bytes.fromhex(bitmap)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_baker(
        aggregation_key=bytes.fromhex(aggregationVerifyKey + aggregationVerifyKeyProof),
        transaction=transaction,
        bitmap=bitmap,
    ):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "d9dbbab9e659c43409fe9cfcd9d18f3f0582656a1bffb420a875c6ef015a7c93d0d0fb34f2131db973647b60cafb745d0f4666d5e26e17b445c636e59a6a5208"
    )


@pytest.mark.active_test_scope
def test_sign_configure_baker_stop_baking(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)

    # Create the transaction that will be sent to the device for signing
    transaction = "0000000000000000"
    transaction = bytes.fromhex(transaction)

    bitmap = "0001"
    bitmap = bytes.fromhex(bitmap)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_baker(
        transaction=transaction,
        bitmap=bitmap,
    ):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "3ca6549799f2f6ba3edcb66c1fc50330bdfe3b1f3df999178a040cf2ae4a68f88630808a4458b72e7aa4ace625e12c087530e976d1941acdaa8a719c4b034401"
    )


@pytest.mark.active_test_scope
def test_sign_configure_baker_capital_restake_open_status_without_keys(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)

    # Create the transaction that will be sent to the device for signing
    transaction = "0000FFFFFFFFFFFF0102"
    transaction = bytes.fromhex(transaction)

    bitmap = "0007"
    bitmap = bytes.fromhex(bitmap)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_baker(
        transaction=transaction,
        bitmap=bitmap,
    ):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "dbcb60773c48e18ffa34c0702db87fbb5998a519cbb7302ff57025298a56a3e869cac88bcf571befa702123e1c0dd58ff75e0d67578e9ea8fec3737da9914009"
    )


@pytest.mark.active_test_scope
def test_sign_configure_baker_only_keys(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)

    # Create the transaction that will be sent to the device for signing
    transaction = (
        signVerifyKey + signVerifyKeyProof + electionVerifyKey + electionVerifyKeyProof
    )
    transaction = bytes.fromhex(transaction)

    bitmap = "0008"
    bitmap = bytes.fromhex(bitmap)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_baker(
        aggregation_key=bytes.fromhex(aggregationVerifyKey + aggregationVerifyKeyProof),
        transaction=transaction,
        bitmap=bitmap,
    ):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "9f38c0162cd68b61ba74f6b421b2d11970fa9c3de712d604387a340f289d17783d9832e9863ac916ad5a0c6cead51f0902e9d2fb7486d0f7988b148f56afb100"
    )


@pytest.mark.active_test_scope
def test_sign_configure_baker_url_only(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)

    # Create the transaction that will be sent to the device for signing
    url_bytes = url.encode("utf-8")

    bitmap = "0010"
    bitmap = bytes.fromhex(bitmap)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_baker_url(
        url=url_bytes,
        bitmap=bitmap,
    ):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "d9582b9f03d31562024145b37f536942dcd6111c553630a74ea5c38a379fd16289de8cbf7523731a1207c74362f09ea5804d601b1a472de109c576c431f75707"
    )


@pytest.mark.active_test_scope
def test_sign_configure_baker_commission_rate_only(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)

    # Create the transaction that will be sent to the device for signing
    url_bytes = url.encode("utf-8")

    bitmap = "00E0"
    bitmap = bytes.fromhex(bitmap)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_baker_commission_rate(
        bitmap=bitmap,
        transaction_fee=True,
        baking_reward=True,
        finalization_reward=True,
    ):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "2a6f49a786b62514d89bfc0e354689e1af2f3d3194b18b93433abd032e518fb72c55501fb5605402bf5a84b88f74d30a46ccb6a05907cb78144e93363b133205"
    )


@pytest.mark.active_test_scope
def test_sign_configure_baker_suspended_only(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    bitmap = "0100"
    bitmap = bytes.fromhex(bitmap)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_baker_suspended(
        bitmap=bitmap, suspended=False, is_called_first=True
    ):
        # Validate the on-screen request by performing the navigation appropriate for this device
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name
        )

    # The device as yielded the result, parse it and ensure that the signature is correct
    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "ebc5ded8d20f43387615a40818e5890558008dd8c9a596cc4ecd007b3ec7923b918dc428b5a37a411c4f9c12a9df7c4d2d6edf3c0793159925720e7bb7078a08"
    )


@pytest.mark.active_test_scope
def test_sign_configure_baker_all_parameters(
    backend, navigator, default_screenshot_path, test_name
):

    client = BoilerplateCommandSender(backend)

    # Create the transaction with all keys
    transaction = "0000FFFFFFFFFFFF01027873cd57848d7aea7be03fbb3f1e8b9e69987fc73f13e473356776a16f26c96ba47cdf9133572e9ad5c02c3a7ffd1d05db7bb98860d918092454146153d62788f224c0157c65853ed4a0245ab3e0a593a3f85fa81cc4cb99eeaa643bfc793eab32f892fb3d0dc6138976b6848259cf730e37fa4a61a659c782ec6def978c082801fc695a8c51d4599cbe032a39832ad49bab900d88105b01d025b760b0d0d555b8c828f2d8fe29cc78c6307d979e6358b8bba9cf4d8200f272cc85b2a3813eff"

    # URL from the existing test
    url_bytes = url.encode("utf-8")

    # Bitmap indicating all features are enabled (keys, url, and commission rates)
    bitmap = "01FF"

    with client.sign_configure_baker(
        transaction=bytes.fromhex(transaction),
        bitmap=bytes.fromhex(bitmap),
        aggregation_key=bytes.fromhex(aggregationVerifyKey + aggregationVerifyKeyProof),
    ):
        if backend.device.is_nano:
            navigate_until_text_and_compare(
                backend,
                navigator,
                "Continue",
                default_screenshot_path,
                test_name + "_1",
                False,
                False,
            )
        else:
            navigate_until_text_and_compare(
                backend,
                navigator,
                "Continue",
                default_screenshot_path,
                test_name + "_1",
                True,
                False,
                NavInsID.USE_CASE_CHOICE_CONFIRM,
            )
    with client.sign_configure_baker_url(
        url=url_bytes, bitmap=bitmap, is_called_first=False
    ):
        if backend.device.is_nano:
            navigate_until_text_and_compare(
                backend,
                navigator,
                "Continue",
                default_screenshot_path,
                test_name + "_2",
                False,
                False,
            )
        else:
            navigate_until_text_and_compare(
                backend,
                navigator,
                "Continue",
                default_screenshot_path,
                test_name + "_2",
                True,
                False,
                NavInsID.USE_CASE_CHOICE_CONFIRM,
            )
    with client.sign_configure_baker_commission_rate(
        bitmap=bitmap,
        transaction_fee=True,
        baking_reward=True,
        finalization_reward=True,
        is_called_first=False,
    ):
        navigate_until_text_and_compare(
            backend,
            navigator,
            "Continue",
            default_screenshot_path,
            test_name + "_3",
            True,
            False,
            NavInsID.USE_CASE_CHOICE_CONFIRM,
        )
    with client.sign_configure_baker_suspended(
        bitmap=bitmap,
        suspended=True,
        is_called_first=False,
    ):
        navigate_until_text_and_compare(
            backend, navigator, "Sign", default_screenshot_path, test_name + "_4"
        )

    response = client.get_async_response().data
    response_hex = response.hex()
    print("response", response_hex)
    assert (
        response_hex
        == "bba026c23bae21bf5deb23cab3f6ec57ccbe2fb504324dbfe28298ea9eb0a1874626db70d911c0922348616862a7eadac7b479dd181a706e31246ceab1a5f604"
    )
