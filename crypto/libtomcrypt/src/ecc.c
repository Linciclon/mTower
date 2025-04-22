// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2014-2019, Linaro Limited
 */

#include <config.h>
#include <crypto/crypto_impl.h>
#include <stdlib.h>
#include <string.h>
#include <tee_api_types.h>
#include <trace.h>
#include <utee_defines.h>

#include <crypto/acipher_helpers.h>

/*
 * For a given TEE @curve, return key size and LTC curve name. Also check that
 * @algo is compatible with this curve.
 * @curve: TEE_ECC_CURVE_NIST_P192, ...
 * @algo: TEE_ALG_ECDSA_SHA1, ...
 */
static TEE_Result ecc_get_curve_info(uint32_t curve, uint32_t algo,
				     size_t *key_size_bytes,
				     size_t *key_size_bits,
				     const char **curve_name)
{
	size_t size_bytes = 0;
	size_t size_bits = 0;
	const char *name = NULL;

	/*
	 * Excerpt of libtomcrypt documentation:
	 * ecc_make_key(... key_size ...): The keysize is the size of the
	 * modulus in bytes desired. Currently directly supported values
	 * are 12, 16, 20, 24, 28, 32, 48, and 65 bytes which correspond
	 * to key sizes of 112, 128, 160, 192, 224, 256, 384, and 521 bits
	 * respectively.
	 */

	switch (curve) {
	case TEE_ECC_CURVE_NIST_P192:
		size_bits = 192;
		size_bytes = 24;
		name = "NISTP192";
		if ((algo != 0) && (algo != TEE_ALG_ECDSA_SHA1) &&
		    (algo != TEE_ALG_ECDH_DERIVE_SHARED_SECRET))
			return TEE_ERROR_BAD_PARAMETERS;
		break;
	case TEE_ECC_CURVE_NIST_P224:
		size_bits = 224;
		size_bytes = 28;
		name = "NISTP224";
		if ((algo != 0) && (algo != TEE_ALG_ECDSA_SHA224) &&
		    (algo != TEE_ALG_ECDH_DERIVE_SHARED_SECRET))
			return TEE_ERROR_BAD_PARAMETERS;
		break;
	case TEE_ECC_CURVE_NIST_P256:
		size_bits = 256;
		size_bytes = 32;
		name = "NISTP256";
		if ((algo != 0) && (algo != TEE_ALG_ECDSA_SHA256) &&
		    (algo != TEE_ALG_ECDH_DERIVE_SHARED_SECRET))
			return TEE_ERROR_BAD_PARAMETERS;
		break;
	case TEE_ECC_CURVE_NIST_P384:
		size_bits = 384;
		size_bytes = 48;
		name = "NISTP384";
		if ((algo != 0) && (algo != TEE_ALG_ECDSA_SHA384) &&
		    (algo != TEE_ALG_ECDH_DERIVE_SHARED_SECRET))
			return TEE_ERROR_BAD_PARAMETERS;
		break;
	case TEE_ECC_CURVE_NIST_P521:
		size_bits = 521;
		size_bytes = 66;
		name = "NISTP521";
		if ((algo != 0) && (algo != TEE_ALG_ECDSA_SHA512) &&
		    (algo != TEE_ALG_ECDH_DERIVE_SHARED_SECRET))
			return TEE_ERROR_BAD_PARAMETERS;
		break;
	case TEE_ECC_CURVE_SM2:
		size_bits = 256;
		size_bytes = 32;
		name = "SM2";
		if ((algo != 0) && (algo != TEE_ALG_SM2_PKE) &&
		    (algo != TEE_ALG_SM2_DSA_SM3) &&
		    (algo != TEE_ALG_SM2_KEP))
			return TEE_ERROR_BAD_PARAMETERS;
		break;
	default:
		return TEE_ERROR_NOT_SUPPORTED;
	}

	if (key_size_bytes)
		*key_size_bytes = size_bytes;
	if (key_size_bits)
		*key_size_bits = size_bits;
	if (curve_name)
		*curve_name = name;
	return TEE_SUCCESS;
}

/* Note: this function clears the key before setting the curve */
static TEE_Result ecc_set_curve_from_name(ecc_key *ltc_key,
					  const char *curve_name)
{
	const ltc_ecc_curve *curve = NULL;
	int ltc_res = 0;

	ltc_res = ecc_find_curve(curve_name, &curve);
	if (ltc_res != CRYPT_OK)
		return TEE_ERROR_NOT_SUPPORTED;

	ltc_res = ecc_set_curve(curve, ltc_key);
	if (ltc_res != CRYPT_OK)
		return TEE_ERROR_GENERIC;

	return TEE_SUCCESS;
}

