#include <reg51.h>
#include <stdio.h>
#include <stdlib.h>

// Configuration defines
#define MAX_INPUT_LENGTH 5  // Maximum input length for numbers
#define SECURITY_LEVEL 5    // Security level for primality test
#define ASCII_PRINTABLE_MIN 32  // Minimum printable ASCII value
#define ASCII_PRINTABLE_MAX 126 // Maximum printable ASCII value

// Typedefs
typedef unsigned char           u8;
typedef unsigned short int      u16;
typedef unsigned long int       u32;

typedef signed int              s32;

// UART function prototypes
void UART_Initialize();
void UART_TransmitChar(u8 c);
u8 UART_ReceiveChar();
void UART_TransmitString(u8* str);
u32 UART_ReceiveLong();
void UART_ReceiveInputString(char* buffer, int length);

// Math function prototypes
u32 ModularExponentiation(u32 base, u32 exp, u32 mod);
u32 CalculateGCD(u32 a, u32 b);
u32 GeneratePublicKeyE(u32 phi);
u32 CheckPrime(u32 n, u8 security_level);
u32 CalculateModularInverse(u32 e, u32 phi);

// Global variables
u16 p, q;
u32 n, phi, d , e ;

int main(void) {
   unsigned int choice;
   unsigned int plaintext, ciphertext, decrypted_value;
   unsigned char buffer[50];
   unsigned char Buf[10];

    // Initialize UART
    UART_Initialize();
    UART_TransmitString("8051 RSA Encryption/Decryption System\0");

    // Prompt user for operation choice
    while (1) {
        UART_TransmitString("Choose operation: \r\n 1. Encrypt. \r\n 2. Decrypt \r\n .Enter choice: ");
        choice = UART_ReceiveLong();
        
        if (choice == 1 || choice == 2) break;
        UART_TransmitString("Invalid choice. Please enter 1 or 2.");
    }

    // Input p (ensure it's prime)
    while (1) {
        UART_TransmitString("\r\nEnter prime number p: ");
        p = UART_ReceiveLong();
       
        if (CheckPrime(p, SECURITY_LEVEL)) {
            UART_TransmitString("\r\np is a valid prime number.");
            break;
        } else {
            UART_TransmitString("Error: p is not a prime number. Please try again.");
        }
    }

    // Input q (ensure it's prime)
    while (1) {
        UART_TransmitString("Enter prime number q: ");
        q = UART_ReceiveLong();
        

        if (CheckPrime(q, SECURITY_LEVEL)) {
            UART_TransmitString("\r\nq is a valid prime number.");
            break;
        } else {
            UART_TransmitString("\r\nError: q is not a prime number. Please try again.");
        }
    }

    // Calculate n and phi
    n = p * q;
    phi = (p - 1) * (q - 1);

    // Generate public key e
     while (1) {
        UART_TransmitString("\r\nEnter public key value e: ");
        e = UART_ReceiveLong();
        

        if (CalculateGCD(e, phi) == 1) {
            UART_TransmitString("\r\ne is a valid public key.");
            break;
        } 
    }

    // Calculate private key d
    d = CalculateModularInverse(e, phi);

    // Display keys
    sprintf(buffer, "\r\nPublic Key: (e = %lu, n = %lu)", e, n);
    UART_TransmitString(buffer);
    sprintf(buffer, "\r\nPrivate Key: (d = %lu, phi(n) = %lu)", d, phi);
    UART_TransmitString(buffer);

    // Perform selected operation
    if (choice == 1) {  // Encryption
        UART_TransmitString("\r\nEnter plaintext to encrypt (as integer): ");
        UART_ReceiveInputString(Buf, 10);
        plaintext = atoi(Buf);
       

        // Encrypt plaintext
        ciphertext = ModularExponentiation(plaintext, e, n);
        sprintf(buffer, "Ciphertext: %i", ciphertext);
        UART_TransmitString(buffer);
    } else if (choice == 2) {  // Decryption
        UART_TransmitString("Enter ciphertext to decrypt: ");
        UART_ReceiveInputString(Buf, 10);
        ciphertext = atoi(Buf);
        

        // Decrypt ciphertext
        decrypted_value = ModularExponentiation(ciphertext, d, n);
        sprintf(buffer, "Decrypted value: %i", decrypted_value);
        UART_TransmitString(buffer);

       
    }

 while(1);
    
}

