

#include <capture.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "string.h"
#include <sys/time.h>
#include "rsa.h"
#include <netinet/in.h>
#include <arpa/inet.h>


#ifdef DEBUG
#define D(x)    x
#else
#define D(x)
#endif

#define LOGINFO(fmt, args...)    { syslog(LOG_INFO, fmt, ## args); printf(fmt, ## args); }
#define LOGERR(fmt, args...)     { syslog(LOG_CRIT, fmt, ## args); fprintf(stderr, fmt, ## args); }

#define MY_PORT		8082
#define MAXBUF		1024

typedef struct
{
  int32_t frequency;
  int32_t resWidth;
  int32_t resHeight;
}__attribute__((packed, aligned(4))) sockCliData;


typedef struct
{
  int frequency;
  int resWidth;
  int resHeight;
}__attribute__((packed, aligned(4))) ClientInput;

typedef struct
{
  int32_t modulus;
  int32_t exponent;
}__attribute__((packed, aligned(4))) mycliPubKey;

typedef struct
{
  int modulus;
  int pubexp;
  int privexp;
}__attribute__((packed, aligned(4))) RSAKey;

mycliPubKey cliPubKey = {0, 0};


sockCliData streamCliData = {0, 0, 0};
ClientInput cliData =  {0, 0, 0};
RSAKey cliRSA =  {0, 0, 0};
RSAKey serRSA =  {0, 0, 0};

int cipherText;
int shared_secret = 0;

//sending to the client
int SendInteger(int clientfd, int number)
{
  ssize_t bytesWritten = 0;
  int result = 0;
  LOGINFO("Sending number = %d\n", number);
  int32_t nlNumber = htonl(number);  
  bytesWritten = write(clientfd, (void*)&nlNumber, sizeof(int32_t)); //sending to the client
  if (-1 != bytesWritten)
  {
    LOGINFO("Write to socket: bytesWritten = %d\n", bytesWritten);
  }
  else
  {
    LOGINFO("Write to socket failed (Size)\n");
    result = -1;
  }
  return result;
}

