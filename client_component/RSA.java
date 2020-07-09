import java.util.Random;
public class RSA {
	int n;
	int e;
	int d;

	// Returns floor of square root of x
	private int floorSqrt(int x) {
		// Base cases
		if (x == 0 || x == 1)
			return x;

		// Starting from 1, try all numbers until
		// i*i is greater than or equal to x.
		int inc = 1, result = 1;
		while (result < x) {
			if (result == x)
				return result;
			inc++;
			result = inc * inc;
		}
		return inc - 1;
	}

	private int checkPrime(int prime) {
		int counter;
		int sqrt;
		sqrt = floorSqrt(prime);

		for (counter = 2; counter <= sqrt; counter++) {
			if (prime % counter == 0)
				return 0;
		}
		return 1;
	}

	private int generatePrime(int upper, int lower) {
		int prime = 0, isPrime = 0;
		Random random = new Random();
		while (isPrime == 0) {
			prime = random.nextInt(upper - lower) + lower;
			isPrime = checkPrime(prime);
		}
		return prime;
	}

	private int gcd(int a, int b) {
		if (b == 0)
			return a;
		else
			return gcd(b, a % b);
	}

	// https://rosettacode.org/wiki/Modular_inverse
	private int mulInv(int a, int n) {
		int n0 = n, t, q;
		int x0 = 0, x1 = 1;
		if (n == 1)
			return 1;
		while (a > 1) {
			q = a / n;
			t = n;
			n = a % n;
			a = t;
			t = x0;
			x0 = x1 - q * x0;
			x1 = t;
		}
		if (x1 < 0)
			x1 += n0;
		// (a*x1)%n == 1
		return x1;
	}

	
	int squareandmul(int m, int e, int n) {
		int result = 1;
		int i = 0;
		for (i = 0; i < e; i++) {
			result *= m;
			result %= n;
		}
		return result % n;
	}

	
	//generating key pair
	void generateKeyPair() {
		int p, q;
		int upper = 216;
		int lower = upper / 2;
		int totient = 0;
		
		// Generation of primenumbers p and q
		p = generatePrime(upper, lower);
		q = generatePrime(upper, lower);
		
		// If they are equal, generate new q
		while (p == q) { q = generatePrime(upper, lower); }

		// Generate n and toitient
		n = p * q;
		totient = (p - 1) * (q - 1);
		
		
		// Generate e such that e is relatively prime to totient
		// Generate d as the multiplicative inverse of e
		
		do {
			e = derivePubExponent(totient);
			d = mulInv(e, totient);
		} while (e * d % totient != 1); 
	}

	// Getting public exponent e 
	int derivePubExponent(int totient) {
		int upper = 128;
		int lower = 17;
		int pube = 0;

		pube = generatePrime(upper, lower);
		
		while (gcd(totient, pube) != 1) {
			pube = generatePrime(upper, lower);
		}
		return pube;
	}

	// Cipher text is calculated using the equation c = m^e mod n 
	public int encrypt(int m) {
		System.out.println("Enter rsa_encryption\n");
		return squareandmul(m, e, n);
	}

	// With the help of c and d we decrypt message using equation m = c^d mod n
	// where d is the private key.
	public int decrypt(int c) {
		System.out.println("Enter rsa_decryption\n");
		return squareandmul(c, d, n);

	}
}