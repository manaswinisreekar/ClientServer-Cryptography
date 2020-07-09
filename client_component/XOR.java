public class XOR {
	static int secretkey;
//
	public static int getSecret() {
		return secretkey;
	}
    //Decryption of the shared symmetric key
	public static void setSecret(int cipher_xor, RSA rsa) {
		secretkey = rsa.decrypt(cipher_xor);
		System.out.println("Shared secret key is " + secretkey);
	}

	
	//Decrypting image byte
	
	public static byte decryptByte(byte input) {
		
		return (byte) (input ^ secretkey);
		
	}
	//Encryption of Frames and Resoultion and Decrypting the Size
	public static int encryptdecryptInt(int input) {
		return input ^ secretkey;
	}
	
	
}