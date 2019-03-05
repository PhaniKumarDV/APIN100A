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

extern struct nss_cryptoapi gbl_ctx;

struct nss_cryptoapi_ablk_info {
	void *iv;
	struct nss_crypto_params *params;
	nss_crypto_comp_t cb_fn;
};

/*
 * nss_cryptoapi_ablkcipher_init()
 * 	Cryptoapi ablkcipher init function.
 */
int nss_cryptoapi_ablkcipher_init(struct crypto_tfm *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct crypto_ablkcipher *sw_tfm;

	nss_cfi_assert(ctx);

	ctx->sid = NSS_CRYPTO_MAX_IDXS;
	ctx->queued = 0;
	ctx->completed = 0;
	ctx->queue_failed = 0;
	ctx->fallback_req = 0;
	atomic_set(&ctx->refcnt, 0);

	nss_cryptoapi_set_magic(ctx);

	/* Alloc fallback transform for future use */
	sw_tfm = crypto_alloc_ablkcipher(crypto_tfm_alg_name(tfm), 0, CRYPTO_ALG_ASYNC | CRYPTO_ALG_NEED_FALLBACK);
	if (IS_ERR(sw_tfm)) {
		nss_cfi_err("unable to alloc software crypto for %s\n", crypto_tfm_alg_name(tfm));
		return -EINVAL;
	}

	/* set this tfm reqsize same to fallback tfm */
	crypto_ablkcipher_crt(__crypto_ablkcipher_cast(tfm))->reqsize = crypto_ablkcipher_reqsize(sw_tfm);
	ctx->sw_tfm = crypto_ablkcipher_tfm(sw_tfm);

	return 0;
}

/*
 * nss_cryptoapi_ablkcipher_exit()
 * 	Cryptoapi ablkcipher exit function.
 */
void nss_cryptoapi_ablkcipher_exit(struct crypto_tfm *tfm)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct nss_cryptoapi *sc = &gbl_ctx;
	nss_crypto_status_t status;

	nss_cfi_assert(ctx);

	if (!atomic_dec_and_test(&ctx->refcnt)) {
		nss_cfi_err("Process done is not completed, while exit is called\n");
		nss_cfi_assert(false);
	}

	nss_cfi_assert(ctx->sw_tfm);
	crypto_free_ablkcipher(__crypto_ablkcipher_cast(ctx->sw_tfm));
	ctx->sw_tfm = NULL;

	/*
	 * When NSS_CRYPTO_MAX_IDXS is set, it means that fallback tfm was used
	 * we didn't create any sessions
	 */
	if (ctx->sid == NSS_CRYPTO_MAX_IDXS)
		return;

	nss_cryptoapi_debugfs_del_session(ctx);

	status = nss_crypto_session_free(sc->crypto, ctx->sid);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_err("unable to free session: idx %d\n", ctx->sid);
	}

	nss_cryptoapi_clear_magic(ctx);
}

/*
 * nss_cryptoapi_ablkcipher_setkey()
 * 	Populate nss_crypto_key structures for cipher.
 */
static inline void nss_cryptoapi_ablkcipher_setkey(const u8 *key, unsigned int keylen, struct nss_crypto_key *cip)
{
	cip->key = (uint8_t *)key;
	cip->key_len = keylen;
}

/*
 * nss_cryptoapi_aes_cbc_setkey()
 * 	Cryptoapi setkey routine for aes.
 */