// UART initialization
void UART_Initialize() {
    TMOD = 0x20;  // Timer1 in Mode2 (8-bit auto-reload)
    TH1 = 0xFD;   // Timer1 value for 9600 baud with 11.0592 MHz crystal
    SCON = 0x50;  // 8-bit UART mode, REN enabled
    TR1 = 1;      // Start Timer1
}

void UART_TransmitChar(char c) {
      SBUF = c;          // Load the character into UART buffer
    while (TI == 0);    // Wait for the transmission to complete
    TI = 0;     
}

u8 UART_ReceiveChar() {
    while (RI == 0);
    RI = 0;
    return SBUF;
}

void UART_TransmitString(u8* str) {
       while (*str != '\0') {      // Loop until the null terminator
        UART_TransmitChar(*str++);     // Transmit each character via UART
    }
		UART_TransmitChar('\0');
}

u32 UART_ReceiveLong() {
    u32 value = 0;
    u8 i;
    u8 c;

    for (i = 0; i < MAX_INPUT_LENGTH; i++) {
        c = UART_ReceiveChar();
        if (c == '\r' || c == '\n') break;  // End of input
        value = value * 10 + (c - '0');
    }
    return value;
}

void UART_ReceiveInputString(unsigned char* buffer, unsigned int length) {
   unsigned int i = 0;
   unsigned char ch;
    while (i < length) {
        ch = UART_ReceiveChar();
        if (ch == '\n' || ch == '\r') {
            break;
        }
        buffer[i++] = ch;
    }
}

// Modular exponentiation (square and multiply)
u32 ModularExponentiation(u32 base, u32 exp, u32 mod) {
    u16 result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = ((result * base) % mod);
        }
        exp = exp >> 1;
        base = ((base * base) % mod);
    }
    return result;
}

