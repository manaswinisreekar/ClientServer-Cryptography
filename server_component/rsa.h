int generate_shared_secret();
void XOR_encrypt(unsigned char* input, unsigned int inlen, unsigned char* output, int *pOlen, int key);
int XOR_ecrypt_int(int num, int key);
void rsa_generate(int *modulus, int *pubExponent, int *privExponent);
void rsa_encrypt(int n, int e, int m, int *pEncryptedMsg);
void rsa_decrypt(int n, int d, int *pMsg, int c);