int nss_cryptoapi_aes_cbc_setkey(struct crypto_ablkcipher *cipher, const u8 *key, unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_crypto_key cip = { .algo = NSS_CRYPTO_CIPHER_AES };
	struct nss_crypto_key *cip_ptr = &cip;
	uint32_t flag = CRYPTO_TFM_RES_BAD_KEY_LEN;
	int ret;
	nss_crypto_status_t status;

	/*
	 * validate magic number - init should be called before setkey
	 */
	nss_cryptoapi_verify_magic(ctx);

	if (atomic_cmpxchg(&ctx->refcnt, 0, 1)) {
		nss_cfi_err("reusing context, setkey is already called\n");
		return -EINVAL;
	}

	/*
	 * set cipher key
	 */
	nss_cryptoapi_ablkcipher_setkey(key, keylen, &cip);

	/*
	 * Validate key length
	 */
	switch (cip.key_len) {
	case NSS_CRYPTOAPI_KEYLEN_AES128:
	case NSS_CRYPTOAPI_KEYLEN_AES256:
		/* success */
		ctx->fallback_req = false;
		break;
	case NSS_CRYPTOAPI_KEYLEN_AES192:
		/*
		 * AES192 is not supported by hardware, falling back to software
		 * crypto.
		 */
		nss_cfi_assert(ctx->sw_tfm);
		ctx->fallback_req = true;
		ctx->sid = NSS_CRYPTO_MAX_IDXS;

		/* set flag to fallback tfm */
		crypto_tfm_clear_flags(ctx->sw_tfm, CRYPTO_TFM_REQ_MASK);
		crypto_tfm_set_flags(ctx->sw_tfm, crypto_ablkcipher_get_flags(cipher) & CRYPTO_TFM_REQ_MASK);

		 /* Set key to the fallback tfm */
		ret = crypto_ablkcipher_setkey(__crypto_ablkcipher_cast(ctx->sw_tfm), key, keylen);
		if (ret) {
			nss_cfi_err("Failed to set key to the sw crypto");

			/*
			 * Set back the fallback tfm flag to the original flag one after
			 * doing setkey
			 */
			crypto_ablkcipher_set_flags(cipher, crypto_tfm_get_flags(ctx->sw_tfm));
		}
		return ret;
	default:
		nss_cfi_err("Bad Cipher key_len(%d)\n", cip.key_len);
		goto fail;
	}

	status = nss_crypto_session_alloc(sc->crypto, cip_ptr, NULL, &ctx->sid);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_err("nss_crypto_session_alloc failed - status: %d\n", status);
		ctx->sid = NSS_CRYPTO_MAX_IDXS;
		flag = CRYPTO_TFM_RES_BAD_FLAGS;
		goto fail;
	}

	nss_cryptoapi_debugfs_add_session(sc, ctx);

	nss_cfi_info("session id created: %d\n", ctx->sid);

	ctx->cip_alg = NSS_CRYPTO_CIPHER_AES;

	return 0;

fail:
	crypto_ablkcipher_set_flags(cipher, flag);
	return -EINVAL;
}

/*
 * nss_cryptoapi_ablkcipher_done()
 * 	Cipher operation completion callback function
 */
void nss_cryptoapi_ablkcipher_done(struct nss_crypto_buf *buf)
{
	struct nss_cryptoapi_ctx *ctx;
	struct ablkcipher_request *req;
	int err = 0;

	nss_cfi_assert(buf);

	req = (struct ablkcipher_request *)nss_crypto_get_cb_ctx(buf);

	/*
	 * check cryptoapi context magic number.
	 */
	ctx = crypto_tfm_ctx(req->base.tfm);
	nss_cryptoapi_verify_magic(ctx);

	/*
	 * Free Crypto buffer.
	 */
	nss_crypto_buf_free(gbl_ctx.crypto, buf);

	nss_cfi_dbg("after transformation\n");
	nss_cfi_dbg_data(sg_virt(req->dst), req->nbytes, ' ');

	/*
	 * Passing always pass in case of encrypt.
	 * Perhaps whenever core crypto invloke callback routine, it is always pass.
	 */
	req->base.complete(&req->base, err);

	nss_cfi_assert(atomic_read(&ctx->refcnt));
	atomic_dec(&ctx->refcnt);
	ctx->completed++;
}

/*
 * nss_cryptoapi_ablk_checkaddr()
 * 	Cryptoapi: obtain sg to virtual address mapping.
 * 	Check for multiple sg in src and dst
 */
