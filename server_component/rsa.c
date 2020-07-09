#include <stdio.h> 
#include <stdlib.h>
#include <syslog.h>

#include <math.h>
#include <time.h>


#ifdef DEBUG
#define D(x)    x
#else
#define D(x)
#endif

#define LOGINFO(fmt, args...)    { syslog(LOG_INFO, fmt, ## args); printf(fmt, ## args); }
#define LOGERR(fmt, args...)     { syslog(LOG_CRIT, fmt, ## args); fprintf(stderr, fmt, ## args); }
 
void derivePublicExponent();
long int derivePrivExponent(long int);
void encrypt();
void decrypt();

/*
 * Method to generate Symmetric key or X-OR key
*/
int generate_shared_secret()
{
  srand(time(NULL));   
  // returns a random integer value between 0 and RAND_MAX
  return ((rand() % 125) + 2); // Avoid 0, 1
}

/*
 *  Encrypting a char array using XOR key
*/
void XOR_encrypt(unsigned char* input, unsigned int inlen, unsigned char* output, int *pOlen, int key) {
  int i;
  for (i = 0; i < inlen; i++) {
	output[i] = input[i] ^ key;
  }
  *pOlen = i;
}

/*
 * Encrypting an integer using symmetric key
*/
int XOR_ecrypt_int(int num, int key) {
 LOGINFO ("SHARED SECRET IN XOR =%d", key);
 LOGINFO ("XOR MESSAGE =%d", num);
 LOGINFO ("XOR OPERATION = %d", (num ^ key));
 return num ^ key;
}

/*
 * Checking if an integer is Prime or not.
*/
int checkPrime(int prime) 
{ 
	int counter;
  
  for(counter = 2; counter <= prime - 1; counter++) 
	{ 
    	if ( prime % counter == 0) 
	        return 0;
	}
	
	return 1; 
}

/*
 * generation of prime number with setting an upper bound and lower bound
*/
int generatePrime(int upper, int lower)
{
    int prime, isPrime=0;
    srand(time(NULL));// Seed Rand
    while (isPrime == 0)
    {
      // to avoid 0, 1 etc.
        prime = rand() % (upper - lower) + lower;
        isPrime = checkPrime(prime);
    }
    return prime;
}

/*
 * Computation of gcd(greatest common divisor)
*/

int gcd (int a, int b){
	if(b == 0)
		return a;
	else
		return gcd(b, a%b);
}

/*
 *  Calulating private exponent d.
*/

int mulInv(int exp, int totient)
{
	int n0 = totient, t, q;
	int x0 = 0, x1 = 1;
	if (totient == 1) 
		return 1;
	while (exp > 1) {
		q = exp / totient;
		t = totient, totient = exp % totient, exp = t;
		t = x0, x0 = x1 - q * x0, x1 = t;    
	}
  
	if (x1 < 0) 
		x1 += n0;
	
	return x1;
}

/*
 *  Public Exponent
 * Choose e such that e > 1 and coprime to totient and gcd (e, totient) must be equal to 1
 * e is the public key
*/
int derivePubExponent(int totient)
{
  int upper = 128; //setting the upper bound
  int lower = 17; // setting the lower bound
  int pube = 0;   
  
   pube= generatePrime(upper, lower);
   while (gcd(totient, pube) != 1)
   {
     pube = generatePrime(upper, lower);
   }
  return pube;
}

/*
 * Generation of RSA
 */
void rsa_generate(int *modulus, int *pubExponent, int *privExponent) 
{
  int p, q;
  int upper = 216; //  max 8 bits
  int lower = upper/2; // max 7 bits
  int totient = 0;

  p = generatePrime(upper, lower);
  LOGINFO("fsr: Prime p = %d", p);

  q = generatePrime(upper, lower);
  while (p == q)
  {
    q = generatePrime(upper, lower);
  }
	LOGINFO("fsr: Prime q = %d", q);

	*modulus = p * q;
	totient = (p-1) * (q-1);

  // Choose e such that e > 1 and coprime to totient which means gcd (e, totient) must be equal to 1, e is the public key
  // Choose d such that it satisfies the equation de = 1 + k (totient), d is the private key
  //  e * d % totient == 1.

 
  do{
   
     *pubExponent = derivePubExponent(totient);
     *privExponent = mulInv(*pubExponent, totient);
    } while( ((*pubExponent) * (*privExponent)) % totient != 1 ); // numbers which dont have an inverse	}
}


// c = m^e mod n

int squareandmul(int m, int e, int n){
  int result = 1;
  int i = 0;
  LOGINFO("Enter squareandmul\n");
  LOGINFO("m = %d, e=%d, n = %d\n", m, e, n);
	for ( i=0; i< e; i++) 
  {
    result *= m;
    result %= n;
  }
  
  return result % n;
}

/*
 *  RSA Encryption
*/

// We calculated Cipher text using the formula (c = m^e mod n) 
void rsa_encrypt(int n, int e, int m, int *pEncryptedMsg)
{
    LOGINFO("Enter rsa_encrypt\n");
    LOGINFO("n = %d, e=%d, m = %d\n", n, e, m);

    *pEncryptedMsg = squareandmul(m, e, n);
    LOGINFO("*pEncryptedMsg = %d", *pEncryptedMsg);   
    LOGINFO("rsa_encrypt: Exit\n");
}

/*
 *  RSA Decryption
*/

//After getting the value of cwhich is cipher text, we decrypt the cipher text using formula (m = c^d mod n) and d is the private key.
void rsa_decrypt(int n, int d, int *pMsg, int c)
{
    LOGINFO("Enter rsa_decrypt\n");
    LOGINFO("n = %d, d = %d, c = %d\n", n, d, c);

    *pMsg = squareandmul(c, d, n);
    LOGINFO("*pMsg = %d", *pMsg);
}
