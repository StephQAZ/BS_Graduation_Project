#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */

#define PTA_RPI3DHT11_UUID \
	{ 0xbd867e69, 0x58fb, 0x4675, \
		{ 0x83, 0x26, 0x5a, 0x68, 0xa3, 0xbf, 0xcc, 0x7f}}
/* The function IDs implemented in this TA */

#define GPIO_ON             0
#define GPIO_OFF            1
#define TEST_PSEUDO_TA      2
#define DHT11_READ_DATA		3

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("please input with 1 argument\n");
		printf("input 0 : GPIO_ON\n");
		printf("input 1 : GPIO_OFF\n");
		printf("input 3 : DHT11_READ_DATA\n");
		return 0;
	}
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = PTA_RPI3DHT11_UUID;
	uint32_t err_origin;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);

	/*
	 * Execute a function in the TA by invoking it, 
	 *
	 * The value of command ID part and how the parameters are
	 * interpreted is part of the interface provided by the TA.
	 */
	
	/* Clear the TEEC_Operation struct */	
	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
                                                   TEEC_NONE,
                                                   TEEC_NONE,
                                                   TEEC_NONE);


	res = TEEC_InvokeCommand(&sess, TEST_PSEUDO_TA, &op,
				 &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	
	
	printf("\n*****TA value = %d ******\n", op.params[0].value.a);

	// GPIO_ON
	if (strcmp(argv[1], "0") == 0) {
		/* Clear the TEEC_Operation struct */
		memset(&op, 0, sizeof(op));

		op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
		                                           TEEC_NONE,
		                                           TEEC_NONE,
		                                           TEEC_NONE);


		res = TEEC_InvokeCommand(&sess, GPIO_ON, &op,
					 &err_origin);
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				res, err_origin);
	
		printf("\n*****TA value = %d ******\n", op.params[0].value.a);
	}

	// GPIO_OFF
	if (strcmp(argv[1], "1") == 0) {
		/* Clear the TEEC_Operation struct */
		memset(&op, 0, sizeof(op));

		op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
		                                           TEEC_NONE,
		                                           TEEC_NONE,
		                                           TEEC_NONE);


		res = TEEC_InvokeCommand(&sess, GPIO_OFF, &op,
					 &err_origin);
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				res, err_origin);
	
		printf("\n*****TA value = %d ******\n", op.params[0].value.b);
	}

	// DHT11 read data
	if (strcmp(argv[1], "3") == 0) {
		/* Clear the TEEC_Operation struct */
		memset(&op, 0, sizeof(op));

		op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_OUTPUT,
										 TEEC_VALUE_OUTPUT,
										 TEEC_NONE,
										 TEEC_NONE);


		res = TEEC_InvokeCommand(&sess, DHT11_READ_DATA, &op,
								 &err_origin);
		if (res != TEEC_SUCCESS)
			errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				 res, err_origin);

		printf("\n**************************\n start to receive dht11 data\n");
		printf("RH:%d.%d\n", op.params[0].value.a, op.params[0].value.b);
		printf("TMP:%d.%d\n", op.params[1].value.a, op.params[1].value.b);
	}
	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 */

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);

	return 0;
}