int nss_cryptoapi_ablk_checkaddr(struct ablkcipher_request *req)
{
	/*
	 * Currently only single sg is supported
	 * 	return error, if caller send multiple sg for any of src and dst.
	 */
	if (nss_cryptoapi_sg_has_frags(req->src)) {
		nss_cfi_err("Only single sg supported: src invalid\n");
		return -EINVAL;
	}

	if (nss_cryptoapi_sg_has_frags(req->dst)) {
		nss_cfi_err("Only single sg supported: dst invalid\n");
		return -EINVAL;
	}

	/*
	 * If the size of data is more than 65K reject transformation
	 */
	if (req->nbytes > NSS_CRYPTOAPI_MAX_DATA_LEN) {
		nss_cfi_err("Buffer length exceeded limit\n");
		return -EINVAL;
	}

	return 0;
}

/*
 * nss_cryptoapi_ablk_transform()
 * 	Crytoapi common routine for encryption and decryption operations.
 */
struct nss_crypto_buf *nss_cryptoapi_ablk_transform(struct ablkcipher_request *req, struct nss_cryptoapi_ablk_info *info)
{
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nss_crypto_buf *buf;
	struct nss_cryptoapi *sc = &gbl_ctx;
	nss_crypto_status_t status;
	uint16_t ivsize;
	uint16_t cipher_len = 0, auth_len = 0;
	uint8_t *iv_addr;

	nss_cfi_assert(ctx);

	nss_cfi_dbg("src_vaddr: 0x%p, dst_vaddr: 0x%p, iv: 0x%p\n",
			sg_virt(req->src), sg_virt(req->dst), req->info);

	info->params->cipher_skip = 0;
	info->params->auth_skip = 0;

	if (nss_cryptoapi_ablk_checkaddr(req)) {
		nss_cfi_err("Invalid address!!\n");
		return NULL;
	}
	/*
	 * Update the crypto session data
	 */
	status = nss_crypto_session_update(sc->crypto, ctx->sid, info->params);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_err("Invalid crypto session parameters\n");
		return NULL;
	}

	/*
	 * Allocate crypto buf
	 */
	buf = nss_crypto_buf_alloc(sc->crypto);
	if (!buf) {
		nss_cfi_err("not able to allocate crypto buffer\n");
		return NULL;
	}

	/*
	 *  set crypto buffer callback
	 */
	nss_crypto_set_cb(buf, info->cb_fn, req);
	nss_crypto_set_session_idx(buf, ctx->sid);

	ivsize = crypto_ablkcipher_ivsize(crypto_ablkcipher_reqtfm(req));

	/*
	 * Get IV location and memcpy the IV
	 */
	iv_addr = nss_crypto_get_ivaddr(buf);
	memcpy(iv_addr, info->iv, ivsize);

	/*
	 * Fill Cipher and Auth len
	 */
	cipher_len = req->nbytes;
	auth_len = 0;

	nss_crypto_set_data(buf, sg_virt(req->src), sg_virt(req->dst), cipher_len);
	nss_crypto_set_transform_len(buf, cipher_len, auth_len);

	nss_cfi_dbg("cipher_len: %d, iv_len: %d, auth_len: %d"
			"cipher_skip: %d, auth_skip: %d\n",
			buf->cipher_len, ivsize, buf->auth_len,
			info->params->cipher_skip, info->params->auth_skip);
	nss_cfi_dbg("before transformation\n");
	nss_cfi_dbg_data(sg_virt(req->src), cipher_len, ' ');

	return buf;
}

/*
 * nss_cryptoapi_ablkcipher_fallback()
 *	Cryptoapi fallback for ablkcipher algorithm.
 */
