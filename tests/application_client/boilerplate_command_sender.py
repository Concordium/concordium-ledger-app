from enum import IntEnum
from typing import Generator, List, Literal, Optional
from contextlib import contextmanager

from ragger.backend.interface import BackendInterface, RAPDU
from ragger.error import ExceptionRAPDU
from ragger.bip import pack_derivation_path
from utils import split_message

MAX_APDU_LEN: int = 255

CLA: int = 0xE0


class P1(IntEnum):
    # Parameter 1 for derivation path type for VERIFY_ADDRESS.
    P1_VERIFY_ADDRESS_LEGACY_PATH = 0x00
    P1_VERIFY_ADDRESS_NEW_PATH = 0x01
    # Parameter 1 for screen confirmation for GET_PUBLIC_KEY.
    P1_CONFIRM = 0x00
    P1_NO_CONFIRM = 0x01
    # Parameter 1 for the scheduled amounts
    P1_SCHEDULED_AMOUNTS = 0x01
    # Parameter 1 for scheduled transfer with memo
    P1_MEMO_SCHEDULED_TRANSFER = 0x03
    P1_INITIAL_SCHEDULED_TRANSFER_WITH_MEMO = 0x02
    # Parameter 1 for export private key
    P1_EXPORT_PRIVATE_KEY = 0x00
    P1_EXPORT_WITH_ALTERNATIVE_DISPLAY = 0x01
    P1_EXPORT_PRFKEY_AND_IDCREDSEC = 0x02
    # Parameter 1 for transfer to public
    P1_INITIAL_TRANSFER_TO_PUBLIC = 0x00
    P1_REMAINING_AMOUNT_TRANSFER_TO_PUBLIC = 0x01
    P1_PROOF_TRANSFER_TO_PUBLIC = 0x02
    # Parameter 1 for register data
    P1_REGISTER_DATA_INITIAL = 0x00
    P1_REGISTER_DATA_PAYLOAD = 0x01
    # Basic P1 for all instructions
    P1_NONE = 0x00

    # Parameter 1 for credential deployment
    P1_INITIAL_PACKET = 0x00  # Sent for 1st packet of the transfer
    P1_VERIFICATION_KEY_LENGTH = 0x0A
    P1_VERIFICATION_KEY = 0x01  # Sent for packets containing a verification key
    P1_SIGNATURE_THRESHOLD = 0x02  # Sent for packet with signature threshold etc
    P1_AR_IDENTITY = 0x03  # Sent for aridentity/encidcredpubshares pair
    P1_CREDENTIAL_DATES = 0x04  # Sent for credential valid to/create at dates
    P1_ATTRIBUTE_TAG = 0x05  # Sent for attribute tag and value length
    P1_ATTRIBUTE_VALUE = 0x06  # Sent for attribute value
    P1_LENGTH_OF_PROOFS = 0x07  # Sent for byte length of proofs
    P1_PROOFS = 0x08  # Sent for proof bytes
    P1_NEW_OR_EXISTING = 0x09  # Sent for new/existing credential flag

    # Parameter 1 for deploy module
    P1_DEPLOY_MODULE_INITIAL = 0x00
    P1_DEPLOY_MODULE_SOURCE = 0x01
    # Parameter 1 for init contract
    P1_INIT_CONTRACT_INITIAL = 0x00
    P1_INIT_CONTRACT_NAME = 0x01
    P1_INIT_CONTRACT_PARAMS = 0x02
    # Parameter 1 for update contract
    P1_UPDATE_CONTRACT_INITIAL = 0x00
    P1_UPDATE_CONTRACT_NAME = 0x01
    P1_UPDATE_CONTRACT_PARAMS = 0x02


class P2(IntEnum):
    # Parameter 2 for sign for GET_PUBLIC_KEY.
    P2_SIGN = 0x01
    P2_NO_SIGN = 0x00
    # Parameter 2 for export private key
    P2_EXPORT_BLS_KEY = 0x02
    # Basic P2 for all instructions
    P2_NONE = 0x00
    # # Parameter 2 for last APDU to receive.
    # P2_LAST = 0x00
    # # Parameter 2 for more APDU to receive.
    # P2_MORE = 0x80
    # Parameter 2 for credential deployment
    P2_CREDENTIAL_INITIAL = 0x00  # Initial credential data
    P2_CREDENTIAL_CREDENTIAL_INDEX = 0x01  # Credential index
    P2_CREDENTIAL_CREDENTIAL = 0x02  # Credential data
    P2_CREDENTIAL_ID_COUNT = 0x03  # Number of credential IDs
    P2_CREDENTIAL_ID = 0x04  # Credential ID
    P2_THRESHOLD = 0x05  # Threshold value


