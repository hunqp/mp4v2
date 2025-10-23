/*
 * maz_com_asserts.h
 *
 *  Created on: Jun 14, 2020
 *      Author: wangbing
 */

#ifndef MAZ_COM_ASSERTS_H_
#define MAZ_COM_ASSERTS_H_

#include <maz_com_print.h>

#define MAZASSERT_RETVAL(condition, ret, fmt, msg...)                   \
    if (condition)                                                      \
    {                                                                   \
        dlog(DERR, fmt, ##msg);                                         \
        return ret;                                                     \
    }

#define MAZASSERT_RET(condition, fmt, msg...)                           \
    if (condition)                                                      \
    {                                                                   \
        dlog(DERR, fmt, ##msg);                                         \
        return;                                                         \
    }

#define MAZASSERT_CTN(condition, fmt, msg...)                           \
    if (condition)                                                      \
    {                                                                   \
        dlog(DERR, fmt, ##msg);                                         \
        continue;                                                       \
    }

#define MAZASSERT_BRK(condition, fmt, msg...)                           \
    if (condition)                                                      \
    {                                                                   \
        dlog(DERR, fmt, ##msg);                                         \
        break;                                                          \
    }

#define MAZASSERT_RETVAL_NOMSG(condition, ret)                          \
    if (condition)                                                      \
    {                                                                   \
        return ret;                                                     \
    }

#define MAZASSERT_RET_NOMSG(condition)                                  \
    if (condition)                                                      \
    {                                                                   \
        return;                                                         \
    }

#define MAZASSERT_CTN_NOMSG(condition)                                  \
    if (condition)                                                      \
    {                                                                   \
        continue;                                                       \
    }

#define MAZASSERT_BRK_NOMSG(condition)                                  \
    if (condition)                                                      \
    {                                                                   \
        break;                                                          \
    }

#endif /* MAZ_COM_ASSERTS_H_ */