int nss_cryptoapi_ablkcipher_fallback(struct nss_cryptoapi_ctx *ctx, struct ablkcipher_request *req, int type)
{
	struct crypto_ablkcipher *orig_tfm = crypto_ablkcipher_reqtfm(req);
	int err;

	nss_cfi_assert(ctx->sw_tfm);

	/* Set new fallback tfm to the request */
	ablkcipher_request_set_tfm(req, __crypto_ablkcipher_cast(ctx->sw_tfm));

	ctx->queued++;

	switch (type) {
	case NSS_CRYPTOAPI_ENCRYPT:
		err = crypto_ablkcipher_encrypt(req);
		break;
	case NSS_CRYPTOAPI_DECRYPT:
		err = crypto_ablkcipher_decrypt(req);
		break;
	default:
		err = -EINVAL;
	}

	if (!err)
		ctx->completed++;

	/* Set original tfm to the request */
	ablkcipher_request_set_tfm(req, orig_tfm);

	return err;
}

/*
 *
 * nss_cryptoapi_ablkcipher_init()
 * 	Cryptoapi ablkcipher init function.
 */
int nss_cryptoapi_aes_cbc_encrypt(struct ablkcipher_request *req)
{
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nss_crypto_params params = { .req_type = NSS_CRYPTO_REQ_TYPE_ENCRYPT };
	struct nss_crypto_buf *buf;
	struct nss_cryptoapi_ablk_info info;

	/*
	 * check cryptoapi context magic number.
	 */
	nss_cryptoapi_verify_magic(ctx);

	if (ctx->fallback_req)
		return nss_cryptoapi_ablkcipher_fallback(ctx, req, NSS_CRYPTOAPI_ENCRYPT);

	/*
	 * Check if previous call to setkey couldn't allocate session with core crypto.
	 */
	if (ctx->sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("Invalid session\n");
		return -EINVAL;
	}

	if (nss_crypto_get_cipher(ctx->sid) != NSS_CRYPTO_CIPHER_AES) {
		nss_cfi_err("Invalid Algo for session id: %d\n", ctx->sid);
		return -EINVAL;
	}

	if (nss_cryptoapi_check_unalign(req->nbytes, AES_BLOCK_SIZE)) {
		nss_cfi_err("Invalid cipher len - Not aligned to algo blocksize\n");
		crypto_ablkcipher_set_flags(crypto_ablkcipher_reqtfm(req), CRYPTO_TFM_RES_BAD_BLOCK_LEN);
		return -EINVAL;
	}

	info.iv = req->info;
	info.params = &params;
	info.cb_fn = nss_cryptoapi_ablkcipher_done;

	buf = nss_cryptoapi_ablk_transform(req, &info);
	if (!buf) {
		nss_cfi_err("Invalid parameters\n");
		return -EINVAL;
	}

	/*
	 *  Send the buffer to CORE layer for processing
	 */
	if (nss_crypto_transform_payload(sc->crypto, buf) !=  NSS_CRYPTO_STATUS_OK) {
		nss_cfi_info("Not enough resources with driver\n");
		nss_crypto_buf_free(sc->crypto, buf);
		ctx->queue_failed++;
		return -EINVAL;
	}

	ctx->queued++;
	atomic_inc(&ctx->refcnt);

	return -EINPROGRESS;
}

/*
 *
 * nss_cryptoapi_ablkcipher_init()
 * 	Cryptoapi ablkcipher init function.
 */