static TEE_Result _ltc_ecc_generate_keypair(struct ecc_keypair *key,
					    size_t key_size)
{
	TEE_Result res;
	ecc_key ltc_tmp_key;
	int ltc_res;
	size_t key_size_bytes = 0;
	size_t key_size_bits = 0;
	const char *name = NULL;

	res = ecc_get_curve_info(key->curve, 0, &key_size_bytes, &key_size_bits,
				 &name);
	if (res != TEE_SUCCESS)
		return res;

	if (key_size != key_size_bits)
		return TEE_ERROR_BAD_PARAMETERS;

	res = ecc_set_curve_from_name(&ltc_tmp_key, name);
	if (res)
		return res;

	/* Generate the ECC key */
	ltc_res = ecc_generate_key(NULL, find_prng("prng_crypto"),
				   &ltc_tmp_key);
	if (ltc_res != CRYPT_OK)
		return TEE_ERROR_BAD_PARAMETERS;

	/* check the size of the keys */
	if (((size_t)mp_count_bits(ltc_tmp_key.pubkey.x) > key_size_bits) ||
	    ((size_t)mp_count_bits(ltc_tmp_key.pubkey.y) > key_size_bits) ||
	    ((size_t)mp_count_bits(ltc_tmp_key.k) > key_size_bits)) {
		res = TEE_ERROR_BAD_PARAMETERS;
		goto exit;
	}

	/* check LTC is returning z==1 */
	if (mp_count_bits(ltc_tmp_key.pubkey.z) != 1) {
		res = TEE_ERROR_BAD_PARAMETERS;
		goto exit;
	}

	/* Copy the key */
	ltc_mp.copy(ltc_tmp_key.k, key->d);
	ltc_mp.copy(ltc_tmp_key.pubkey.x, key->x);
	ltc_mp.copy(ltc_tmp_key.pubkey.y, key->y);

	res = TEE_SUCCESS;

exit:
	ecc_free(&ltc_tmp_key);		/* Free the temporary key */
	return res;
}

/*
 * Given a keypair "key", populate the Libtomcrypt private key "ltc_key"
 * It also returns the key size, in bytes
 */
TEE_Result ecc_populate_ltc_private_key(ecc_key *ltc_key,
					struct ecc_keypair *key,
					uint32_t algo, size_t *key_size_bytes)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	const char *name = NULL;

	res = ecc_get_curve_info(key->curve, algo, key_size_bytes, NULL, &name);
	if (res)
		return res;

	memset(ltc_key, 0, sizeof(*ltc_key));

	res = ecc_set_curve_from_name(ltc_key, name);
	if (res)
		return res;

	ltc_key->type = PK_PRIVATE;
	mp_copy(key->d, ltc_key->k);
	mp_copy(key->x, ltc_key->pubkey.x);
	mp_copy(key->y, ltc_key->pubkey.y);
	mp_set_int(ltc_key->pubkey.z, 1);

	return TEE_SUCCESS;
}

static TEE_Result _ltc_ecc_sign(uint32_t algo, struct ecc_keypair *key,
				const uint8_t *msg, size_t msg_len,
				uint8_t *sig, size_t *sig_len)
{
	TEE_Result res = TEE_ERROR_GENERIC;
	int ltc_res = 0;
	size_t key_size_bytes = 0;
	ecc_key ltc_key = { };
	unsigned long ltc_sig_len = 0;

	if (algo == 0)
		return TEE_ERROR_BAD_PARAMETERS;

	res = ecc_populate_ltc_private_key(&ltc_key, key, algo,
					   &key_size_bytes);
	if (res != TEE_SUCCESS)
		return res;

	if (*sig_len < 2 * key_size_bytes) {
		*sig_len = 2 * key_size_bytes;
		res = TEE_ERROR_SHORT_BUFFER;
		goto out;
	}

	ltc_sig_len = *sig_len;
	ltc_res = ecc_sign_hash_rfc7518(msg, msg_len, sig, &ltc_sig_len,
				    NULL, find_prng("prng_crypto"), &ltc_key);
	if (ltc_res == CRYPT_OK) {
		res = TEE_SUCCESS;
	} else {
		res = TEE_ERROR_GENERIC;
	}
	*sig_len = ltc_sig_len;

out:
	ecc_free(&ltc_key);
	return res;
}

static const struct crypto_ecc_keypair_ops ecc_keypair_ops = {
	.generate = _ltc_ecc_generate_keypair,
	.sign = _ltc_ecc_sign,/*,
	//.shared_secret = _ltc_ecc_shared_secret,*/
};


TEE_Result crypto_asym_alloc_ecc_keypair(struct ecc_keypair *s,
					 uint32_t key_type,
					 size_t key_size_bits __unused)
{
	memset(s, 0, sizeof(*s));
	switch (key_type) {
	case TEE_TYPE_ECDSA_KEYPAIR:
	case TEE_TYPE_ECDH_KEYPAIR:
		s->ops = &ecc_keypair_ops;
		break;
	default:
		return TEE_ERROR_NOT_IMPLEMENTED;
	}

	if (!bn_alloc_max(&s->d))
		goto err;
	if (!bn_alloc_max(&s->x))
		goto err;
	if (!bn_alloc_max(&s->y))
		goto err;

	return TEE_SUCCESS;

err:
	s->ops = NULL;

	crypto_bignum_free(s->d);
	crypto_bignum_free(s->x);

	return TEE_ERROR_OUT_OF_MEMORY;
}

