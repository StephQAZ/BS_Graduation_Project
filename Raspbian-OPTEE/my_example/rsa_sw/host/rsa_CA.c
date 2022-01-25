#include <rsa_CAheader.h>

TEEC_UUID uuid = CRYPTO_VERIFY_UUID_ID;
TEEC_Context ctx;
static int g_TaskInitFlag = -1;    /* Flag if the task done initialize operation */

// 1) Initialize Context
static int l_CryptoVerifyCa_TaskInit(void)
{
    TEEC_Result result;
    int l_RetVal = OK;

    /**1) Check if need to do task initialization operation */
    if(-1 == g_TaskInitFlag)
    {
        result = TEEC_InitializeContext(NULL, &ctx);
        if(result != TEEC_SUCCESS)
        {
            printf("InitializeContext failed, ReturnCode=0x%x\n", result);
            l_RetVal= FAIL;
        }
        else
        {
            g_TaskInitFlag = 1;
            printf("InitializeContext success\n");
            l_RetVal = OK;
        }
    }

    return l_RetVal;
}

// 2) Open Session between TA and CA
static int l_CryptoVerifyCa_OpenSession(TEEC_Session* session)
{
    TEEC_Result result;
    int l_RetVal = FAIL;
    uint32_t origin;

    result = TEEC_OpenSession(&ctx, session, &uuid,
                              TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
    if(result != TEEC_SUCCESS)
    {
        printf("OpenSession failed, ReturnCode=0x%x, ReturnOrigin=0x%x\n", result, origin);
        g_TaskInitFlag = -1;
        l_RetVal = FAIL;
    }
    else
    {
        printf("OpenSession success\n");
        l_RetVal = OK;
    }

    return l_RetVal;
}

// 4) Invoke TA with command ID
static int l_CryptoVerifyCa_SendCommand(TEEC_Operation* operation, TEEC_Session* session, uint32_t commandID)
{
    TEEC_Result result;
    int l_RetVal = FAIL;
    uint32_t origin;

    result = TEEC_InvokeCommand(session, commandID, operation, &origin);
    if (result != TEEC_SUCCESS)
    {
        printf("InvokeCommand failed, ReturnCode=0x%x, ReturnOrigin=0x%x\n", result, origin);
        l_RetVal = FAIL;
    }
    else
    {
        printf("InvokeCommand success\n");
        l_RetVal = OK;
    }


    return l_RetVal;
}

// 3) Prepare param
int g_CryptoVerifyCa_Rsa(RsaCaPara rsaPara)
{
    TEEC_Session   l_session;    /* Define the session of TA&CA */
    TEEC_Operation l_operation;  /* Define the operation for communicating between TA&CA */
    int l_RetVal = FAIL;       /* Define the return value of function */
    printf("The send command id is: %d\n", rsaPara.cmdId);
    /**1) Initialize this task */
    l_RetVal = l_CryptoVerifyCa_TaskInit();
    if(FAIL == l_RetVal) {
        goto cleanup_1;
    }

    /**2) Open session */
    l_RetVal = l_CryptoVerifyCa_OpenSession(&l_session);
    if (FAIL == l_RetVal) {
        goto cleanup_2;
    }

    /**3) Set the communication context between CA&TA */
    memset(&l_operation, 0x0, sizeof(TEEC_Operation));
    l_operation.started = 1;
    l_operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT,
                                              TEEC_VALUE_INPUT, TEEC_NONE);
    l_operation.params[0].tmpref.size = rsaPara.m_InputLen;
    l_operation.params[0].tmpref.buffer = rsaPara.m_pInput;
    l_operation.params[1].tmpref.size = rsaPara.m_OutputLen;
    l_operation.params[1].tmpref.buffer = rsaPara.m_pOutput;
    l_operation.params[2].value.a = rsaPara.Rsa_Elect;


    /**4) Send command to TA */
    l_RetVal = l_CryptoVerifyCa_SendCommand(&l_operation, &l_session, rsaPara.cmdId);
    if (FAIL == l_RetVal) {
        goto cleanup_3;
    }
    /****** enc command *******/
    if (rsaPara.cmdId == CMD_RSA_ENC_PKCS1_OPER) {
    
    	/******** print the encrypt data   ********/
    	
    	printf("The encrypt data you send is :\n");
    	for (int i = 0; i < rsaPara.m_OutputLen; i++) {
	    	if (0U == i%16U)
		{
		    printf("\n");
		    
		}
	    	printf("0x%02x, ", (rsaPara.m_pOutput[i] & 0x000000FFU));
	}
	printf("\n");
	
	/******** mosquitto system call  ********/
	
	char systemCall[1660] = "mosquitto_pub -p 8883 -h 192.168.31.125 --cafile ca.crt -t t1 --debug -m  \""; 
	char tempStr[1] = "\"";
	char tempbuf[6];
	for (int i = 0; i < rsaPara.m_OutputLen; i++) {
		sprintf(tempbuf, "0x%02x, ", rsaPara.m_pOutput[i]);
		strncat(systemCall, tempbuf, 6);
	}
	strncat(systemCall, tempStr, 1);
	// invoke bash call
	printf("\n");
	printf("%s\n", systemCall);
	printf("\n");
	system(systemCall);
    }
    /****** dec command *******/
    if (rsaPara.cmdId == CMD_RSA_DEC_PKCS1_OPER) {
    	/******** print the encrypt data   ********/
    	
    	printf("The decrypt data is :\n");
    	printf("%s\n", rsaPara.m_pOutput);
    }
    /**5) The clean up operation */
    cleanup_3:
    TEEC_CloseSession(&l_session);

    cleanup_2:
    TEEC_FinalizeContext(&ctx);
    cleanup_1:
    return l_RetVal;
}

