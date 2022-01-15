/*
 * Copyright (c) 2017, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <err.h>
#include <stdio.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* TA API: UUID and command IDs */
#include <secure_storage_ta.h>

/* TEE resources */
struct test_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

void prepare_tee_session(struct test_ctx *ctx)
{
	TEEC_UUID uuid = TA_SECURE_STORAGE_UUID;
	uint32_t origin;
	TEEC_Result res;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx->ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	/* Open a session with the TA */
	res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, origin);
}

void terminate_tee_session(struct test_ctx *ctx)
{
	TEEC_CloseSession(&ctx->sess);
	TEEC_FinalizeContext(&ctx->ctx);
}

TEEC_Result read_secure_object(struct test_ctx *ctx, char *id,
			char *data, size_t data_len)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_OUTPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	op.params[1].tmpref.buffer = data;
	op.params[1].tmpref.size = data_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_SECURE_STORAGE_CMD_READ_RAW,
				 &op, &origin);
	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_SHORT_BUFFER:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command READ_RAW failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result write_secure_object(struct test_ctx *ctx, char *id,
			char *data, size_t data_len)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	op.params[1].tmpref.buffer = data;
	op.params[1].tmpref.size = data_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_SECURE_STORAGE_CMD_WRITE_RAW,
				 &op, &origin);
	if (res != TEEC_SUCCESS)
		printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);

	switch (res) {
	case TEEC_SUCCESS:
		break;
	default:
		printf("Command WRITE_RAW failed: 0x%x / %u\n", res, origin);
	}

	return res;
}

TEEC_Result delete_secure_object(struct test_ctx *ctx, char *id)
{
	TEEC_Operation op;
	uint32_t origin;
	TEEC_Result res;
	size_t id_len = strlen(id);

	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE, TEEC_NONE);

	op.params[0].tmpref.buffer = id;
	op.params[0].tmpref.size = id_len;

	res = TEEC_InvokeCommand(&ctx->sess,
				 TA_SECURE_STORAGE_CMD_DELETE,
				 &op, &origin);

	switch (res) {
	case TEEC_SUCCESS:
	case TEEC_ERROR_ITEM_NOT_FOUND:
		break;
	default:
		printf("Command DELETE failed: 0x%x / %u\n", res, origin);
	}

	return res;
}


#define TEST_OBJECT_SIZE	4096
const char obj1_data[] = "-----BEGIN RSA PRIVATE KEY-----\
MIIEowIBAAKCAQEAyqogNEypkZDIU4Yo1GbTpk7lNUo0vZ9T5lj3BvldKYDOwqWc\
oOJjecHcF539lZM6vJ3T9ft67jG3OQEYh6csANEu+wTto5LJopmmTWuCFJoABsYW\
VoNIrScOJ2tHLYTUr+q3AwC+zZHUM5ZBYUP5NxgUZ/BIaKH+UHPazrv/LkkMFwnd\
Yj+87zBF202pSOav40RnMs+jbfZhSZd0WN5X1fMHlo6wrk38rfD8SeIYQ5N6IXUm\
0n+jH+m0LkNN6Q53C+Qce4RXZ0O8tELtFjjDA/5aPdaQwBUAoCwRIt0xESouy0Eq\
VDte4ErnkJmYBk6tUTYMuqa4Y6Qlm7XW44TQrwIDAQABAoIBAQDICs9I8Fb8MS4b\
i7Rnm6vUX9HNHRccCNW27B/BiYiUu+japsAI6g2IlHsb8L3q023J2fvrtLQtw3L4\
WIWO+Mrtb41xBvdOW0ieMA3AI5s9nJjkFsjb0jB9gaxt6m1Sl+ecUREebLDplsUf\
unOEf6TR5fnWco32gdfWL9VN9LE4S+zRnfOC14hlTrC43xx8XI70rBOPWulPlzCr\
QifLMZ1R+FCVuXXagH4zDMD70n0i2o4r5AIEb62jDUxzxx1TxAmSl2WLrLT9nTzH\
HZxCLD3OZXhrFPHbF8M6gtgxcCnjgT/Hg71mhKso3mdVUtTUJW2463lOV6Hk0xYc\
7YiGT/zxAoGBAPf8bhxvaP5Qfm58FCtyEVHCj1zjefCfY+fwsmA20SA8hQ3Zfn0k\
Ust0m3irC0Va3GZ0YwF5AMXlDsMfGIh7N0j5eeorjZSgtSqRAds2AoNyxL+dP5pk\
Sp3Zii5kMCcoM13N6cwRpSggr6KrquEKpolqo8o2wXvdvIxVd84Ux6bXAoGBANE2\
wRtoVnTuC0pK41+NJoldiEDlghcDnXh4i/pIK+61rtG5otENKd/5+e+Xfw6QwFlU\
E4T8eWYS4sNvIrm0Kn5igjmtQi6mJRrNEIPaiPdEYBAOkHXoxDL8PSzUgu1Aejl7\
TJexWZ2TSA1ceAw1qMouYKQfBIjW8sJC1h6KzeHpAoGAS9F0xOLXIfHwRc1PyVyt\
qOaoOEkgLg49WFuB3eLEm2uFbo+RmacyXRwAsKry3jiCbBynbHrXXqnCkqt+L7Qk\
Wf5UZOD9/3QHD8WX4LKpoBDF0KO+H3EMUyiAk+BzzHUBB2v+UDMQ4KzlpgGAVB+S\
fS8Kdre4/ir0C5/R6kL2K68CgYBlovIBPJjzyUkW2HbJTqswBI5S6KxgPdNcWUO3\
XSRqnTzGiiZ85KY5Nxmiubhx/QRU4TFyY60eV4Juayk1ij8ykuys1GzYeMKB2Klz\
RNFRPphkdI7dmSznj/6wLrVESRSXpDy7XN8qXAie5RM060SKpCky0hbD25inowye\
XdRTMQKBgFudQObPKLgVzxaYS/BeCOhgUm3PAZVW8TLNPS4nH8OZUkiGEJFOc1/t\
kEeqYF4UMOi2+Q5aRNYqnfg0udgiWqC3dcQ7W0IaJJ3koQRcfkZgtrGPUcb4EeaD\
ItiX8UZpG3VyBuU8DRR7z4BtvpdSGou8Z/Q0zNgUZUh+q73W6HQJ\
-----END RSA PRIVATE KEY-----";  
char read_data[TEST_OBJECT_SIZE];


