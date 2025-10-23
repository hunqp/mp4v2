/*
 * maz_com_print.h
 *      Author: wangbing
 */

#ifndef INC_MAZ_COM_PRINT_H_
#define INC_MAZ_COM_PRINT_H_

#include "stdio.h"

#define DERR        0   /* error */
#define DWAN        1   /* warning */
#define DINF        2   /* information */
#define DBUG        3   /* debug */
#define DIGN        4   /* ignore */

/**
 * dmsg: debug message
 * dlog: debug log
 * cmsg: console message(with timestamp)
 * clog: console log(with timestamp)
 */
#define dmsg(lvl, fmt, arg...)  if(lvl <= dlvl) {                                                   \
                                    printf(fmt, ##arg);                                             \
                                }

#define dlog(lvl, fmt, arg...)  if(lvl <= dlvl) {                                                   \
                                    printf("[%04d][%s] "fmt"\r\n", __LINE__, __FUNCTION__, ##arg);  \
                                }

#endif /* INC_MAZ_COM_PRINT_H_ */