CHAR rsa_raw_Data[128] = "";
/*CHAR rsa_encrypt_Data[] =
{
	0x98, 0x52, 0x67, 0x1e, 0x72, 0xc1, 0x3f, 0xb3, 0x25, 0x13, 0xb2, 0xa2, 0xc4, 0xc5, 0x1c, 0x84, 
	0xe2, 0x1e, 0xe3, 0x9a, 0x82, 0x5c, 0x04, 0xa0, 0xc7, 0xfe, 0x69, 0xf6, 0xb4, 0xf7, 0xf0, 0xbf, 
	0x33, 0xd9, 0xd6, 0x17, 0x54, 0x29, 0xba, 0x6c, 0x85, 0xd2, 0xc3, 0xbc, 0x29, 0x30, 0xf8, 0x7a, 
	0x6f, 0xeb, 0xa2, 0x06, 0x7b, 0x15, 0x22, 0x7e, 0xd2, 0x00, 0x92, 0x7e, 0x7d, 0xe5, 0x9d, 0xeb, 
	0xcd, 0x82, 0xa6, 0x72, 0x89, 0xe0, 0x63, 0x6c, 0xef, 0x3e, 0xc2, 0x0d, 0x77, 0xe8, 0x29, 0x43, 
	0x04, 0x3f, 0xba, 0xf2, 0xd4, 0x92, 0x96, 0x2f, 0xe1, 0xf3, 0x92, 0x39, 0x13, 0x4d, 0x0a, 0xec, 
	0x6c, 0xb3, 0xbc, 0x0a, 0x77, 0xd3, 0xdb, 0x69, 0x20, 0x0c, 0x41, 0xa6, 0x06, 0x6e, 0xf0, 0xd9, 
	0xfb, 0xf7, 0x66, 0xd9, 0x49, 0xf6, 0x5c, 0xe3, 0xdc, 0xae, 0x96, 0xba, 0x7f, 0x63, 0xb8, 0x4a, 
	0x1c, 0x77, 0xea, 0x8d, 0x09, 0xf8, 0xc7, 0xf2, 0x1e, 0x06, 0xf2, 0x81, 0xbb, 0xb6, 0x84, 0x4a, 
	0x35, 0x6d, 0x6f, 0xcc, 0xd6, 0x09, 0x86, 0xca, 0xba, 0x57, 0xc1, 0xa7, 0x89, 0xce, 0x9b, 0xc4, 
	0x40, 0x2f, 0x44, 0xdf, 0x97, 0xc5, 0xc2, 0x0a, 0x08, 0x89, 0x44, 0x2d, 0x2c, 0x25, 0x53, 0xd8, 
	0x5b, 0x1b, 0x5b, 0x64, 0xf7, 0x6f, 0x4a, 0x7a, 0x94, 0x1f, 0x30, 0xa4, 0xa9, 0xd4, 0x53, 0x43, 
	0xb5, 0x72, 0xa3, 0x6d, 0x01, 0x1e, 0x8f, 0x01, 0x4b, 0xf2, 0x15, 0x91, 0xc7, 0xcd, 0x6b, 0x18, 
	0xc5, 0xe1, 0xaf, 0xf7, 0xba, 0x12, 0x0c, 0x80, 0xbf, 0xbd, 0x23, 0x58, 0x89, 0x67, 0xde, 0x88, 
	0xe5, 0x89, 0x12, 0xb8, 0xe7, 0x8f, 0x09, 0xd4, 0x43, 0x60, 0x19, 0xe1, 0xce, 0xb9, 0x6f, 0x5a, 
	0xb2, 0x1b, 0x19, 0xfe, 0x79, 0xb9, 0x5c, 0x91, 0x3e, 0xfc, 0xc5, 0x7a, 0xc6, 0x56, 0xe2, 0x59
	
};*/
char rsa_encrypt_temp_Data[1600] = "";
char rsa_encrypt_Data[256] = "";

CHAR rsa_Output1024[128] = {0};
CHAR rsa_Output2048[256] = {0};