void createPrivateKey() {
	FILE *fpWrite = fopen("./ca.key", "w");
	if (fpWrite == NULL) return 0;
	fprintf(fpWrite, "%s\n", "-----BEGIN RSA PRIVATE KEY-----");
	for (int i = 0; i < 24; i++) {
	for (int j = 0; j < 64; j++) {
		fprintf(fpWrite, "%c", *(read_data + 31 + i * 64 + j));
	}
	fprintf(fpWrite, "\n");
	}
	for (int j = 0; j < 52; j++) {
	fprintf(fpWrite, "%c", *(read_data + 31 + 24 * 64 + j));
	}
	fprintf(fpWrite, "\n");
	fprintf(fpWrite, "%s\n", "-----END RSA PRIVATE KEY-----");
	fclose(fpWrite);
	return 0;
}


int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("please enter with extra argument\n");
		return 0;
	}
	struct test_ctx ctx;
	char obj1_id[] = "object#1";		/* string identification for the object */
	TEEC_Result res;

	printf("Prepare session with the TA\n");
	prepare_tee_session(&ctx);

	/*
	 * Create object, read it, delete it.
	 */

	printf("- Create and load object in the TA secure storage\n");

	// case 0 : write private key in secure world
	if (strcmp(argv[1], "0") == 0) {
		printf("\nTest on object \"%s\"\n", obj1_id);
		printf("\n");
		printf("############## The private key is below ##########\n");
		printf("%s\n", obj1_data);
		res = write_secure_object(&ctx, obj1_id, obj1_data, sizeof(obj1_data));
		printf("\nsizeof(obj1_data) is : %d\n", sizeof(obj1_data));
		if (res != TEEC_SUCCESS) {
			errx(1, "Failed to create an object in the secure storage");
		}
	}
	// case 1 : read private key from secure world
        else if (strcmp(argv[1], "1") == 0) {
		res = read_secure_object(&ctx, obj1_id, read_data, sizeof(read_data));
		if (res != TEEC_SUCCESS) {
			errx(1, "Failed to read an object from the secure storage");
		}
		if (memcmp(obj1_data, read_data, sizeof(obj1_data))) {
			errx(1, "Unexpected content found in secure storage");
		}
		printf("\n ############## the read data is below ############## \n");
		printf("%s\n", read_data);
		createPrivateKey();
		printf("\nWe have generate ca.key file\n");
	}
	// case 2 : delete private key in secure world
	else if (strcmp(argv[1], "2") == 0) {
		printf("- Delete the object\n");
		res = delete_secure_object(&ctx, obj1_id);
		if (res != TEEC_SUCCESS) {
			errx(1, "Failed to delete the object: 0x%x", res);
		}
	}
	terminate_tee_session(&ctx);
	return 0;
}
