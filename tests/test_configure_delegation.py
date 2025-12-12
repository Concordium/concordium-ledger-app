import pytest

from application_client.boilerplate_command_sender import BoilerplateCommandSender
from utils import navigate_until_text_and_compare


# In these tests we send to the device a transaction to sign and validate it on screen
# The transactions are short and will be sent in one chunk
# We will ensure that the displayed information is correct by using screenshots comparison

@pytest.mark.active_test_scope
def test_sign_configure_delegation_capital(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44/919/0/0/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00010000ffffffffffff"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_delegation(path=path, transaction=transaction):
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
        == "91f6646bed5597d32c9ecbb31795b9ecb93963063bd804549e66c66eafa44b8eb17984b11c5770a302d0dfb0acb9f821e85a1e60bd1485f09fa6391a40118d00"
    )

@pytest.mark.active_test_scope
def test_sign_configure_delegation_stop_delegation(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44/919/0/0/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00010000000000000000"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_delegation(path=path, transaction=transaction):
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
        == "b68d17594f23337afa2778ffb7a784a1c3986e216f6ad5a9d8a3a7b914dd0bf148c096167f441cbce3a7e3870d2985a8144a3eba457f2abe51a340e90363a901"
    )

@pytest.mark.active_test_scope
def test_sign_configure_delegation_restake(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44/919/0/0/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a000201"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_delegation(path=path, transaction=transaction):
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
        == "b981185bdd6a26e9371ae7045eecf7206069d9b3fe350a4a32e23c94f30127785bb33743d959bd5a0548aba9b5dee1bfe83c9061d803d2e0344831a0996c7007"
    )

@pytest.mark.active_test_scope
def test_sign_configure_delegation_target(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44/919/0/0/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00040100000000abcdefff"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_delegation(path=path, transaction=transaction):
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
        == "b639ca39861fc3cd9b569b16a778dc69389c9e70bc536cc0e0c4bc70751e3d8239f59c7298fd6b5a3648081a01ce232bc32edabf2b9fffdaf8a677bc2930aa0e"
    )

@pytest.mark.active_test_scope
def test_sign_configure_delegation_capital_target(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44/919/0/0/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00050000ffffffffffff0100000000abcdefff"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_delegation(path=path, transaction=transaction):
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
        == "7577ec2c11776977e15197954ff48a9df675b7e1ecd778d4ec2de8bb31c1b04fd3b5a65bc9711ce8d1ffc9ee1dd8333f235f7d43c8ca27e1e3f02d9e4d9b3508"
    )

@pytest.mark.active_test_scope
def test_sign_configure_delegation_capital_restake_target(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44/919/0/0/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a00070000ffffffffffff010100000000abcdefff"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_delegation(path=path, transaction=transaction):
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
        == "6b5f0920d7d5abee49148514e67d2054e844c109e8e8cf3f4ce7fce69a71d0d61be185d3d90fed8c13e0cc3c87db972a8635ae9455550ff4ffa57dfef12cdb08"
    )

@pytest.mark.active_test_scope
def test_sign_configure_delegation_passive_delegation(
    backend, navigator, default_screenshot_path, test_name
):
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44/919/0/0/0/0"

    # Create the transaction that will be sent to the device for signing
    transaction = "20a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000000000000a0000000000000064000000290000000063de5da71a000400"
    transaction = bytes.fromhex(transaction)

    # Send the sign device instruction.
    # As it requires on-screen validation, the function is asynchronous.
    # It will yield the result when the navigation is done
    with client.sign_configure_delegation(path=path, transaction=transaction):
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
        == "374719c3038f1fe8f7edd8254b6e79e0c91dc3ed5cc7c344bb3cd906068b5b9da3ef19c70b52e39fcf1e3697670769a6c8a751525d28a2ea4a75d9201191d906"
    )