// RSA1024:   populate rsa.param 
/*void Rsa1024_Test(CHAR* operFlag, EN_RSA_KEY_TYPE keyType, CHAR* output, UINT32 outputLen)
{
    RsaCaPara l_rsaPara;
    l_rsaPara.m_pOutput = output;
    l_rsaPara.m_OutputLen = outputLen;
    l_rsaPara.Rsa_Elect = keyType;

//    if(0 == memcmp(operFlag, "sign", 4))
//    {
//        l_rsaPara.m_pInput = g_RsaPaddingRaw1;
//        l_rsaPara.m_InputLen = sizeof(g_RsaPaddingRaw1);
//        l_rsaPara.cmdId = CMD_RSA_SIGN_PKCS1_OPER;
//    }
//    else if(0 == memcmp(operFlag, "verify", 6))
//    {
//        l_rsaPara.m_pInput = g_RsaPcks1Verify1;
//        l_rsaPara.m_InputLen = 128U;
//        l_rsaPara.cmdId = CMD_RSA_VERIFY_PKCS1_OPER;
//    }
    if(0 == memcmp(operFlag, "enc", 3))
    {
        l_rsaPara.m_pInput = rsa_raw_Data;
        l_rsaPara.m_InputLen = sizeof(rsa_raw_Data);
        l_rsaPara.cmdId = CMD_RSA_ENC_PKCS1_OPER;
    }
    else if(0 == memcmp(operFlag, "dec", 3))
    {
        l_rsaPara.m_pInput = rsa_encrypt_Data;
        l_rsaPara.m_InputLen = 128U;
        l_rsaPara.cmdId = CMD_RSA_DEC_PKCS1_OPER;
    }
    else
    {
    }
    g_CryptoVerifyCa_Rsa(l_rsaPara);
}*/

void ByteOrNum(char c, int *temp) {
	if (c >= 'a' && c <= 'f') {
		*temp += 10 + c - 'a';
	} else if (c >= '0' && c <= '9') {
		*temp += c - '0';
	}
}

void hex2str(char* str) {
	int len = strlen(str);
	int j = 0;
	char tempChar[1];
	for (int i = 0; i < len; i+=6) {
		int temp =  0;
		ByteOrNum(rsa_encrypt_temp_Data[i + 2], &temp);
		temp *= 16;
		ByteOrNum(rsa_encrypt_temp_Data[i + 3], &temp);
		sprintf(tempChar, "%c", temp);
		rsa_encrypt_Data[j++] = tempChar[0];
	}
}
// RSA2048:   populate rsa.param
void Rsa2048_Test(CHAR* operFlag, EN_RSA_KEY_TYPE keyType, CHAR* output, UINT32 outputLen)
{
    RsaCaPara l_rsaPara;

    l_rsaPara.m_pOutput = output;
    l_rsaPara.m_OutputLen = outputLen;
    l_rsaPara.Rsa_Elect = keyType;

//    if(0 == memcmp(operFlag, "sign", 4))
//    {
//        l_rsaPara.m_pInput = g_RsaPaddingRaw2;
//        l_rsaPara.m_InputLen = sizeof(g_RsaPaddingRaw2);
//        l_rsaPara.cmdId = CMD_RSA_SIGN_PKCS1_OPER;
//    }
//    else if(0 == memcmp(operFlag, "verify", 6))
//    {
//        l_rsaPara.m_pInput = g_RsaPcks1Verify2;
//        l_rsaPara.m_InputLen = 256U;
//        l_rsaPara.cmdId = CMD_RSA_VERIFY_PKCS1_OPER;
//    }
    // encrypt data
    if(0 == memcmp(operFlag, "enc", 3))
    {
    	// get the string you want to encrypt 
	printf("Please input the string you want to encrypt: \n");
	gets(rsa_raw_Data);
	printf("The str you input is below: %s\n", rsa_raw_Data);
	printf("Your encrypt data will be sent by mosquitto\n");
	l_rsaPara.m_pInput = rsa_raw_Data;
	l_rsaPara.m_InputLen = sizeof(rsa_raw_Data);
	l_rsaPara.cmdId = CMD_RSA_ENC_PKCS1_OPER;
    }
    // decrypt data
    else if(0 == memcmp(operFlag, "dec", 3))
    {
        // get the string you want to decrypt
	printf("Please input the string you want to decrypt: \n");
	gets(rsa_encrypt_temp_Data);
	printf("The str you input is below: %s\n", rsa_encrypt_temp_Data);
	hex2str(rsa_encrypt_temp_Data);
	l_rsaPara.m_pInput = rsa_encrypt_Data;
	l_rsaPara.m_InputLen = 256U;
	l_rsaPara.cmdId = CMD_RSA_DEC_PKCS1_OPER;
    }
    // error
    else
    {
    	printf("Please enter correct arguments\n");
    	return ;
    }
    g_CryptoVerifyCa_Rsa(l_rsaPara);
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
	printf(" Please enter correct arguments \n");
	printf(" rsa2048 enc \n    or \n rsa2048 dec\n");
	return 0;
    }
    if (0 == memcmp(argv[1], "rsa2048", 7))
    {
        printf("Entry rsa1024 operation\n");
        /* rsa2048 + sign/verify/enc/dec + pcks1/nopadding */
        
        Rsa2048_Test(argv[2], EN_KEY_2048, rsa_Output2048, 256);
        
    } else {
    	printf(" Please enter correct arguments \n");
	printf(" rsa2048 enc \n    or \n rsa2048 dec\n");
    }  
    return 0;
}
