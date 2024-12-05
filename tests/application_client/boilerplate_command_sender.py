from enum import IntEnum
from typing import Generator, Optional
from contextlib import contextmanager

from ragger.backend.interface import BackendInterface, RAPDU
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
    # Basic P1 for all instructions
    P1_NONE = 0x00


class P2(IntEnum):
    # Parameter 2 for sign for GET_PUBLIC_KEY.
    P2_SIGN = 0x01
    P2_NO_SIGN = 0x00
    # Basic P2 for all instructions
    P2_NONE = 0x00
    # # Parameter 2 for last APDU to receive.
    # P2_LAST = 0x00
    # # Parameter 2 for more APDU to receive.
    # P2_MORE = 0x80


class InsType(IntEnum):
    VERIFY_ADDRESS = 0x00
    GET_PUBLIC_KEY = 0x01
    GET_APP_NAME = 0x21
    SIGN_TRANSFER = 0x02
    SIGN_TRANSFER_WITH_SCHEDULE = 0x03
    CREDENTIAL_DEPLOYMENT = 0x04
    EXPORT_PRIVATE_KEY = 0x05
    ENCRYPTED_AMOUNT_TRANSFER = 0x10
    TRANSFER_TO_ENCRYPTED = 0x11
    TRANSFER_TO_PUBLIC = 0x12
    CONFIGURE_DELEGATION = 0x17
    CONFIGURE_BAKER = 0x18
    PUBLIC_INFO_FOR_IP = 0x20
    SIGN_UPDATE_CREDENTIAL = 0x31
    SIGN_TRANSFER_WITH_MEMO = 0x32
    ENCRYPTED_AMOUNT_TRANSFER_WITH_MEMO = 0x33
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


class BoilerplateCommandSender:
    def __init__(self, backend: BackendInterface) -> None:
        self.backend = backend

    # def get_app_and_version(self) -> RAPDU:
    #     return self.backend.exchange(
    #         cla=0xB0,  # specific CLA for BOLOS
    #         ins=0x01,  # specific INS for get_app_and_version
    #         p1=P1.P1_START,
    #         p2=P2.P2_LAST,
    #         data=b"",
    #     )

    # def get_version(self) -> RAPDU:
    #     return self.backend.exchange(
    #         cla=CLA, ins=InsType.GET_VERSION, p1=P1.P1_NONE, p2=P2.P2_NONE, data=b""
    #     )

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
        memo_chunk = split_message(memo, MAX_APDU_LEN)
        for chunk in memo_chunk:
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

    # @contextmanager
    # def sign_tx(
    #     self, path: str, tx_type_ins: InsType, transaction: bytes
    # ) -> Generator[None, None, None]:

    #     self.backend.exchange(
    #         cla=CLA,
    #         ins=InsType.SIGN_TRANSFER,
    #         p1=P1.P1_START,
    #         p2=P2.P2_MORE,
    #         data=pack_derivation_path(path),
    #     )
    #     messages = split_message(transaction, MAX_APDU_LEN)
    #     idx: int = P1.P1_START + 1

    #     for msg in messages[:-1]:
    #         self.backend.exchange(
    #             cla=CLA, ins=tx_type_ins, p1=idx, p2=P2.P2_MORE, data=msg
    #         )
    #         idx += 1

    #     with self.backend.exchange_async(
    #         cla=CLA, ins=tx_type_ins, p1=idx, p2=P2.P2_LAST, data=messages[-1]
    #     ) as response:
    #         yield response

    def get_async_response(self) -> Optional[RAPDU]:
        return self.backend.last_async_response