int nss_cryptoapi_aes_cbc_decrypt(struct ablkcipher_request *req)
{
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nss_crypto_params params = { .req_type = NSS_CRYPTO_REQ_TYPE_DECRYPT };
	struct nss_crypto_buf *buf;
	struct nss_cryptoapi_ablk_info info;

	/*
	 * check cryptoapi context magic number.
	 */
	nss_cryptoapi_verify_magic(ctx);

	if (ctx->fallback_req)
		return nss_cryptoapi_ablkcipher_fallback(ctx, req, NSS_CRYPTOAPI_DECRYPT);

	/*
	 * Check if previous call to setkey couldn't allocate session with core crypto.
	 */
	if (ctx->sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("Invalid session\n");
		return -EINVAL;
	}

	if (nss_crypto_get_cipher(ctx->sid) != NSS_CRYPTO_CIPHER_AES) {
		nss_cfi_err("Invalid Algo for session id: %d\n", ctx->sid);
		return -EINVAL;
	}

	if (nss_cryptoapi_check_unalign(req->nbytes, AES_BLOCK_SIZE)) {
		nss_cfi_err("Invalid cipher len - Not aligned to algo blocksize\n");
		crypto_ablkcipher_set_flags(crypto_ablkcipher_reqtfm(req), CRYPTO_TFM_RES_BAD_BLOCK_LEN);
		return -EINVAL;
	}

	info.iv = req->info;
	info.params = &params;
	info.cb_fn = nss_cryptoapi_ablkcipher_done;

	buf = nss_cryptoapi_ablk_transform(req, &info);
	if (!buf) {
		nss_cfi_err("Invalid parameters\n");
		return -EINVAL;
	}

	/*
	 *  Send the buffer to CORE layer for processing
	 */
	if (nss_crypto_transform_payload(sc->crypto, buf) !=  NSS_CRYPTO_STATUS_OK) {
		nss_cfi_info("Not enough resources with driver\n");
		nss_crypto_buf_free(sc->crypto, buf);
		ctx->queue_failed++;
		return -EINVAL;
	}

	ctx->queued++;
	atomic_inc(&ctx->refcnt);

	return -EINPROGRESS;
}

/*
 *
 * nss_cryptoapi_ablkcipher_init()
 * 	Cryptoapi ablkcipher init function.
 */
int nss_cryptoapi_3des_cbc_setkey(struct crypto_ablkcipher *cipher, const u8 *key, unsigned int keylen)
{
	struct crypto_tfm *tfm = crypto_ablkcipher_tfm(cipher);
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(tfm);
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_crypto_key cip = { .algo = NSS_CRYPTO_CIPHER_DES };
	struct nss_crypto_key *cip_ptr = &cip;
	uint32_t flag = CRYPTO_TFM_RES_BAD_KEY_LEN;
	nss_crypto_status_t status;

	/*
	 * validate magic number - init should be called before setkey
	 */
	nss_cryptoapi_verify_magic(ctx);

	if (atomic_cmpxchg(&ctx->refcnt, 0, 1)) {
		nss_cfi_err("reusing context, setkey is already called\n");
		return -EINVAL;
	}

	/*
	 * set cipher key
	 */
	nss_cryptoapi_ablkcipher_setkey(key, keylen, &cip);

	/*
	 * Validate key length
	 */
	switch (cip.key_len) {
	case NSS_CRYPTOAPI_KEYLEN_3DES:
		/* success */
		break;
	default:
		nss_cfi_err("Bad Cipher key_len(%d)\n", cip.key_len);
		goto fail;
	}

	status = nss_crypto_session_alloc(sc->crypto, cip_ptr, NULL, &ctx->sid);
	if (status != NSS_CRYPTO_STATUS_OK) {
		nss_cfi_err("nss_crypto_session_alloc failed - status: %d\n", status);
		ctx->sid = NSS_CRYPTO_MAX_IDXS;
		flag = CRYPTO_TFM_RES_BAD_FLAGS;
		goto fail;
	}

	nss_cryptoapi_debugfs_add_session(sc, ctx);

	nss_cfi_info("session id created: %d\n", ctx->sid);

	ctx->cip_alg = NSS_CRYPTO_CIPHER_DES;

	return 0;

fail:
	crypto_ablkcipher_set_flags(cipher, flag);
	return -EINVAL;
}

/*
 *
 * nss_cryptoapi_ablkcipher_init()
 * 	Cryptoapi ablkcipher init function.
 */
