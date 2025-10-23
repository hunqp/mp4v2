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
        if (fmt != NULL)                                                \
        {                                                               \
            dmsg(DERR, fmt"\r\n", ##msg);                               \
        }                                                               \
        return ret;                                                     \
    }

#endif /* MAZ_COM_ASSERTS_H_ */