//sending byte array to the client
int SendByteArray(int clientfd, unsigned char* data, int size)
{
  int result = 0;
  ssize_t bytesWritten = 0;
	LOGINFO("Sending Byte Array\n");
	bytesWritten = write(clientfd, data, size);
	if (-1 != bytesWritten)
	{
		LOGINFO("Write to socket: data = %d\n", bytesWritten);
	}
	else
	{
		LOGINFO("Write to socket failed (Image)\n");
		result = 1;;
	}
  return result;  
}
/* This is the main function for the server program. Here we create socket, bind it to a client and
listen to it on a certain port. When the client connection is accepted, a process for the particular client
is created.
*/
int main()
{
  openlog("server", LOG_PID | LOG_CONS, LOG_USER);
  LOGINFO ("Enter main");
  struct sockaddr_in client_addr;
  int clientfd = 0;
  unsigned char inputBuf1[8];
  unsigned char inputBuf2[12];
 
  pid_t pid = 0;
  
  int sockfd;
  int portno = MY_PORT;
  struct sockaddr_in serv_addr;     

  /*---Create streaming socket---*/
  LOGINFO ("Creating Socket");
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    perror("ERROR opening socket");

  /*---Initialize address/port ---*/
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  /*---Assign a port number to the socket---*/
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
  sizeof(serv_addr)) < 0) 
  perror("ERROR on binding");
  /*--- "listening socket"---*/
  if ( listen(sockfd, 10) != 0 )
  {
    perror("socket--listen");
    exit(errno);
  }

 
  while (1)
  {
    socklen_t addrlen = sizeof(client_addr);
    /*---accept a connection---*/
    clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);

    if (0 < clientfd)
    {
      LOGINFO ("Socket connection established with %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
      LOGINFO ("Executing fork for clientfd = %d", clientfd);
      pid = fork();
      
      if (pid == 0)
      {
        LOGINFO ("pid == 0, running client thread");
       
        int count = 0;
        int bytesRead = -1;

        // Recieve Client RSA public key
        {
          count = 8;
          do
          {
            bytesRead = read(clientfd, &(inputBuf1[sizeof(inputBuf1) - count]), count);
            count -= bytesRead;
            LOGINFO ("bytesRead = %d, count = %d", bytesRead, count);
          } while (bytesRead != -1 && count > 0);
          memcpy(&cliPubKey, &inputBuf1, 8);

          cliRSA.modulus = ntohl(cliPubKey.modulus);
          cliRSA.pubexp = ntohl(cliPubKey.exponent);
        
          LOGINFO ("client RSA modulus = %d", cliRSA.modulus);
          LOGINFO ("client RSA pubexponent = %d", cliRSA.pubexp);
        }
               
        // Generating RSA Keypair
        rsa_generate(&serRSA.modulus, &serRSA.pubexp, &serRSA.privexp);
        LOGINFO (" server RSA modulus = %d", serRSA.modulus);
        LOGINFO ("server RSA pubexponent = %d", serRSA.pubexp);
        LOGINFO ("server RSA privexponent = %d", serRSA.privexp);
        
        // Sending server public key(modulus and exponent)
        SendInteger(clientfd, serRSA.modulus);
        SendInteger(clientfd, serRSA.pubexp);
  
        // XOR key generation
        shared_secret = generate_shared_secret();
        LOGINFO ("shared_secret = %d", shared_secret);

        // Encrypts XOR key with RSA public key the key which is sent from the client
        rsa_encrypt(cliRSA.modulus, cliRSA.pubexp, shared_secret, &cipherText);
        LOGINFO(" Encrypted message %d", cipherText);

        // Sending encrypted XOR key to the client
        SendInteger(clientfd, cipherText);
        
        // Receive fps and resolution
        {
          count = 12;
          bytesRead = -1; 
          do
          {
            bytesRead = read(clientfd, &(inputBuf2[sizeof(inputBuf2) - count]), count);
            count -= bytesRead;
            LOGINFO ("bytesRead = %d, count = %d", bytesRead, count);
          } while (bytesRead != -1 && count > 0);
          memcpy(&streamCliData, &inputBuf2, 12);
        
          // Send the integers in network-byte-order, this need to be converted to host-byte-order.
          LOGINFO ("frequency = %d", ntohl(streamCliData.frequency));
          LOGINFO ("resolution Width = %d", ntohl(streamCliData.resWidth));
          LOGINFO ("resolution Height = %d", ntohl(streamCliData.resHeight));

          // Decryption of fps 
      	 
          int decryptedfreq=   XOR_ecrypt_int(ntohl(streamCliData.frequency), shared_secret);
           cliData.frequency  = decryptedfreq;
          // Decryption of reslotions 
          LOGINFO ("SHARED SECRET =%d", shared_secret);
          int decryptedWidth = XOR_ecrypt_int(ntohl(streamCliData.resWidth), shared_secret);
          LOGINFO (" Decrypted width = %d", decryptedWidth);
          cliData.resWidth = decryptedWidth;
          LOGINFO (" width = %d", cliData.resWidth);

          int decryptedHeight = XOR_ecrypt_int(ntohl(streamCliData.resHeight), shared_secret);
          LOGINFO ("Decrypted height = %d", decryptedHeight);
          cliData.resHeight = decryptedHeight;
          LOGINFO (" Height = %d", cliData.resHeight);

          LOGINFO ("Decrypt frequency = %d", cliData.frequency);
          LOGINFO ("Decrypt resWidth = %d", cliData.resWidth);
          LOGINFO ("Decrypt resHeight = %d", cliData.resHeight);
        }

        // Sending image to the client that got connected
        capture_send_image(clientfd);
        
        //---Close data connection---
        close(clientfd);
        
        LOGINFO ("closes clientfd");
      }
      else if (pid > 0)
      {
        LOGINFO ("pid > 0");
      }
      else
      {
        // fork failed
        printf("fork() failed!\n");
        return 1;
      }
    }
  }//end of while

  
  close(sockfd);
  LOGINFO ("Socket is closed");  
  return EXIT_SUCCESS;
}

int capture_send_image(int clientfd);

