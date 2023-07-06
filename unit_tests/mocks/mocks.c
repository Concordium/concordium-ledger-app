/** Mock unused functions to do nothing. They should be properly mocked, if a function using them is tested */

typedef int cx_err_t;
cx_err_t cx_hash_no_throw() {
    return 0;
}
void cx_sha256_init() {
}
void io_exchange() {
}
void ui_idle() {
}
cx_err_t os_derive_bip32_with_seed_no_throw() {
    return 0;
}
void cx_ecfp_init_private_key_no_throw() {
}
void cx_ecfp_generate_pair_no_throw() {
}
void cx_eddsa_sign_no_throw() {
}
void cx_hkdf_extract() {
}
void cx_hkdf_expand() {
}
void cx_math_modm_no_throw() {
}
void cx_math_is_zero() {
}
