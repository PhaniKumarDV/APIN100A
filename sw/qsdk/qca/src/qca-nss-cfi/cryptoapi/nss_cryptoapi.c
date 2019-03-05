/* Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */

/**
 * nss_cryptoapi.c
 * 	Interface to communicate Native Linux crypto framework specific data
 * 	to Crypto core specific data
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/random.h>
#include <asm/scatterlist.h>
#include <linux/moduleparam.h>
#include <linux/spinlock.h>
#include <asm/cmpxchg.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/rtnetlink.h>
#include <linux/debugfs.h>

#include <crypto/ctr.h>
#include <crypto/des.h>
#include <crypto/aes.h>
#include <crypto/sha.h>
#include <crypto/hash.h>
#include <crypto/algapi.h>
#include <crypto/aead.h>
#include <crypto/authenc.h>
#include <crypto/scatterwalk.h>

#include <nss_api_if.h>
#include <nss_crypto_if.h>
#include <nss_cfi_if.h>
#include "nss_cryptoapi.h"

struct nss_cryptoapi gbl_ctx;

/*
 * crypto_alg structure initialization
 *	AEAD (Cipher and Authentication) and ABLK are supported by core crypto driver.
 */

static struct crypto_alg cryptoapi_algs[] = {
	{	/* sha1, aes */
		.cra_name       = "authenc(hmac(sha1),cbc(aes))",
		.cra_driver_name = "cryptoapi-aead-hmac-sha1-cbc-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = AES_BLOCK_SIZE,
				.maxauthsize    = SHA1_DIGEST_SIZE,
				.setkey = nss_cryptoapi_sha1_aes_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_sha1_aes_encrypt,
				.decrypt = nss_cryptoapi_sha1_aes_decrypt,
				.givencrypt = nss_cryptoapi_sha1_aes_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{	/* sha1, 3des */
		.cra_name       = "authenc(hmac(sha1),cbc(des3_ede))",
		.cra_driver_name = "cryptoapi-aead-hmac-sha1-cbc-3des",
		.cra_priority   = 300,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG,
		.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = DES3_EDE_BLOCK_SIZE,
				.maxauthsize    = SHA1_DIGEST_SIZE,
				.setkey = nss_cryptoapi_sha1_3des_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_sha1_3des_encrypt,
				.decrypt = nss_cryptoapi_sha1_3des_decrypt,
				.givencrypt = nss_cryptoapi_sha1_3des_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{	/* sha256, aes */
		.cra_name       = "authenc(hmac(sha256),cbc(aes))",
		.cra_driver_name = "cryptoapi-aead-hmac-sha256-cbc-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = AES_BLOCK_SIZE,
				.maxauthsize    = SHA256_DIGEST_SIZE,
				.setkey = nss_cryptoapi_sha256_aes_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_sha256_aes_encrypt,
				.decrypt = nss_cryptoapi_sha256_aes_decrypt,
				.givencrypt = nss_cryptoapi_sha256_aes_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{	/* sha256, 3des */
		.cra_name       = "authenc(hmac(sha256),cbc(des3_ede))",
		.cra_driver_name = "cryptoapi-aead-hmac-sha256-cbc-3des",
		.cra_priority   = 300,
		.cra_flags      = CRYPTO_ALG_TYPE_AEAD | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG,
		.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_aead_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_aead_init,
		.cra_exit       = nss_cryptoapi_aead_exit,
		.cra_u          = {
			.aead = {
				.ivsize         = DES3_EDE_BLOCK_SIZE,
				.maxauthsize    = SHA256_DIGEST_SIZE,
				.setkey = nss_cryptoapi_sha256_3des_setkey,
				.setauthsize = nss_cryptoapi_aead_setauthsize,
				.encrypt = nss_cryptoapi_sha256_3des_encrypt,
				.decrypt = nss_cryptoapi_sha256_3des_decrypt,
				.givencrypt = nss_cryptoapi_sha256_3des_geniv_encrypt,
				.geniv = "<built-in>",
			}
		}
	},
	{
		.cra_name       = "cbc(aes)",
		.cra_driver_name = "cryptoapi-ablkcipher-cbc-aes",
		.cra_priority   = 10000,
		.cra_flags      = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG | CRYPTO_ALG_NEED_FALLBACK,
		.cra_blocksize  = AES_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_ablkcipher_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_ablkcipher_init,
		.cra_exit       = nss_cryptoapi_ablkcipher_exit,
		.cra_u          = {
			.ablkcipher = {
				.ivsize         = AES_BLOCK_SIZE,
				.min_keysize    = AES_MIN_KEY_SIZE,
				.max_keysize    = AES_MAX_KEY_SIZE,
				.setkey         = nss_cryptoapi_aes_cbc_setkey,
				.encrypt        = nss_cryptoapi_aes_cbc_encrypt,
				.decrypt        = nss_cryptoapi_aes_cbc_decrypt,
			},
		},
	},
	{
		.cra_name       = "cbc(des3_ede)",
		.cra_driver_name = "cryptoapi-ablkcipher-cbc-3des",
		.cra_priority   = 300,
		.cra_flags      = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC | CRYPTO_ALG_NOSUPP_SG,
		.cra_blocksize  = DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize    = sizeof(struct nss_cryptoapi_ctx),
		.cra_alignmask  = 0,
		.cra_type       = &crypto_ablkcipher_type,
		.cra_module     = THIS_MODULE,
		.cra_init       = nss_cryptoapi_ablkcipher_init,
		.cra_exit       = nss_cryptoapi_ablkcipher_exit,
		.cra_u          = {
			.ablkcipher = {
				.ivsize         = DES3_EDE_BLOCK_SIZE,
				.min_keysize    = DES3_EDE_KEY_SIZE,
				.max_keysize    = DES3_EDE_KEY_SIZE,
				.setkey         = nss_cryptoapi_3des_cbc_setkey,
				.encrypt        = nss_cryptoapi_3des_cbc_encrypt,
				.decrypt        = nss_cryptoapi_3des_cbc_decrypt,
			},
		},
	},
};

/*
 * nss_cryptoapi_register()
 * 	register crypto core with the cryptoapi CFI
 */
static nss_crypto_user_ctx_t nss_cryptoapi_register(nss_crypto_handle_t crypto)
{
	int i;
	int rc = 0;
	struct nss_cryptoapi *sc = &gbl_ctx;

	nss_cfi_info("register nss_cryptoapi with core\n");

	sc->crypto = crypto;

	for (i = 0; i < ARRAY_SIZE(cryptoapi_algs); i++) {

		rc = crypto_register_alg(&cryptoapi_algs[i]);
		if (rc) {
			nss_cfi_err("Aead registeration failed, algo: %s\n", cryptoapi_algs[i].cra_name);
			continue;
		}
		nss_cfi_info("Aead registeration succeed, algo: %s\n", cryptoapi_algs[i].cra_name);
	}

	/*
	 * Initialize debugfs for cryptoapi.
	 */
	nss_cryptoapi_debugfs_init(sc);

	return sc;
}

/*
 * nss_cryptoapi_unregister()
 * 	Unregister crypto core with cryptoapi CFI layer
 */
static void nss_cryptoapi_unregister(nss_crypto_user_ctx_t cfi)
{
	struct nss_cryptoapi *sc = &gbl_ctx;
	int rc = 0;
	int i;

	nss_cfi_info("unregister nss_cryptoapi\n");

	for (i = 0; i < ARRAY_SIZE(cryptoapi_algs); i++) {

		rc = crypto_unregister_alg(&cryptoapi_algs[i]);
		if (rc) {
			nss_cfi_err("Aead unregister failed, algo: %s\n", cryptoapi_algs[i].cra_name);
			continue;
		}
		nss_cfi_info("Aead unregister succeed, algo: %s\n", cryptoapi_algs[i].cra_name);
	}

	/*
	 * cleanup cryptoapi debugfs.
	 */
	nss_cryptoapi_debugfs_exit(sc);
}

/*
 * nss_cryptoapi_init()
 * 	Initializing crypto core layer
 */
int nss_cryptoapi_init(void)
{
	struct nss_cryptoapi *sc = &gbl_ctx;

	sc->crypto = NULL;

	nss_crypto_register_user(nss_cryptoapi_register, nss_cryptoapi_unregister, "nss_cryptoapi");
	nss_cfi_info("initialize nss_cryptoapi\n");

	return 0;
}

/*
 * nss_cryptoapi_exit()
 * 	De-Initialize cryptoapi CFI layer
 */
void nss_cryptoapi_exit(void)
{
	struct nss_cryptoapi *sc = &gbl_ctx;

	if (sc->crypto) {
		nss_crypto_unregister_user(sc->crypto);
	}
	nss_cfi_info("exiting nss_cryptoapi\n");
}

MODULE_LICENSE("Dual BSD/GPL");

module_init(nss_cryptoapi_init);
module_exit(nss_cryptoapi_exit);
