#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */

#define PTA_RPI3LED_UUID \
		{ 0xeef266dd, 0x883d, 0x414d, \
			{ 0x82, 0xf4, 0xdc, 0x94, 0x88, 0x08, 0xd0, 0x42}}
/* The function IDs implemented in this TA */

#define GPIO_ON             0
#define GPIO_OFF            1
#define TEST_PSEUDO_TA      2

int main(void)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = PTA_RPI3LED_UUID;
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
	
	
	printf("\n*****TA value = %d\n******", op.params[0].value.a);


	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
                                                   TEEC_NONE,
                                                   TEEC_NONE,
                                                   TEEC_NONE);


	res = TEEC_InvokeCommand(&sess, GPIO_ON, &op,
				 &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
	
	printf("\n*****TA sets led on\n******");


	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 */

	TEEC_CloseSession(&sess);

	TEEC_FinalizeContext(&ctx);

	return 0;
}

