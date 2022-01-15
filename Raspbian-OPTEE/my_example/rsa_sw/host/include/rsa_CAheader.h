#ifndef RSA_CA_HEADER_H
#define RSA_CA_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "tee_client_api.h"
#include <unistd.h>

//#define CRYPTO_VERIFY_UUID_ID {0x6718101e, 0x4746, 0x4b2b, \
//                                  { \
//                                      0xad, 0x32, 0xba, 0xc7, 0x51, 0xf7, 0x25, 0x1b \
//                                  } \
//                              }
#define CRYPTO_VERIFY_UUID_ID {0xebb6f4b5, 0x7e33, 0x4ad2, \
                                  { \
                                      0x98, 0x02, 0xe6, 0x4f, 0x2a, 0x7c, 0xc2, 0x0c \
                                  } \
                              }

/* Define the type of variable */
typedef unsigned char  UINT8;    /**< Typedef for 8bits unsigned integer  */
typedef unsigned short UINT16;   /**< Typedef for 16bits unsigned integer */
typedef unsigned int   UINT32;   /**< Typedef for 32bits unsigned integer */
typedef signed char    INT8;     /**< Typedef for 8bits signed integer    */
typedef signed short   INT16;    /**< Typedef for 16bits signed integer   */
typedef signed int     INT32;    /**< Typedef for 32bits signed integer   */
typedef char           CHAR;     /**< Typedef for char                    */

#define CMD_RSA_ENC_PKCS1_OPER            6U     /**< Command ID of using RSA algorithm to signa data */
#define CMD_RSA_DEC_PKCS1_OPER            7U
#define CMD_RSA_SIGN_PKCS1_OPER           8U
#define CMD_RSA_VERIFY_PKCS1_OPER         9U


#define   FAIL     -1            /* Return value when operation fail */
#define   OK        0            /* Return value when operation OK */


/* RSA key type(1024,2048) */
typedef enum
{
    EN_KEY_1024 = 1,
    EN_KEY_2048,
    EN_KEY_INVALID
}EN_RSA_KEY_TYPE;

typedef struct RsaCaPara_s
{
    CHAR* m_pInput;
    CHAR* m_pOutput;
    UINT32 m_InputLen;
    UINT32 m_OutputLen;
    UINT32 cmdId;
    EN_RSA_KEY_TYPE Rsa_Elect;
}RsaCaPara;

extern int g_CryptoVerifyCa_Rsa(RsaCaPara rsaPara);

#endif
