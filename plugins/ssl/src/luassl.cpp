#include "luassl.h"

typedef struct  
{
	RSA *rsa;
} pkey_data, *lp_pkey_data;

#define checksslpkey(L) \
	(lp_pkey_data) luaL_checkudata(L, 1, "microhelix.luassl_pubkey")

static const struct luaL_Reg luasslpubkey_m[] =
{
	{ "__gc", luassl_deletepubkey },
	{ "close", luassl_deletepubkey },
	{ "verify", luassl_verifypubkey },
	{ NULL, NULL }
};

static const struct luaL_Reg luassl_f[] =
{
	{ "newPublicKey", luassl_newpubkey },
	{ NULL, NULL }
};

extern "C"
{
	int DLL_PUBLIC luaopen_luassl(lua_State *L)
	{
		luaL_newmetatable(L, "microhelix.luassl_pubkey");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
		luaL_setfuncs(L, luasslpubkey_m, 0);

		luaL_newlib(L, luassl_f);

		return 1;
	}
}

size_t calcDecodeLength(const char* b64input)
{
	size_t len = strlen(b64input),
		padding = 0;

	if (b64input[len - 1] == '=' && b64input[len - 2] == '=') //last two chars are =
		padding = 2;
	else if (b64input[len - 1] == '=') //last char is =
		padding = 1;

	return (len * 3) / 4 - padding;
}

int Base64Decode(const char* b64message, unsigned char** buffer, size_t* length)
{
	BIO *bio, *b64;

	size_t decodeLen = calcDecodeLength(b64message);
	*buffer = (unsigned char*)malloc(decodeLen + 1);
	(*buffer)[decodeLen] = '\0';

	bio = BIO_new_mem_buf(b64message, -1);
	b64 = BIO_new(BIO_f_base64());
	bio = BIO_push(b64, bio);

	BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
	*length = BIO_read(bio, *buffer, strlen(b64message));
	BIO_free_all(bio);

	return 0;
}

int luassl_newpubkey(lua_State *L)
{
	if (lua_gettop(L) == 1)
	{
		lp_pkey_data ptr = (lp_pkey_data)lua_newuserdata(L, sizeof(pkey_data));
		luaL_setmetatable(L, "microhelix.luassl_pubkey");
		const char* str = luaL_checkstring(L, 1);
		ptr->rsa = NULL;
		BIO *buffer;
		buffer = BIO_new_mem_buf(str, strlen(str));
		PEM_read_bio_RSA_PUBKEY(buffer, &(ptr->rsa), 0, NULL);
		ERR_print_errors_fp(stdout);
		BIO_free_all(buffer);
		if (ptr->rsa == NULL)
		{
			lua_pop(L, 1);
			lua_pushnil(L);
		}
		return 1;
	}
	else
		luaL_error(L, "SSL newPublicKey expects a string parameter (the public key in PEM format).");
	lua_pushnil(L);
	return 1;
}

int luassl_deletepubkey(lua_State *L)
{
	lp_pkey_data ptr = checksslpkey(L);
	if (ptr != NULL)
	{
		if(ptr->rsa != NULL)
			RSA_free(ptr->rsa);
		ptr->rsa = NULL;
	}
	return 0;
}

int luassl_verifypubkey(lua_State *L)
{
	if (lua_gettop(L) == 3)
	{
		lp_pkey_data ptr = checksslpkey(L);
		const char* message = luaL_checkstring(L, 2); // The original message.
		const char* signature = luaL_checkstring(L, 3); // The base64 signature
		unsigned char md_value[EVP_MAX_MD_SIZE];
		unsigned int md_len;

		// Calculate SHA1 hash of input data.
		EVP_MD_CTX *mdctx;
		mdctx = EVP_MD_CTX_create();
		EVP_DigestInit_ex(mdctx, EVP_sha1(), NULL);
		EVP_DigestUpdate(mdctx, message, strlen(message));
		EVP_DigestFinal_ex(mdctx, md_value, &md_len);
		EVP_MD_CTX_destroy(mdctx);

		unsigned char* decodedSignature;
		size_t decodedSignatureLen;
		Base64Decode(signature, &decodedSignature, &decodedSignatureLen);
		lua_pushboolean(L, RSA_verify(NID_sha1, md_value, md_len, decodedSignature, decodedSignatureLen, ptr->rsa) == 1);
		free(decodedSignature);
		return 1;
	}
	else
		luaL_error(L, "SSL publicKey.verify expects three parameters (publicKey, base64Signature, messageToVerify).");
	lua_pushboolean(L, false);
	return 1;
}