class InsType(IntEnum):
    VERIFY_ADDRESS = 0x00
    GET_PUBLIC_KEY = 0x01
    GET_APP_NAME = 0x21
    SIGN_TRANSFER = 0x02
    SIGN_TRANSFER_WITH_SCHEDULE = 0x03
    CREDENTIAL_DEPLOYMENT = 0x04
    EXPORT_PRIVATE_KEY = 0x05
    DEPLOY_MODULE = 0x06
    INIT_CONTRACT = 0x07
    UPDATE_CONTRACT = 0x08
    TRANSFER_TO_PUBLIC = 0x12
    CONFIGURE_DELEGATION = 0x17
    CONFIGURE_BAKER = 0x18
    PUBLIC_INFO_FOR_IP = 0x20
    SIGN_UPDATE_CREDENTIAL = 0x31
    SIGN_TRANSFER_WITH_MEMO = 0x32
    SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO = 0x34
    REGISTER_DATA = 0x35


class Errors(IntEnum):
    SW_DENY = 0x6985
    SW_WRONG_P1P2 = 0x6A86
    SW_WRONG_DATA_LENGTH = 0x6A87
    SW_INS_NOT_SUPPORTED = 0x6D00
    SW_CLA_NOT_SUPPORTED = 0x6E00
    SW_WRONG_RESPONSE_LENGTH = 0xB000
    SW_DISPLAY_BIP32_PATH_FAIL = 0xB001
    SW_DISPLAY_ADDRESS_FAIL = 0xB002
    SW_DISPLAY_AMOUNT_FAIL = 0xB003
    SW_WRONG_TX_LENGTH = 0xB004
    SW_TX_PARSING_FAIL = 0xB005
    SW_TX_HASH_FAIL = 0xB006
    SW_BAD_STATE = 0xB007
    SW_SIGNATURE_FAIL = 0xB008


