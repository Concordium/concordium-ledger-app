from enum import IntEnum
from typing import Generator, List, Optional
from contextlib import contextmanager

from ragger.backend.interface import BackendInterface, RAPDU
from ragger.bip import pack_derivation_path


MAX_APDU_LEN: int = 255

CLA: int = 0xE0


class P1(IntEnum):
    # Parameter 1 for first APDU number.
    P1_START = 0x00
    # Parameter 1 for verify address new derivation path
    P1_VERIFY_ADDRESS_NEW_PATH = 0x01
    # Parameter 1 for maximum APDU number.
    P1_MAX = 0x03
    # Parameter 1 for screen confirmation for GET_PUBLIC_KEY.
    P1_CONFIRM = 0x01


class P2(IntEnum):
    # Parameter 2 for last APDU to receive.
    P2_LAST = 0x00
    # Parameter 2 for more APDU to receive.
    P2_MORE = 0x80
    # Parameter 2 for sign public key
    P2_SIGN_PUBLIC_KEY = 0x01


class InsType(IntEnum):
    VERIFY_ADDRESS = 0x00
    GET_PUBLIC_KEY = 0x01
    GET_VERSION = 0x03
    GET_APP_NAME = 0x21
    SIGN_SIMPLE_TRANSFER = 0x06


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


def split_message(message: bytes, max_size: int) -> List[bytes]:
    return [message[x : x + max_size] for x in range(0, len(message), max_size)]


class BoilerplateCommandSender:
    def __init__(self, backend: BackendInterface) -> None:
        self.backend = backend

    def get_app_and_version(self) -> RAPDU:
        return self.backend.exchange(
            cla=0xB0,  # specific CLA for BOLOS
            ins=0x01,  # specific INS for get_app_and_version
            p1=P1.P1_START,
            p2=P2.P2_LAST,
            data=b"",
        )

    def get_version(self) -> RAPDU:
        return self.backend.exchange(
            cla=CLA, ins=InsType.GET_VERSION, p1=P1.P1_START, p2=P2.P2_LAST, data=b""
        )

    def get_app_name(self) -> RAPDU:
        return self.backend.exchange(
            cla=CLA, ins=InsType.GET_APP_NAME, p1=P1.P1_START, p2=P2.P2_LAST, data=b""
        )

    def get_public_key(self, path: str, signPublicKey: bool = False) -> RAPDU:
        return self.backend.exchange(
            cla=CLA,
            ins=InsType.GET_PUBLIC_KEY,
            p1=P1.P1_START,
            p2=P2.P2_SIGN_PUBLIC_KEY if signPublicKey else P2.P2_LAST,
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
            p2=P2.P2_SIGN_PUBLIC_KEY if signPublicKey else P2.P2_LAST,
            data=pack_derivation_path(path),
        ) as response:
            yield response

    @contextmanager
    def verify_address(
        self, identity_index: int, credential_counter: int, idp_index: int = -1
    ) -> Generator[None, None, None]:
        data = b""
        p1 = P1.P1_START

        if idp_index != -1:
            data += idp_index.to_bytes(4, byteorder="big")
            p1 = P1.P1_VERIFY_ADDRESS_NEW_PATH

        data += identity_index.to_bytes(
            4, byteorder="big"
        ) + credential_counter.to_bytes(4, byteorder="big")
        with self.backend.exchange_async(
            cla=CLA, ins=InsType.VERIFY_ADDRESS, p1=p1, p2=P2.P2_LAST, data=data
        ) as response:
            yield response

    @contextmanager
    def sign_tx(
        self, path: str, tx_type_ins: InsType, transaction: bytes
    ) -> Generator[None, None, None]:

        self.backend.exchange(
            cla=CLA,
            ins=tx_type_ins,
            p1=P1.P1_START,
            p2=P2.P2_MORE,
            data=pack_derivation_path(path),
        )
        messages = split_message(transaction, MAX_APDU_LEN)
        idx: int = P1.P1_START + 1

        for msg in messages[:-1]:
            self.backend.exchange(
                cla=CLA, ins=tx_type_ins, p1=idx, p2=P2.P2_MORE, data=msg
            )
            idx += 1

        with self.backend.exchange_async(
            cla=CLA, ins=tx_type_ins, p1=idx, p2=P2.P2_LAST, data=messages[-1]
        ) as response:
            yield response

    def get_async_response(self) -> Optional[RAPDU]:
        return self.backend.last_async_response