// GCD calculation
u32 CalculateGCD(u32 a, u32 b) {
    while (b != 0) {
       unsigned int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}


// Primality test using Fermat's method
u32 CheckPrime(u32 n, u8 security_level) {
    u8 i;
    if (n <= 1) return 0; // 0 and 1 are not prime
    if (n <= 3) return 1; // 2 and 3 are prime
		if (n==4) return 0 ;
    for (i = 0; i < security_level; i++) {
        unsigned int a = (rand() % (n - 4)) + 2;  // Random number in range [2, n-2]
        if (ModularExponentiation(a, n - 1, n) != 1) {
            return 0;  // Not prime
        }
    }
    return 1;  // Likely prime
}

// Modular inverse using Extended Euclidean Algorithm
u32 CalculateModularInverse(u32 e, u32 phi) 
	{
    int t, new_t;
    int r, new_r;
    int quotient;
	  int temp;
    
    r = phi;
    new_r = e;
    t = 0;
    new_t = 1;

    while (new_r != 0) 
			{
        quotient = r / new_r;

        // Update r and t
        r = r - quotient * new_r;
        t = t - quotient * new_t;

        // Swap
       temp = r;
        r = new_r;
        new_r = temp;

        temp = t;
        t = new_t;
        new_t = temp;
    }

    // Make sure t is positive
    if (r > 1) {
        return -1;  // No inverse exists
    }
    while (t < 0) {
        t = t + phi;
    }

    return t;
	}


















/*
#include <reg51.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT_LENGTH 5
#define SECURITY_LEVEL 5

typedef unsigned char            u8;
typedef unsigned short int      u16;
typedef unsigned long  int      u32;

void M_void_UART_Init();
void M_void_UART_SendChar(u8 c);
u8 M_U8_UART_ReceiveChar();
void M_void_UART_SendString(u8* str);
u32 M_U32_UART_ReceiveLong();
u32 mod_exp(u32 base, u16 exp, u32 mod);
u32 gcd(u32 a, u32 b);
u32 generate_e(u32 phi);
u32 is_prime(u32 n, u8 security_level);
u32 mod_inverse(u32 e, u32 phi);
void CRT_decrypt(u32 ciphertext, u16 p, u16 q, u32 d);
u16 p, q;
u32 n, phi, d, e;
void main(void) {
   	unsigned char buffer[25];
		unsigned int plaintext, ciphertext;
    unsigned char choice;
    M_void_UART_Init();
    //M_void_UART_SendString("RSA Encryption/Decryption System\r\n\r\n");
    // Prompt user for operation choice
    while (1) {
        //M_void_UART_SendString("Choose operation:\r\n1. Encrypt\r\n2. Decrypt\r\nEnter choice: ");
         M_void_UART_SendString("1. Encrypt\r\n2. Decrypt\r\nEnter choice: ");
			  choice = M_U8_UART_ReceiveChar();
        M_void_UART_SendString("\r\n");
        if (choice == '1' || choice == '2') break;
        //M_void_UART_SendString("Invalid choice. Please enter 1 or 2.\r\n\r\n");
    }
    // Input p (ensure it's prime)
    while (1) {
        M_void_UART_SendString("Enter prime number p: ");
        p = M_U32_UART_ReceiveLong();
        //M_void_UART_SendString("\r\n");
        if (is_prime(p, SECURITY_LEVEL)) {
            M_void_UART_SendString("\r\n p is a valid prime number.\r\n");
            break;
        } else {
            M_void_UART_SendString("Error: p is not a prime number. Please try again.\r\n\r\n");
        }
    }
    // Input q (ensure it's prime)
    while (1) {
        M_void_UART_SendString("Enter prime number q: ");
        q = M_U32_UART_ReceiveLong();
        //M_void_UART_SendString("\r\n");
        if (is_prime(q, SECURITY_LEVEL)) {
            M_void_UART_SendString("\r\n q is a valid prime number.\r\n");
            break;
        } else {
            M_void_UART_SendString("Error: q is not a prime number.\r\n");
        }
    }
    // Calculate n and phi
    n = p * q;
    phi = (p - 1) * (q - 1);

    // Generate public key e
    while (1) {
        M_void_UART_SendString("Enter public key value e: ");
        e = M_U32_UART_ReceiveLong();
        //M_void_UART_SendString("\r\n");
        if (gcd(e, phi) == 1) {
            M_void_UART_SendString("e is a valid public key.\r\n\r\n");
            break;
        }
    }
    // Calculate private key d
    d = mod_inverse(e, phi);

    // Display keys
    //sprintf(buffer, "Public Key: (e = %lu, n = %lu)\r\n", e, n);
    //M_void_UART_SendString(buffer);
    sprintf(buffer, "(d = %lu, n = %lu)\r\n", d, n);
    M_void_UART_SendString(buffer);

    // Perform selected operation
    if (choice == '1') {  // Encryption
        M_void_UART_SendString("Enter plaintext to encrypt (as integer): ");
        plaintext = M_U32_UART_ReceiveLong();
       // M_void_UART_SendString("\r\n");
        ciphertext = mod_exp(plaintext, e, n);
        sprintf(buffer, "Ciphertext: %i\r\n", ciphertext);
        M_void_UART_SendString(buffer);
    } else if (choice == '2') {  // Decryption
        M_void_UART_SendString("Enter ciphertext to decrypt: ");
        ciphertext = M_U32_UART_ReceiveLong();
        //M_void_UART_SendString("\r\n");
        CRT_decrypt(ciphertext, p, q, d);
    }

    while (1);
}

// UART initialization
void M_void_UART_Init() {
    TMOD = 0x20;  // Timer1 in Mode2 (8-bit auto-reload)
    TH1 = 0xFD;   // Timer1 value for 9600 baud with 11.0592 MHz crystal
    SCON = 0x50;  // 8-bit UART mode, REN enabled
    TR1 = 1;      // Start Timer1
}

void M_void_UART_SendChar(char c) {
    SBUF = c;
    while (TI == 0);
    TI = 0;
}

u8 M_U8_UART_ReceiveChar() {
    while (RI == 0);
    RI = 0;
    return SBUF;
}

void M_void_UART_SendString(u8* str) {
    while (*str) {
        M_void_UART_SendChar(*str++);
    }
}
u32 M_U32_UART_ReceiveLong() {
    u32 value = 0;
    u8 i;
    u8 c;

    for (i = 0; i < MAX_INPUT_LENGTH; i++) {
        c = M_U8_UART_ReceiveChar();
        if (c == '\r' || c == '\n') break;  // End of input
        value = value * 10 + (c - '0');
    }
    return value;
}

// Modular exponentiation (square and multiply)
u32 mod_exp(u32 base, u16 exp, u32 mod) {
    u32 result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result % mod ;
}

// GCD calculation
u32 gcd(u32 a, u32 b) {
    while (b != 0) {
        u32 temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Primality test using Fermat's method
u32 is_prime(u32 n, u8 security_level) {
	  u8 i;
    if (n <= 1) return 0;  // 0 and 1 are not prime
    if (n <= 3) return 1;  // 2 and 3 are prime
    for (i = 0; i < security_level; i++) {
        u32 a = (rand() % (n - 4)) + 2;  // Random number in range [2, n-1]
        if (mod_exp(a, n - 1, n) != 1) {
            return 0;  // Not prime
        }
    }
    return 1;  // Likely prime
}

// Modular inverse using Extended Euclidean Algorithm
u32 mod_inverse(u32 e, u32 phi) {
    int t = 0, new_t = 1;
    int r = phi, new_r = e;

    while (new_r != 0) {
        u32 quotient = r / new_r;

        // Update r and t
        int temp = new_r;
        new_r = r - quotient * new_r;
        r = temp;

        temp = new_t;
        new_t = t - quotient * new_t;
        t = temp;
    }

    if (r > 1) return -1;  // No inverse exists
    if (t < 0) t = t + phi;  // Make sure t is positive

    return t;
}

// Decrypt using CRT
void CRT_decrypt(u32 ciphertext, u16 p, u16 q, u32 d) {
		char buffer[20];
		u32 plaintext;

    u16 dp = d % (p - 1);
    u16 dq = d % (q - 1);
    u16 q_inv = mod_inverse(q, p);  // q_inv = q^(-1) mod p
		u16 p_inv = mod_inverse(p, q);
	
    u16 m1 = mod_exp(ciphertext, dp, p);
    u16 m2 = mod_exp(ciphertext, dq, q);

    // Combine using CRT
		plaintext=((m2*p_inv*p)+(m1*q_inv*q))%(p*q);
	/*
    u32 h = (q_inv * (m1 - m2 + p)) % p;  // h = (m1 - m2) * q^(-1) mod p
    plaintext = m2 + h * q;

    sprintf(buffer, "Decrypted value: %lu\r\n", plaintext);
    M_void_UART_SendString(buffer);
}
*/



/*
#include <reg51.h>
#include <stdio.h>
#include <stdlib.h>

// Configuration defines
#define MAX_INPUT_LENGTH 5  // Maximum input length for numbers
#define SECURITY_LEVEL 5    // Security level for primality test
#define ASCII_PRINTABLE_MIN 32  // Minimum printable ASCII value
#define ASCII_PRINTABLE_MAX 126 // Maximum printable ASCII value

// Bit manipulation macros
#define READ_BIT(REG,POS) ((REG>>POS)&1)
#define SET_BIT(REG,POS)  (REG|=(1<<POS))
#define CLEAR_BIT(REG,POS) (REG=~(1<<POS))

// Typedefs
typedef unsigned char           u8;
typedef unsigned short int      u16;
typedef unsigned long int       u32;

typedef signed int              s32;

// UART function prototypes
void M_void_UART_Init();
void M_void_UART_SendChar(u8 c);
u8 M_U8_UART_ReceiveChar();
void M_void_UART_SendString(u8* str);
u32 M_U32_UART_ReceiveLong();
// void UART_ReceiveString(char* buffer, int length);

// Math function prototypes
u32 mod_exp(u32 base, u32 exp, u32 mod);
u32 gcd(u32 a, u32 b);
u32 generate_e(u32 phi);
u32 is_prime(u32 n, u8 security_level);
u32 mod_inverse(u32 e, u32 phi);

// Global variables
u16 p, q;
u32 n, phi, d , e ;

void main(void) {
	
   unsigned int choice;
   unsigned int plaintext, ciphertext, decrypted_value;
   unsigned char buffer[40];
   //unsigned char Buf[5];

    // Initialize UART
    M_void_UART_Init();
    M_void_UART_SendString("RSA Encryption/Decryption System\r\n\r\n");

    // Prompt user for operation choice
    while (1) {
        M_void_UART_SendString("Choose operation:\r\n1. Encrypt\r\n2. Decrypt\r\nEnter choice: ");
        choice = M_U32_UART_ReceiveLong();
        M_void_UART_SendString("\r\n");
        if (choice == 1 || choice == 2) break;
        M_void_UART_SendString("Invalid choice. Please enter 1 or 2.\r\n\r\n");
    }

    // Input p (ensure it's prime)
    while (1) {
        M_void_UART_SendString("Enter prime number p: ");
        p = M_U32_UART_ReceiveLong();
        M_void_UART_SendString("\r\n");

        if (is_prime(p, SECURITY_LEVEL)) {
            M_void_UART_SendString("p is a valid prime number.\r\n\r\n");
            break;
        } else {
            M_void_UART_SendString("Error: p is not a prime number. Please try again.\r\n\r\n");
        }
    }

    // Input q (ensure it's prime)
    while (1) {
        M_void_UART_SendString("Enter prime number q: ");
        q = M_U32_UART_ReceiveLong();
        M_void_UART_SendString("\r\n");

        if (is_prime(q, SECURITY_LEVEL)) {
            M_void_UART_SendString("q is a valid prime number.\r\n\r\n");
            break;
        } else {
            M_void_UART_SendString("Error: q is not a prime number. Please try again.\r\n\r\n");
        }
    }

    // Calculate n and phi
    n = p * q;
    phi = (p - 1) * (q - 1);

    // Generate public key e
     while (1) {
        M_void_UART_SendString("Enter public key value e: ");
        e = M_U32_UART_ReceiveLong();
        M_void_UART_SendString("\r\n");

        if (gcd(e, phi) == 1) {
            M_void_UART_SendString("e is a valid public key.\r\n\r\n");
            break;
        } 
    }

    // Calculate private key d
    d = mod_inverse(e, phi);

    // Display keys
    sprintf(buffer, "Public Key: (e = %lu, n = %lu)\r\n", e, n);
    M_void_UART_SendString(buffer);
    sprintf(buffer, "Private Key: (d = %lu, phi(n) = %lu)\r\n\r\n", d, phi);
    M_void_UART_SendString(buffer);

    // Perform selected operation
    if (choice == 1) {  // Encryption
        M_void_UART_SendString("Enter plaintext to encrypt (as integer): ");
        //UART_ReceiveString(Buf, 5);
        //plaintext = atoi(Buf);
				plaintext=M_U32_UART_ReceiveLong();
        M_void_UART_SendString("\r\n");

        // Encrypt plaintext
        ciphertext = mod_exp(plaintext, e, n);
        sprintf(buffer, "Ciphertext: %d\r\n", ciphertext);
        M_void_UART_SendString(buffer);
    } else if (choice == 2) {  // Decryption
        M_void_UART_SendString("Enter ciphertext to decrypt: ");
        //UART_ReceiveString(Buf, 5);
        //ciphertext = atoi(Buf);
				ciphertext=M_U32_UART_ReceiveLong();
        M_void_UART_SendString("\r\n");

        // Decrypt ciphertext
        decrypted_value = mod_exp(ciphertext, d, n);
        sprintf(buffer, "Decrypted value: %d\r\n", decrypted_value);
        M_void_UART_SendString(buffer);

       
    }

 while(1);
    
}

// UART initialization
void M_void_UART_Init() {
    TMOD = 0x20;  // Timer1 in Mode2 (8-bit auto-reload)
    TH1 = 0xFD;   // Timer1 value for 9600 baud with 11.0592 MHz crystal
    SCON = 0x50;  // 8-bit UART mode, REN enabled
    TR1 = 1;      // Start Timer1
}

void M_void_UART_SendChar(char c) {
    SBUF = c;
    while (TI == 0);
    TI = 0;
}

u8 M_U8_UART_ReceiveChar() {
    while (RI == 0);
    RI = 0;
    return SBUF;
}

void M_void_UART_SendString(u8* str) {
    while (*str) {
        M_void_UART_SendChar(*str++);
    }
}

u32 M_U32_UART_ReceiveLong() {
    u32 value = 0;
    u8 i;
    u8 c;

    for (i = 0; i < MAX_INPUT_LENGTH; i++) {
        c = M_U8_UART_ReceiveChar();
        if (c == '\r' || c == '\n') break;  // End of input
        value = value * 10 + (c - '0');
    }
    return value;
}
*/
/*
void UART_ReceiveString(unsigned char* buffer, unsigned int length) {
   unsigned int i = 0;
   unsigned char ch;
    while (i < length) {
        ch = M_U8_UART_ReceiveChar();
        if (ch == '\n' || ch == '\r') {
            break;
        }
        buffer[i++] = ch;
    }
}
*/
// Modular exponentiation (square and multiply)
/*
u32 mod_exp(u32 base, u32 exp, u32 mod) {
    u16 result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = ((result * base) % mod);
        }
        exp = exp >> 1;
        base = ((base * base) % mod);
    }
    return result;
}

// GCD calculation
u32 gcd(u32 a, u32 b) {
    while (b != 0) {
       unsigned int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}


// Primality test using Fermat's method
u32 is_prime(u32 n, u8 security_level) {
    u8 i;
    if (n <= 1) return 0; // 0 and 1 are not prime
    if (n <= 3) return 1; // 2 and 3 are prime
    for (i = 0; i < security_level; i++) {
        unsigned int a = (rand() % (n - 4)) + 2;  // Random number in range [2, n-1]
        if (mod_exp(a, n - 1, n) != 1) {
            return 0;  // Not prime
        }
    }
    return 1;  // Likely prime
}

// Modular inverse using Extended Euclidean Algorithm
u32 mod_inverse(u32 e, u32 phi) 
	{
    int t, new_t;
    int r, new_r;
    int quotient;
	  int temp;
    
    r = phi;
    new_r = e;
    t = 0;
    new_t = 1;

    while (new_r != 0) 
			{
        quotient = r / new_r;

        // Update r and t
        r = r - quotient * new_r;
        t = t - quotient * new_t;

        // Swap
       temp = r;
        r = new_r;
        new_r = temp;

        temp = t;
        t = new_t;
        new_t = temp;
    }

    // Make sure t is positive
    if (r > 1) {
        return -1;  // No inverse exists
    }
    if (t < 0) {
        t = t + phi;
    }

    return t;
	}


*/


/*
#include <reg51.h>
#include <stdio.h>
#include <stdlib.h>

#define SECURITY_LEVEL 5
#define MAX_HEX_LENGTH 5

typedef unsigned char u8;
typedef unsigned long u32;

// Function prototypes
void M_void_UART_Init();
void M_void_UART_SendChar(u8 c);
void M_void_UART_SendString(u8* str);
void M_U32_UART_ReceiveHex(u32* value);
u32 mod_exp(u32 base, u32 exp, u32 mod);
u32 gcd(u32 a, u32 b);
u32 is_prime(u32 n, u8 security_level);
u32 generate_e(u32 phi);
u32 mod_inverse(u32 e, u32 phi);

u32 p, q, n, phi, e, d;
char s ;
void main(void) {
    u8 choice;
    u32 plaintext, ciphertext;
		char buffer[10];

    M_void_UART_Init();
    M_void_UART_SendString("RSA Implementation\r\n");

    // Input p and q (prime numbers)
    while (1) {
        M_void_UART_SendString("Enter prime number p (4-digit hex): ");
        M_U32_UART_ReceiveHex(&p);
        M_void_UART_SendString("\r\n");
        
        M_void_UART_SendString("Enter prime number q (4-digit hex): ");
        M_U32_UART_ReceiveHex(&q);
        M_void_UART_SendString("\r\n");

        // Validate p and q
        if (is_prime(p, SECURITY_LEVEL) && is_prime(q, SECURITY_LEVEL)) {
            M_void_UART_SendString("Both p and q are prime.\r\n");
            break;
        } else {
            M_void_UART_SendString("Error: p or q is not prime. Please re-enter.\r\n");
        }
    }

    // Calculate n and phi
    n = p * q;
    phi = (p - 1) * (q - 1);

    // Generate e
    e = generate_e(phi);
    if (e == 0) {
        M_void_UART_SendString("Error: Could not generate e.\r\n");
        while (1);
    }

    // Calculate d
    d = mod_inverse(e, phi);
    if (d == -1) {
        M_void_UART_SendString("Error: Could not compute modular inverse for e.\r\n");
        while (1);
    }

    // Display keys
    
    sprintf(buffer, "Public Key: (e = %lu, n = %lu)\r\n", e, n);
    M_void_UART_SendString(buffer);
    sprintf(buffer, "Private Key: (d = %lu, n = %lu)\r\n", d, n);
    M_void_UART_SendString(buffer);

    // Main loop for encrypt/decrypt choice
    while (1) {
        M_void_UART_SendString("Choose operation: 1 for Encrypt, 2 for Decrypt: ");
        choice = M_U8_UART_ReceiveChar();
        M_void_UART_SendString("\r\n");

        if (choice == '1') {
            // Encryption
            M_void_UART_SendString("Enter plaintext (4-digit hex): ");
            M_U32_UART_ReceiveHex(&plaintext);
            M_void_UART_SendString("\r\n");

            // Encrypt the plaintext
            ciphertext = mod_exp(plaintext, e, n);
            sprintf(buffer, "Ciphertext: %lu\r\n", ciphertext);
            M_void_UART_SendString(buffer);

        } else if (choice == '2') {
            // Decryption
            M_void_UART_SendString("Enter ciphertext (4-digit hex): ");
            M_U32_UART_ReceiveHex(&ciphertext);
            M_void_UART_SendString("\r\n");

            // Decrypt the ciphertext
            plaintext = mod_exp(ciphertext, d, n);
            sprintf(buffer, "Decrypted value: %lu\r\n", plaintext);
            M_void_UART_SendString(buffer);
        }
    }
}

// UART initialization
void M_void_UART_Init() {
    TMOD = 0x20;  // Timer1 in Mode 2 (8-bit auto-reload)
    TH1 = 0xFD;   // Timer1 value for 9600 baud with 11.0592 MHz crystal
    SCON = 0x50;  // 8-bit UART mode, REN enabled
    TR1 = 1;      // Start Timer1
}

void M_void_UART_SendChar(char c) {
    SBUF = c;
    while (TI == 0);
    TI = 0;
}

void M_void_UART_SendString(u8* str) {
    while (*str) {
        M_void_UART_SendChar(*str++);
    }
}

void M_U32_UART_ReceiveHex(u32* value) {
    *value = 0;
		 
    for(  s = 0 ; s < 4 ; s++ )
	{
        char c = M_U8_UART_ReceiveChar();
        if (c >= '0' && c <= '9') {
            *value = (*value << 4) | (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            *value = (*value << 4) | (c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            *value = (*value << 4) | (c - 'a' + 10);
        }
    }
}

u32 mod_exp(u32 base, u32 exp, u32 mod) {
    u32 result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;
}

u32 gcd(u32 a, u32 b) {
    while (b != 0) {
        u32 temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

u32 is_prime(u32 n, u8 security_level) {
		
    if (n <= 1) return 0; // 0 and 1 are not prime
    if (n <= 3) return 1; // 2 and 3 are prime
    for (s = 0; s < security_level; s++) {
        u32 a = (rand() % (n - 4)) + 2;  // Random number in range [2, n-1]
        if (mod_exp(a, n - 1, n) != 1) {
            return 0;  // Not prime
        }
    }
    return 1;  // Likely prime
}

u32 generate_e(u32 phi) {
	 u32 e;
    for (e = 3; e < phi; e += 2) {
        if (gcd(e, phi) == 1) {
            return e;  // Found a suitable e
        }
    }
    return 0; // In case no suitable e is found
}

u32 mod_inverse(u32 e, u32 phi) {
    int t = 0, new_t = 1;
    int r = phi, new_r = e;

    while (new_r != 0) {
        u32 quotient = r / new_r;

        // Update r and t
        int temp = new_r;
        new_r = r - quotient * new_r;
        r = temp;

        temp = new_t;
        new_t = t - quotient * new_t;
        t = temp;
    }

    if (r > 1) return -1;  // No inverse exists
    if (t < 0) t += phi;   // Make sure t is positive

    return t;
}

*/

/*
#include <reg51.h>
#include <stdio.h>

#define SECURITY_LEVEL 5

typedef unsigned char u8;
typedef unsigned long u32;

void M_void_UART_Init();
void M_void_UART_SendChar(u8 c);
void M_void_UART_SendString(u8* str);
void M_U32_UART_ReceiveHex(u32* value);
u32 mod_exp(u32 base, u32 exp, u32 mod);
u32 gcd(u32 a, u32 b);
u8 is_prime(u32 n);
u32 generate_e(u32 phi);
u32 mod_inverse(u32 e, u32 phi);

u32 p, q, n, phi, e, d;
char buffer[10];
void main(void) {
    u8 choice;
    u32 input;

    M_void_UART_Init();
    M_void_UART_SendString("RSA Implementation\r\n");

    // Input p and q (prime numbers)
    while (1) {
        M_void_UART_SendString("Enter prime number p (4-digit hex): ");
        M_U32_UART_ReceiveHex(&p);
        M_void_UART_SendString("\r\n");

        M_void_UART_SendString("Enter prime number q (4-digit hex): ");
        M_U32_UART_ReceiveHex(&q);
        M_void_UART_SendString("\r\n");

        if (is_prime(p) && is_prime(q)) {
            M_void_UART_SendString("Both p and q are prime.\r\n");
            break;
        } else {
            M_void_UART_SendString("Error: p or q is not prime. Please re-enter.\r\n");
        }
    }

    n = p * q;
    phi = (p - 1) * (q - 1);
    e = generate_e(phi);
    d = mod_inverse(e, phi);

    // Main loop for encrypt/decrypt choice
    while (1) {
        M_void_UART_SendString("Choose operation: 1 for Encrypt, 2 for Decrypt: ");
        choice = M_U8_UART_ReceiveChar();
        M_void_UART_SendString("\r\n");

        if (choice == '1') {
            M_void_UART_SendString("Enter plaintext (4-digit hex): ");
            M_U32_UART_ReceiveHex(&input);
            M_void_UART_SendString("\r\n");
            u32 ciphertext = mod_exp(input, e, n);
            sprintf(buffer, "Ciphertext: %lu\r\n", ciphertext);
            M_void_UART_SendString(buffer);
        } else if (choice == '2') {
            M_void_UART_SendString("Enter ciphertext (4-digit hex): ");
            M_U32_UART_ReceiveHex(&input);
            M_void_UART_SendString("\r\n");
            u32 plaintext = mod_exp(input, d, n);
            sprintf(buffer, "Decrypted value: %lu\r\n", plaintext);
            M_void_UART_SendString(buffer);
        }
    }
}

void M_void_UART_Init() {
    TMOD = 0x20;  // Timer1 in Mode 2 (8-bit auto-reload)
    TH1 = 0xFD;   // Timer1 value for 9600 baud
    SCON = 0x50;  // 8-bit UART mode, REN enabled
    TR1 = 1;      // Start Timer1
}

void M_void_UART_SendChar(char c) {
    SBUF = c;
    while (TI == 0);
    TI = 0;
}

void M_void_UART_SendString(u8* str) {
    while (*str) {
        M_void_UART_SendChar(*str++);
    }
}

void M_U32_UART_ReceiveHex(u32* value) {
	u8 i;
    *value = 0;
    for ( i = 0; i < 4; i++) {
        char c = M_U8_UART_ReceiveChar();
        if (c >= '0' && c <= '9') {
            *value = (*value << 4) | (c - '0');
        } else if (c >= 'A' && c <= 'F') {
            *value = (*value << 4) | (c - 'A' + 10);
        } else if (c >= 'a' && c <= 'f') {
            *value = (*value << 4) | (c - 'a' + 10);
        }
    }
}

u32 mod_exp(u32 base, u32 exp, u32 mod) {
    u32 result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp >>= 1;
        base = (base * base) % mod;
    }
    return result;
}

u32 gcd(u32 a, u32 b) {
    while (b) {
        u32 temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

u8 is_prime(u32 n) {
	u32 i;
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    for ( i = 2; i * i <= n; i++)
        if (n % i == 0) return 0;
    return 1;
}

u32 generate_e(u32 phi) {
	u32 e;
    for ( e= 3; e < phi; e += 2) {
        if (gcd(e, phi) == 1) return e;
    }
    return 0; // In case no suitable e is found
}

u32 mod_inverse(u32 e, u32 phi) {
    int t = 0, new_t = 1;
    int r = phi, new_r = e;
    while (new_r != 0) {
        u32 quotient = r / new_r;
        int temp = new_r;
        new_r = r - quotient * new_r;
        r = temp;
        temp = new_t;
        new_t = t - quotient * new_t;
        t = temp;
    }
    if (r > 1) return -1;  // No inverse exists
    if (t < 0) t += phi;   // Make sure t is positive
    return t;
}

*/