int nss_cryptoapi_3des_cbc_encrypt(struct ablkcipher_request *req)
{
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nss_crypto_params params = { .req_type = NSS_CRYPTO_REQ_TYPE_ENCRYPT };
	struct nss_crypto_buf *buf;
	struct nss_cryptoapi_ablk_info info;

	/*
	 * check cryptoapi context magic number.
	 */
	nss_cryptoapi_verify_magic(ctx);

	/*
	 * Check if previous call to setkey couldn't allocate session with core crypto.
	 */
	if (ctx->sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("Invalid session\n");
		return -EINVAL;
	}

	if (nss_crypto_get_cipher(ctx->sid) != NSS_CRYPTO_CIPHER_DES) {
		nss_cfi_err("Invalid Algo for session id: %d\n", ctx->sid);
		return -EINVAL;
	}

	if (nss_cryptoapi_check_unalign(req->nbytes, DES3_EDE_BLOCK_SIZE)) {
		nss_cfi_err("Invalid cipher len - Not aligned to algo blocksize\n");
		crypto_ablkcipher_set_flags(crypto_ablkcipher_reqtfm(req), CRYPTO_TFM_RES_BAD_BLOCK_LEN);
		return -EINVAL;
	}

	info.iv = req->info;
	info.params = &params;
	info.cb_fn = nss_cryptoapi_ablkcipher_done;

	buf = nss_cryptoapi_ablk_transform(req, &info);
	if (!buf) {
		nss_cfi_err("Invalid parameters\n");
		return -EINVAL;
	}

	/*
	 *  Send the buffer to CORE layer for processing
	 */
	if (nss_crypto_transform_payload(sc->crypto, buf) !=  NSS_CRYPTO_STATUS_OK) {
		nss_cfi_info("Not enough resources with driver\n");
		nss_crypto_buf_free(sc->crypto, buf);
		ctx->queue_failed++;
		return -EINVAL;
	}

	ctx->queued++;
	atomic_inc(&ctx->refcnt);

	return -EINPROGRESS;
}

/*
 *
 * nss_cryptoapi_ablkcipher_init()
 * 	Cryptoapi ablkcipher init function.
 */
int nss_cryptoapi_3des_cbc_decrypt(struct ablkcipher_request *req)
{
	struct nss_cryptoapi *sc = &gbl_ctx;
	struct nss_cryptoapi_ctx *ctx = crypto_tfm_ctx(req->base.tfm);
	struct nss_crypto_params params = { .req_type = NSS_CRYPTO_REQ_TYPE_DECRYPT };
	struct nss_crypto_buf *buf;
	struct nss_cryptoapi_ablk_info info;

	/*
	 * check cryptoapi context magic number.
	 */
	nss_cryptoapi_verify_magic(ctx);

	/*
	 * Check if previous call to setkey couldn't allocate session with core crypto.
	 */
	if (ctx->sid >= NSS_CRYPTO_MAX_IDXS) {
		nss_cfi_err("Invalid session\n");
		return -EINVAL;
	}

	if (nss_crypto_get_cipher(ctx->sid) != NSS_CRYPTO_CIPHER_DES) {
		nss_cfi_err("Invalid Algo for session id: %d\n", ctx->sid);
		return -EINVAL;
	}

	if (nss_cryptoapi_check_unalign(req->nbytes, DES3_EDE_BLOCK_SIZE)) {
		nss_cfi_err("Invalid cipher len - Not aligned to algo blocksize\n");
		crypto_ablkcipher_set_flags(crypto_ablkcipher_reqtfm(req), CRYPTO_TFM_RES_BAD_BLOCK_LEN);
		return -EINVAL;
	}

	info.iv = req->info;
	info.params = &params;
	info.cb_fn = nss_cryptoapi_ablkcipher_done;

	buf = nss_cryptoapi_ablk_transform(req, &info);
	if (!buf) {
		nss_cfi_err("Invalid parameters\n");
		return -EINVAL;
	}

	/*
	 *  Send the buffer to CORE layer for processing
	 */
	if (nss_crypto_transform_payload(sc->crypto, buf) !=  NSS_CRYPTO_STATUS_OK) {
		nss_cfi_info("Not enough resources with driver\n");
		nss_crypto_buf_free(sc->crypto, buf);
		ctx->queue_failed++;
		return -EINVAL;
	}

	ctx->queued++;
	atomic_inc(&ctx->refcnt);

	return -EINPROGRESS;
}
