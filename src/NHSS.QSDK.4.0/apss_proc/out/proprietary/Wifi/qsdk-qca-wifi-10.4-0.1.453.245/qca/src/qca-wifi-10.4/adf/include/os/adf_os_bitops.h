/*
 * Copyright (c) 2010, Atheros Communications Inc. 
 * All Rights Reserved.
 * 
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 * 
 */

/** 
 * @ingroup adf_os_public
 * @file adf_os_bitops.h
 * This file abstracts bit-level operations on a stream of bytes.
 */

#ifndef _ADF_OS_BITOPS_H
#define _ADF_OS_BITOPS_H

#include <adf_os_types.h>

/**
 * @brief Set a bit atomically
 * @param[in] nr    Bit to change
 * @param[in] addr  Address to start counting from
 *
 * @note its atomic and cannot be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_set_bit_a(a_uint32_t nr, volatile a_uint32_t *addr)
{
    __adf_os_set_bit_a(nr, addr);
}

/**
 * @brief Set a bit
 * @param[in] nr    Bit to change
 * @param[in] addr  Address to start counting from
 *
 * @note its not atomic and can be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_set_bit(a_uint32_t nr, volatile a_uint32_t *addr)
{
    __adf_os_set_bit(nr, addr);
}

/**
 * @brief Clear a bit atomically
 * @param[in] nr    Bit to change
 * @param[in] addr  Address to start counting from
 *
 * @note its atomic and cannot be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_clear_bit_a(a_uint32_t nr, volatile a_uint32_t *addr)
{
    __adf_os_clear_bit_a(nr, addr);
}

/**
 * @brief Clear a bit
 * @param[in] nr    Bit to change
 * @param[in] addr  Address to start counting from
 *
 * @note its not atomic and can be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_clear_bit(a_uint32_t nr, volatile a_uint32_t *addr)
{
    __adf_os_clear_bit(nr, addr);
}

/**
 * @brief Toggle a bit atomically
 * @param[in] nr    Bit to change
 * @param[in] addr  Address to start counting from
 *
 * @note its atomic and cannot be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_change_bit_a(a_uint32_t nr, volatile a_uint32_t *addr)
{
    __adf_os_change_bit_a(nr, addr);
}

/**
 * @brief Toggle a bit
 * @param[in] nr    Bit to change
 * @param[in] addr  Address to start counting from
 *
 * @note its not atomic and can be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_change_bit(a_uint32_t nr, volatile a_uint32_t *addr)
{
    __adf_os_change_bit(nr, addr);
}

/**
 * @brief Test and Set a bit atomically
 * @param[in] nr    Bit to set
 * @param[in] addr  Address to start counting from
 *
 * @note its atomic and cannot be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_test_and_set_bit_a(a_uint32_t nr, 
                                          volatile a_uint32_t *addr)
{
    __adf_os_test_and_set_bit_a(nr, addr);
}

/**
 * @brief Test and Set a bit
 * @param[in] nr    Bit to set
 * @param[in] addr  Address to start counting from
 *
 * @note its not atomic and can be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_test_and_set_bit(a_uint32_t nr, 
                                          volatile a_uint32_t *addr)
{
    __adf_os_test_and_set_bit(nr, addr);
}

/**
 * @brief Test and clear a bit atomically
 * @param[in] nr    Bit to set
 * @param[in] addr  Address to start counting from
 *
 * @note its atomic and cannot be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_test_and_clear_bit_a(a_uint32_t nr, 
                                          volatile a_uint32_t *addr)
{
    __adf_os_test_and_clear_bit_a(nr, addr);
}

/**
 * @brief Test and clear a bit
 * @param[in] nr    Bit to set
 * @param[in] addr  Address to start counting from
 *
 * @note its not atomic and can be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_test_and_clear_bit(a_uint32_t nr, 
                                          volatile a_uint32_t *addr)
{
    __adf_os_test_and_clear_bit(nr, addr);
}

/**
 * @brief Test and change a bit atomically
 * @param[in] nr    Bit to set
 * @param[in] addr  Address to start counting from
 *
 * @note its atomic and cannot be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_test_and_change_bit_a(a_uint32_t nr, 
                                          volatile a_uint32_t *addr)
{
    __adf_os_test_and_change_bit_a(nr, addr);
}

/**
 * @brief Test and clear a bit
 * @param[in] nr    Bit to set
 * @param[in] addr  Address to start counting from
 *
 * @note its not atomic and can be re-ordered.
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 */
static inline void adf_os_test_and_change_bit(a_uint32_t nr, 
                                          volatile a_uint32_t *addr)
{
    __adf_os_test_and_change_bit(nr, addr);
}

/**
 * @brief test_bit - Determine whether a bit is set
 * @param[in] nr    bit number to test
 * @param[in] addr  Address to start counting from
 *
 * @return 1 if set, 0 if not
 */
static inline int adf_os_test_bit(a_uint32_t nr, volatile a_uint32_t *addr)
{
    __adf_os_test_bit(nr, addr);
}


#endif /**_AOD_BITOPS_H*/
