/**
 * @file res.h
 * @author gabriel
 * @brief macros implementing the way to generate symbols related to the resources.
 * @version 0.1
 * @date 2022-10-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef __RES_H__
#define __RES_H__

/* code constants */
#define RES__PREFIX res__
#define RES__SUFFIX_STRT _start
#define RES__SUFFIX_END _end

/* ressources' linked name constants */
#define RES__B_PREFIX "_binary_"
#define RES__B_SUFFIX_STRT "_start"
#define RES__B_SUFFIX_END "_end"

#define DECLARE_RES_IMPL(_BASE_SYMBOL, str_identifier, PREFIX, SUFFIX_START, SUFFIX_END, str_binary_prefix) \
extern const unsigned char PREFIX ## _BASE_SYMBOL ## SUFFIX_START [] asm(str_binary_prefix str_identifier #SUFFIX_START); \
extern const unsigned char PREFIX ## _BASE_SYMBOL ## SUFFIX_END   [] asm(str_binary_prefix str_identifier #SUFFIX_END ); \

#define  DECLARE_RES_RESOLVE(_BASE_SYMBOL, str_identifier, PREFIX, SUFFIX_START, SUFFIX_END, str_binary_prefix) DECLARE_RES_IMPL(_BASE_SYMBOL, str_identifier, PREFIX, SUFFIX_START, SUFFIX_END, str_binary_prefix)

/**
 * @brief Declares an embedded resource as a set of two symbols : the start of a byte array and its end.
 *  declares two symbols : res__`_BASE_SYMBOL'_start and res__`_BASE_SYMBOL'_end.
 * 
 * @tparam _BASE_SYMBOL the base symbol used for the generation of the two other symbols.
 * @tparam str_identifier the base string litteral used to alias real name symbols in the executable. This is typically the relative path + name (from compilation active directory) with every punctuation character replaced by `_' : "res/image.png" becomes "res_image_png".
 */
#define DECLARE_RES(_BASE_SYMBOL, str_identifier) DECLARE_RES_RESOLVE(_BASE_SYMBOL, str_identifier, RES__PREFIX, RES__SUFFIX_STRT, RES__SUFFIX_END, RES__B_PREFIX)

#endif