# pylint: disable=too-many-public-methods
# pylint: disable=too-many-locals
# pylint: disable=too-many-lines
class BoilerplateCommandSender:
    def __init__(self, backend: BackendInterface) -> None:
        self.backend = backend

    def get_app_name(self) -> RAPDU:
        return self.backend.exchange(
            cla=CLA, ins=InsType.GET_APP_NAME, p1=P1.P1_NONE, p2=P2.P2_NONE, data=b""
        )

    def get_public_key(self, path: str, signPublicKey: bool = False) -> RAPDU:
        return self.backend.exchange(
            cla=CLA,
            ins=InsType.GET_PUBLIC_KEY,
            p1=P1.P1_NO_CONFIRM,
            p2=P2.P2_SIGN if signPublicKey else P2.P2_NO_SIGN,
            data=pack_derivation_path(path),
        )

    @contextmanager
    def get_public_key_with_confirmation(
        self, path: str, signPublicKey: bool = False
    ) -> Generator[None, None, None]:
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.GET_PUBLIC_KEY,
            p1=P1.P1_CONFIRM,
            p2=P2.P2_SIGN if signPublicKey else P2.P2_NO_SIGN,
            data=pack_derivation_path(path),
        ) as response:
            yield response

    @contextmanager
    def verify_address(
        self, identity_index: int, credential_counter: int, idp_index: int = -1
    ) -> Generator[None, None, None]:
        data = b""
        p1 = P1.P1_VERIFY_ADDRESS_LEGACY_PATH

        if idp_index != -1:
            data += idp_index.to_bytes(4, byteorder="big")
            p1 = P1.P1_VERIFY_ADDRESS_NEW_PATH

        data += identity_index.to_bytes(
            4, byteorder="big"
        ) + credential_counter.to_bytes(4, byteorder="big")
        with self.backend.exchange_async(
            cla=CLA, ins=InsType.VERIFY_ADDRESS, p1=p1, p2=P2.P2_NONE, data=data
        ) as response:
            yield response

    @contextmanager
    def sign_simple_transfer(
        self, path: str, transaction: bytes
    ) -> Generator[None, None, None]:
        data = pack_derivation_path(path)
        data += transaction

        index = P1.P1_NONE + 1
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_TRANSFER,
            p1=index,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    @contextmanager
    def sign_simple_transfer_with_memo(
        self, path: str, header_and_to_address: bytes, memo: bytes, amount: bytes
    ) -> Generator[None, None, None]:
        data = pack_derivation_path(path)
        data += header_and_to_address
        print("km------------data", data.hex())
        index = P1.P1_NONE + 1
        # Get memo length in bytes
        memo_length = len(memo)
        print("km------------memo_length", memo_length)
        # memo length has to take 2 bytes
        memo_length_bytes = memo_length.to_bytes(2, byteorder="big")
        print("km------------memo_length_bytes", memo_length_bytes.hex())
        data += memo_length_bytes
        print("km------------data", data.hex())
        self.backend.exchange(
            cla=CLA,
            ins=InsType.SIGN_TRANSFER_WITH_MEMO,
            p1=index,
            p2=P2.P2_NONE,
            data=data,
        )
        index += 1
        memo_chunks = split_message(memo, MAX_APDU_LEN)
        for chunk in memo_chunks:
            self.backend.exchange(
                cla=CLA,
                ins=InsType.SIGN_TRANSFER_WITH_MEMO,
                p1=index,
                p2=P2.P2_NONE,
                data=chunk,
            )
            index += 1
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_TRANSFER_WITH_MEMO,
            p1=index,
            p2=P2.P2_NONE,
            data=amount,
        ) as response:
            yield response

    @contextmanager
    def sign_tx_with_schedule_part_1(
        self, path: str, header_and_to_address: bytes, num_pairs: int
    ) -> Generator[None, None, None]:
        # Send the derivation path, the header, the to address and the number of pairs
        data = pack_derivation_path(path)
        data += header_and_to_address
        data += num_pairs.to_bytes(1, byteorder="big")
        print("km------------data", data.hex())
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_TRANSFER_WITH_SCHEDULE,
            p1=P1.P1_NONE,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    @contextmanager
    def sign_tx_with_schedule_part_2(self, data: bytes) -> Generator[None, None, None]:
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_TRANSFER_WITH_SCHEDULE,
            p1=P1.P1_SCHEDULED_AMOUNTS,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    def sign_tx_with_schedule_and_memo_part_1(
        self, path: str, header_and_to_address: bytes, num_pairs: int, memo_length: int
    ) -> RAPDU:
        data = pack_derivation_path(path)
        data += header_and_to_address
        data += num_pairs.to_bytes(1, byteorder="big")
        memo_length_bytes = memo_length.to_bytes(2, byteorder="big")
        data += memo_length_bytes
        return self.backend.exchange(
            cla=CLA,
            ins=InsType.SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO,
            p1=P1.P1_INITIAL_SCHEDULED_TRANSFER_WITH_MEMO,
            p2=P2.P2_NONE,
            data=data,
        )

    @contextmanager
    def sign_tx_with_schedule_and_memo_part_2(
        self, memo_chunk: bytes
    ) -> Generator[None, None, None]:
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO,
            p1=P1.P1_MEMO_SCHEDULED_TRANSFER,
            p2=P2.P2_NONE,
            data=memo_chunk,
        ) as response:
            yield response

    @contextmanager
    def sign_tx_with_schedule_and_memo_part_3(
        self, data: bytes
    ) -> Generator[None, None, None]:
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_TRANSFER_WITH_SCHEDULE_AND_MEMO,
            p1=P1.P1_SCHEDULED_AMOUNTS,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    @contextmanager
    def sign_configure_delegation(
        self, path: str, transaction: bytes
    ) -> Generator[None, None, None]:
        data = pack_derivation_path(path)
        data += transaction

        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.CONFIGURE_DELEGATION,
            p1=P1.P1_NONE,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    @contextmanager
    def sign_transfer_to_public(
        self, path: str, header_and_type: bytes, data: bytes, proof: bytes
    ) -> Generator[None, None, None]:
        temp_data = pack_derivation_path(path)
        temp_data += header_and_type
        # send the header and type (no display)
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.TRANSFER_TO_PUBLIC,
            p1=P1.P1_INITIAL_TRANSFER_TO_PUBLIC,
            p2=P2.P2_NONE,
            data=temp_data,
        )

        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # send remaining amount [192] + transaction amount [8]
        #          + amount index [1] + proof size [1] (no display)
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.TRANSFER_TO_PUBLIC,
            p1=P1.P1_REMAINING_AMOUNT_TRANSFER_TO_PUBLIC,
            p2=P2.P2_NONE,
            data=data,
        )
        print("km------------temp_response 2", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)

        # send proof in chunks
        proof_chunks = split_message(proof, MAX_APDU_LEN)
        last_chunk = proof_chunks.pop()
        for chunk in proof_chunks:
            temp_response = self.backend.exchange(
                cla=CLA,
                ins=InsType.TRANSFER_TO_PUBLIC,
                p1=P1.P1_PROOF_TRANSFER_TO_PUBLIC,
                p2=P2.P2_NONE,
                data=chunk,
            )
            print("km------------temp_response 3", temp_response)
            if temp_response.status != 0x9000:
                raise ExceptionRAPDU(temp_response.status)
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.TRANSFER_TO_PUBLIC,
            p1=P1.P1_PROOF_TRANSFER_TO_PUBLIC,
            p2=P2.P2_NONE,
            data=last_chunk,
        ) as response:
            yield response

    CONFIGURE_BAKER_HEADER = (
        "08000004510000000000000000000000000000000000000002000000000000000020"
        "a845815bd43a1999e90fbf971537a70392eb38f89e6bd32b3dd70e1a9551d7000000"
        "000000000a0000000000000064000000290000000063de5da719"
    )

    @contextmanager
    def sign_configure_baker(
        self,
        transaction: bytes,
        bitmap: bytes,
        aggregation_key: bytes = b"",
    ) -> Generator[None, None, None]:

        self._configure_baker_part_0(bitmap)

        if aggregation_key:
            # Second exchange - using with to wait for completion
            self.backend.exchange(
                cla=CLA,
                ins=InsType.CONFIGURE_BAKER,
                p1=0x01,
                p2=P2.P2_NONE,
                data=transaction,
            )

            # Final exchange with response yielded to caller
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.CONFIGURE_BAKER,
                p1=0x02,
                p2=P2.P2_NONE,
                data=aggregation_key,
            ) as response:
                yield response
        else:
            # If no aggregation key, yield response from second exchange
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.CONFIGURE_BAKER,
                p1=0x01,
                p2=P2.P2_NONE,
                data=transaction,
            ) as response:
                yield response

    @contextmanager
    def sign_configure_baker_url(
        self, url: bytes, bitmap: bytes, is_called_first: bool = True
    ) -> Generator[None, None, None]:
        def encode_word16(value: int) -> bytes:
            # Ensure the value fits in 16 bits
            if not 0 <= value <= 0xFFFF:
                raise ValueError("Value must be between 0 and 65535 (inclusive)")

            # Convert the integer to a 2-byte big-endian representation
            return value.to_bytes(2, byteorder="big")

        if is_called_first:
            self._configure_baker_part_0(bitmap)

        # Send the URL length first
        url_length = encode_word16(len(url))
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.CONFIGURE_BAKER,
            p1=0x03,
            p2=P2.P2_NONE,
            data=url_length,
        ):
            pass

        # Batch the URL into at most 255 byte chunks
        chunked_url = split_message(url, 255)
        for serialized_url_chunk in chunked_url:
            # Send each chunk and yield the last response
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.CONFIGURE_BAKER,
                p1=0x04,
                p2=P2.P2_NONE,
                data=serialized_url_chunk,
            ) as response:
                if serialized_url_chunk == chunked_url[-1]:  # If this is the last chunk
                    yield response
                else:
                    pass

    @contextmanager
    def sign_configure_baker_commission_rate(
        self,
        bitmap: bytes,
        transaction_fee: bool = False,
        baking_reward: bool = False,
        finalization_reward: bool = False,
        is_called_first: bool = True,
    ) -> Generator[None, None, None]:
        serialized_commission_rates = b""

        if is_called_first:
            self._configure_baker_part_0(bitmap)

        if transaction_fee:
            serialized_commission_rates += bytes.fromhex("0000B0C1")

        if baking_reward:
            baking_reward_commission = bytes.fromhex("0000C001")
            serialized_commission_rates += baking_reward_commission

        if finalization_reward:
            finalization_reward_commission = bytes.fromhex("00000B11")
            serialized_commission_rates += finalization_reward_commission

        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.CONFIGURE_BAKER,
            p1=0x05,  # Special P1 value for commission rates
            p2=P2.P2_NONE,
            data=serialized_commission_rates,
        ) as response:
            yield response

    @contextmanager
    def sign_configure_baker_suspended(
        self,
        bitmap: bytes,
        suspended: bool = False,
        is_called_first: bool = True,
    ) -> Generator[None, None, None]:
        serialized_suspended_status = b""

        if is_called_first:
            self._configure_baker_part_0(bitmap)

        if suspended:
            serialized_suspended_status += bytes.fromhex("01")
        else:
            serialized_suspended_status += bytes.fromhex("00")

        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.CONFIGURE_BAKER,
            p1=0x06,  # Special P1 value for commission rates
            p2=P2.P2_NONE,
            data=serialized_suspended_status,
        ) as response:
            yield response

    def _configure_baker_part_0(
        self,
        bitmap: bytes,
    ) -> None:

        data_p1 = bytes.fromhex(self.CONFIGURE_BAKER_HEADER)
        data_p1 += bitmap

        # First exchange - using with to wait for completion
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.CONFIGURE_BAKER,
            p1=P1.P1_NONE,
            p2=P2.P2_NONE,
            data=data_p1,
        ):
            pass

    @contextmanager
    def export_private_key(
        self,
        export_type: Literal["standard", "recovery", "prfkey_and_idcredsec"],
        identity_index: int,
        idp_index: int = -1,
    ) -> Generator[None, None, None]:
        data = b""
        if export_type == "standard":
            p1 = P1.P1_EXPORT_PRIVATE_KEY
        elif export_type == "recovery":
            p1 = P1.P1_EXPORT_WITH_ALTERNATIVE_DISPLAY
        elif export_type == "prfkey_and_idcredsec":
            p1 = P1.P1_EXPORT_PRFKEY_AND_IDCREDSEC
        else:
            raise ValueError(f"Invalid export type: {export_type}")
        if idp_index != -1:
            data += bytes.fromhex("01")
            data += idp_index.to_bytes(4, byteorder="big")
        else:
            data += bytes.fromhex("00")

        data += identity_index.to_bytes(4, byteorder="big")
        print("km------------data", data.hex())
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.EXPORT_PRIVATE_KEY,
            p1=p1,
            p2=P2.P2_EXPORT_BLS_KEY,
            data=data,
        ) as response:
            yield response

    def credential_deployment_part_1(
        self,
        path: str,
        number_of_keys: int,
    ) -> bool:
        # send derivation path (no display)
        data = pack_derivation_path(path)
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_INITIAL_PACKET,
            p2=P2.P2_NONE,
            data=data,
        )
        print("km--------sent derivation path", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # handle credential deployment keys
        ## send number of keys
        data = number_of_keys.to_bytes(1, byteorder="big")
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_VERIFICATION_KEY_LENGTH,
            p2=P2.P2_NONE,
            data=data,
        )
        print("km--------sent number of keys", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        return True

    @contextmanager
    def credential_deployment_part_2(self, key_index: int, key: bytes):
        key_index = key_index + 1
        data = key_index.to_bytes(1, byteorder="big") + key
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_VERIFICATION_KEY,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    @contextmanager
    def credential_deployment_part_3(
        self,
        last_key: bytes,
        signature_threshold: bytes,
        ar_identity: bytes,
        credential_dates: bytes,
        attribute_tag: bytes,
        attribute_value: bytes,
        proofs: bytes,
        transaction: bytes,
    ) -> Generator[None, None, None]:
        ## send last key (display ?)

        data = (0).to_bytes(1, byteorder="big") + last_key
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_VERIFICATION_KEY,
            p2=P2.P2_NONE,
            data=data,
        )
        print("km--------sent last key", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)

        # send signature threshold
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_SIGNATURE_THRESHOLD,
            p2=P2.P2_NONE,
            data=signature_threshold,
        )
        print("km--------sent signature threshold", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # send ar_identity
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_AR_IDENTITY,
            p2=P2.P2_NONE,
            data=ar_identity,
        )
        print("km--------sent ar_identity", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # send credential dates
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_CREDENTIAL_DATES,
            p2=P2.P2_NONE,
            data=credential_dates,
        )
        print("km--------sent credential dates", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # send attribute tag
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_ATTRIBUTE_TAG,
            p2=P2.P2_NONE,
            data=attribute_tag,
        )
        print("km--------sent attribute tag", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # send attribute value
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_ATTRIBUTE_VALUE,
            p2=P2.P2_NONE,
            data=attribute_value,
        )
        print("km--------sent attribute value", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # send length of proofs
        data = len(proofs).to_bytes(4, byteorder="big")
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_LENGTH_OF_PROOFS,
            p2=P2.P2_NONE,
            data=data,
        )
        print("km--------sent length of proofs", temp_response)
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # send proofs in chunks
        proof_chunks = split_message(proofs, MAX_APDU_LEN)
        for i, chunk in enumerate(proof_chunks):
            temp_response = self.backend.exchange(
                cla=CLA,
                ins=InsType.CREDENTIAL_DEPLOYMENT,
                p1=P1.P1_PROOFS,
                p2=P2.P2_NONE,
                data=chunk,
            )
            print(f"km--------sent proof chunk {i+1}", temp_response)
            if temp_response.status != 0x9000:
                raise ExceptionRAPDU(temp_response.status)
        # send new or existing

        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.CREDENTIAL_DEPLOYMENT,
            p1=P1.P1_NEW_OR_EXISTING,
            p2=P2.P2_NONE,
            data=transaction,
        ) as response:
            print("km--------sent new or existing", response)
            yield response

    @contextmanager
    def credential_update_part_1(
        self,
        data: bytes,
    ) -> Generator[None, None, None]:
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_NONE,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    @contextmanager
    def sign_update_credential_part_2(
        self,
        key_index: bytes,
        number_of_keys: int,
        key: bytes,
    ) -> Generator[None, None, None]:
        # Send all keys
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_NONE,
            p2=P2.P2_CREDENTIAL_CREDENTIAL_INDEX,
            data=key_index,
        ):
            pass
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_VERIFICATION_KEY_LENGTH,
            p2=P2.P2_CREDENTIAL_CREDENTIAL,
            data=bytes.fromhex(f"{number_of_keys:02x}"),
        ):
            pass
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_VERIFICATION_KEY,
            p2=P2.P2_CREDENTIAL_CREDENTIAL,
            data=key,
        ) as response:
            yield response

    @contextmanager
    def sign_update_credential_part_3(
        self,
        signature_threshold: bytes,
        ar_identity: bytes,
        credential_dates: bytes,
        attribute_tag: bytes,
        attribute_value: bytes,
        proof_length: bytes,
        proofs: bytes,
        credential_id_list: List[bytes],
    ) -> Generator[None, None, None]:
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_SIGNATURE_THRESHOLD,
            p2=P2.P2_CREDENTIAL_CREDENTIAL,
            data=signature_threshold,
        ):
            pass

        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_AR_IDENTITY,
            p2=P2.P2_CREDENTIAL_CREDENTIAL,
            data=ar_identity,
        ):
            pass
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_CREDENTIAL_DATES,
            p2=P2.P2_CREDENTIAL_CREDENTIAL,
            data=credential_dates,
        ):
            pass
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_ATTRIBUTE_TAG,
            p2=P2.P2_CREDENTIAL_CREDENTIAL,
            data=attribute_tag,
        ):
            pass
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_ATTRIBUTE_VALUE,
            p2=P2.P2_CREDENTIAL_CREDENTIAL,
            data=attribute_value,
        ):
            pass
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_LENGTH_OF_PROOFS,
            p2=P2.P2_CREDENTIAL_CREDENTIAL,
            data=proof_length,
        ):
            pass
        proof_chunks = split_message(proofs, MAX_APDU_LEN)
        for i, chunk in enumerate(proof_chunks):
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.SIGN_UPDATE_CREDENTIAL,
                p1=P1.P1_PROOFS,
                p2=P2.P2_CREDENTIAL_CREDENTIAL,
                data=chunk,
            ):
                pass
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_NONE,
            p2=P2.P2_CREDENTIAL_ID_COUNT,
            data=bytes.fromhex(f"{len(credential_id_list):02x}"),
        ):
            pass
        for i, credential_id in enumerate(credential_id_list):
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.SIGN_UPDATE_CREDENTIAL,
                p1=P1.P1_NONE,
                p2=P2.P2_CREDENTIAL_ID,
                data=credential_id,
            ) as response:
                if i == len(credential_id_list) - 1:
                    yield response

    @contextmanager
    def sign_update_credential_part_4(
        self,
        threshold: bytes,
    ) -> Generator[None, None, None]:
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.SIGN_UPDATE_CREDENTIAL,
            p1=P1.P1_NONE,
            p2=P2.P2_THRESHOLD,
            data=threshold,
        ) as response:
            yield response

    @contextmanager
    def sign_public_information_for_ip_part_1(
        self, chunks: List[bytes]
    ) -> Generator[None, None, None]:
        # Handle first chunk
        if len(chunks) > 0:
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.PUBLIC_INFO_FOR_IP,
                p1=P1.P1_NONE,
                p2=P2.P2_NONE,
                data=chunks[0],
            ):
                pass

        # Handle middle chunks
        for i in range(1, len(chunks)):
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.PUBLIC_INFO_FOR_IP,
                p1=0x01,
                p2=P2.P2_NONE,
                data=chunks[i],
            ) as response:
                yield response

    @contextmanager
    def sign_public_information_for_ip_part_2(
        self, chunks: List[bytes]
    ) -> Generator[None, None, None]:
        # Handle all chunks
        for i, data in enumerate(chunks):
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.PUBLIC_INFO_FOR_IP,
                p1=0x01,
                p2=P2.P2_NONE,
                data=data,
            ) as response:
                if i == len(chunks) - 1:
                    yield response

    @contextmanager
    def sign_public_information_for_ip_part_3(
        self, chunks: List[bytes]
    ) -> Generator[None, None, None]:
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.PUBLIC_INFO_FOR_IP,
            p1=0x01,
            p2=P2.P2_NONE,
            data=chunks[0],
        ):
            pass
        # Handle last chunk
        if len(chunks) > 1:
            with self.backend.exchange_async(
                cla=CLA,
                ins=InsType.PUBLIC_INFO_FOR_IP,
                p1=0x02,
                p2=P2.P2_NONE,
                data=chunks[-1],
            ) as response:
                yield response

    @contextmanager
    def deploy_module(
        self,
        path: str,
        header_and_type: bytes,
        version: int,
        source: bytes,
    ) -> Generator[None, None, None]:

        if version > 0xFFFFFFFF:
            raise ValueError("Version must be less than 4294967296")
        if len(source) > 0xFFFFFFFF:
            raise ValueError("Source length must be less than 4294967296")

        data = pack_derivation_path(path)
        data += header_and_type
        data += version.to_bytes(4, byteorder="big")
        data += len(source).to_bytes(4, byteorder="big")
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.DEPLOY_MODULE,
            p1=P1.P1_DEPLOY_MODULE_INITIAL,
            p2=P2.P2_NONE,
            data=data,
        )
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)

        source_chunks = split_message(source, MAX_APDU_LEN)
        last_chunk = source_chunks.pop()
        for chunk in source_chunks:
            temp_response = self.backend.exchange(
                cla=CLA,
                ins=InsType.DEPLOY_MODULE,
                p1=P1.P1_DEPLOY_MODULE_SOURCE,
                p2=P2.P2_NONE,
                data=chunk,
            )
            if temp_response.status != 0x9000:
                raise ExceptionRAPDU(temp_response.status)
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.DEPLOY_MODULE,
            p1=P1.P1_DEPLOY_MODULE_SOURCE,
            p2=P2.P2_NONE,
            data=last_chunk,
        ) as response:
            yield response

    @contextmanager
    def init_contract(
        self,
        path: str,
        header_and_type: bytes,
        amount: int,
        module_ref: bytes,
        name: bytes,
        params: bytes,
    ) -> Generator[None, None, None]:

        if amount > 0xFFFFFFFFFFFFFFFF:
            raise ValueError("Amount must be less than 18446744073709551615")
        # Send the initial data
        data = pack_derivation_path(path)
        data += header_and_type
        data += amount.to_bytes(8, byteorder="big")
        data += module_ref
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.INIT_CONTRACT,
            p1=P1.P1_INIT_CONTRACT_INITIAL,
            p2=P2.P2_NONE,
            data=data,
        )
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # Send the name
        data = len(name).to_bytes(2, byteorder="big")
        name_chunks = split_message(name, MAX_APDU_LEN - len(data))
        for i, chunk in enumerate(name_chunks):
            if i == 0:
                data += chunk
            else:
                data = chunk
            temp_response = self.backend.exchange(
                cla=CLA,
                ins=InsType.INIT_CONTRACT,
                p1=P1.P1_INIT_CONTRACT_NAME,
                p2=P2.P2_NONE,
                data=data,
            )
            if temp_response.status != 0x9000:
                raise ExceptionRAPDU(temp_response.status)
        # Send the params
        data = len(params).to_bytes(2, byteorder="big")
        params_chunks = split_message(params, MAX_APDU_LEN - len(data))
        last_chunk = params_chunks.pop()
        for i, chunk in enumerate(params_chunks):
            if i == 0:
                data += chunk
            else:
                data = chunk
            temp_response = self.backend.exchange(
                cla=CLA,
                ins=InsType.INIT_CONTRACT,
                p1=P1.P1_INIT_CONTRACT_PARAMS,
                p2=P2.P2_NONE,
                data=data,
            )
            if temp_response.status != 0x9000:
                raise ExceptionRAPDU(temp_response.status)
        if len(params_chunks) == 0:
            data = len(last_chunk).to_bytes(2, byteorder="big") + last_chunk
        else:
            data = last_chunk
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.INIT_CONTRACT,
            p1=P1.P1_INIT_CONTRACT_PARAMS,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    @contextmanager
    def update_contract(
        self,
        path: str,
        header_and_type: bytes,
        amount: int,
        index: bytes,
        sub_index: bytes,
        name: bytes,
        params: bytes,
    ) -> Generator[None, None, None]:
        if amount > 0xFFFFFFFFFFFFFFFF:
            raise ValueError("Amount must be less than 18446744073709551615")
        # Send the initial data
        data = pack_derivation_path(path)
        data += header_and_type
        data += amount.to_bytes(8, byteorder="big")
        data += index
        data += sub_index
        temp_response = self.backend.exchange(
            cla=CLA,
            ins=InsType.UPDATE_CONTRACT,
            p1=P1.P1_UPDATE_CONTRACT_INITIAL,
            p2=P2.P2_NONE,
            data=data,
        )
        if temp_response.status != 0x9000:
            raise ExceptionRAPDU(temp_response.status)
        # Send the name
        data = len(name).to_bytes(2, byteorder="big")
        name_chunks = split_message(name, MAX_APDU_LEN - len(data))
        for i, chunk in enumerate(name_chunks):
            if i == 0:
                data += chunk
            else:
                data = chunk
            temp_response = self.backend.exchange(
                cla=CLA,
                ins=InsType.UPDATE_CONTRACT,
                p1=P1.P1_UPDATE_CONTRACT_NAME,
                p2=P2.P2_NONE,
                data=data,
            )
            if temp_response.status != 0x9000:
                raise ExceptionRAPDU(temp_response.status)
        # Send the params
        data = len(params).to_bytes(2, byteorder="big")
        params_chunks = split_message(params, MAX_APDU_LEN - len(data))
        last_chunk = params_chunks.pop()
        for i, chunk in enumerate(params_chunks):
            if i == 0:
                data += chunk
            else:
                data = chunk
            temp_response = self.backend.exchange(
                cla=CLA,
                ins=InsType.UPDATE_CONTRACT,
                p1=P1.P1_UPDATE_CONTRACT_PARAMS,
                p2=P2.P2_NONE,
                data=data,
            )
            if temp_response.status != 0x9000:
                raise ExceptionRAPDU(temp_response.status)
        if len(params_chunks) == 0:
            data = len(last_chunk).to_bytes(2, byteorder="big") + last_chunk
        else:
            data = last_chunk
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.UPDATE_CONTRACT,
            p1=P1.P1_UPDATE_CONTRACT_PARAMS,
            p2=P2.P2_NONE,
            data=data,
        ) as response:
            yield response

    @contextmanager
    def register_data_part_1(
        self,
        path: str,
        header_and_type: bytes,
        data_length: int,
    ) -> Generator[None, None, None]:
        temp_data = pack_derivation_path(path)
        temp_data += header_and_type
        temp_data += data_length.to_bytes(2, byteorder="big")
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.REGISTER_DATA,
            p1=P1.P1_REGISTER_DATA_INITIAL,
            p2=P2.P2_NONE,
            data=temp_data,
        ) as response:
            yield response

    @contextmanager
    def register_data_part_2(
        self,
        data: bytes,
    ) -> Generator[None, None, None]:
        data_chunks = split_message(data, MAX_APDU_LEN)
        last_chunk = data_chunks.pop()
        for chunk in data_chunks:
            temp_response = self.backend.exchange(
                cla=CLA,
                ins=InsType.REGISTER_DATA,
                p1=P1.P1_REGISTER_DATA_PAYLOAD,
                p2=P2.P2_NONE,
                data=chunk,
            )
            if temp_response.status != 0x9000:
                raise ExceptionRAPDU(temp_response.status)
        with self.backend.exchange_async(
            cla=CLA,
            ins=InsType.REGISTER_DATA,
            p1=P1.P1_REGISTER_DATA_PAYLOAD,
            p2=P2.P2_NONE,
            data=last_chunk,
        ) as response:
            yield response

    def get_async_response(self) -> Optional[RAPDU]:
        return self.backend.last_async_response