int capture_send_image(int clientfd)
{
  LOGINFO ("Enter capture_send_image");
  
  media_stream *     stream = NULL;
  media_frame *frame;
  void *data = NULL;
  size_t size = 0;
  struct timeval tv_start, tv_end;
  int timeConsumed = 0, msec = 0;
  int numframes = 0;
  int isStop = 0;
  int fps = cliData.frequency;
  int resW = cliData.resWidth;
  int resH = cliData.resHeight;
  int nrFramesToSkip = 0;
  int framesSkipped = 0;

 // Setting resolutions width(wStr) and height(hStr)
  char wStr[5] = {0};
  char hStr[5] = {0};
  char fpsResStr[50] = {0};
  
  // converting width to string
  sprintf(wStr, "%d", resW);
  LOGINFO("Width= %d, Width Str= %s\n", resW, wStr);
  // converting height to string
  sprintf(hStr, "%d", resH);
  LOGINFO("Height= %d, Height Str= %s\n", resH, hStr);
  strncat(fpsResStr, "fps=30&resolution=", 18);
  strncat(fpsResStr, wStr, strnlen(wStr, 4));
  strncat(fpsResStr, "x", 1);
  strncat(fpsResStr, hStr, strnlen(hStr, 4));
  LOGINFO("Input fpsResStr= %s\n", fpsResStr);
    
  // Captures streams of image from the camera
 
  stream = capture_open_stream(IMAGE_JPEG, fpsResStr);
    if (stream == NULL) {
      LOGERR("Failed to open stream\n");
      closelog();
      return EXIT_FAILURE;
    }

  char* resolList = capture_get_optimal_resolutions_list 	((int)stream);
  LOGINFO("RresolList ==   %s\n", resolList);
  free (resolList);
  
 
  
  if (fps > 0)
    nrFramesToSkip = 30 / fps;
  LOGINFO("Requested FPS = %d, nrFramesToSkip = %d\n", fps, nrFramesToSkip);
  
  while (isStop == 0) //starting of while loop
  {
    gettimeofday(&tv_start, NULL);
    frame     = capture_get_frame(stream); //The actual capturing for the image.
    data      = capture_frame_data(frame); //This is where the image will be saved.
    size      = capture_frame_size (frame); //Gets the image's size
    if (framesSkipped < nrFramesToSkip)
    {
      framesSkipped++;
      capture_frame_free(frame);
      LOGINFO("framesSkipped = %d\n", framesSkipped);
      
      gettimeofday(&tv_end, NULL);    
      // calculate fps
      msec  = tv_end.tv_sec * 1000 + tv_end.tv_usec / 1000;
      msec -= tv_start.tv_sec * 1000 + tv_start.tv_usec / 1000;
      timeConsumed += msec;
      LOGINFO("Time consumed for this frame = %d mSec\n", msec);
      LOGINFO("Total timeConsumed = %d mSec\n", timeConsumed);
      continue;
    }
    
    framesSkipped = 0;
    
    int encrypted_size = 0;    
    LOGINFO("Size of image data = %d\n", size);    
    //encryption of the size using XOR with the key
    encrypted_size = XOR_ecrypt_int (size, shared_secret); 
    LOGINFO("Encrypted size = %d\n", encrypted_size);
    // Sending encryted size to the client
    SendInteger(clientfd, encrypted_size);

    int outLen = 0;
    unsigned char *pXorData = malloc(size);
    // Encrypt image data
    XOR_encrypt(data, size, pXorData, &outLen, shared_secret);
    // Sending image to the client
    isStop = SendByteArray(clientfd, pXorData, outLen);
   
    free(pXorData);
    capture_frame_free(frame);
    gettimeofday(&tv_end, NULL);
    
    //calculation fps
    msec  = tv_end.tv_sec * 1000 + tv_end.tv_usec / 1000;
    msec -= tv_start.tv_sec * 1000 + tv_start.tv_usec / 1000;
    timeConsumed += msec;
    numframes++;
    
    LOGINFO("Time consumed for this frame = %d mSec\n", msec);
    LOGINFO("Total timeConsumed = %d mSec\n", timeConsumed);
    LOGINFO("Total frames sent to client = %d\n", numframes);
  } // end of while

 
  capture_close_stream(stream);
  LOGINFO ("Closes the stream");
  return EXIT_SUCCESS;
}
