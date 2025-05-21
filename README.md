# 🔐 RSA Encryption/Decryption on 8051 Microcontroller

This project implements **RSA Encryption and Decryption** on an **8051 microcontroller** using UART for communication with the user. It demonstrates how asymmetric cryptography can be applied on low-resource embedded systems.

---

## 🧠 How it Works

The system allows the user to:

1. Choose between **encryption** and **decryption**.
2. Input two prime numbers `p` and `q`.
3. Compute:
   - `n = p * q`
   - `phi = (p-1)*(q-1)`
4. Choose public key `e` (must satisfy `gcd(e, phi) == 1`).
5. Compute private key `d` using the Extended Euclidean Algorithm.
6. Perform:
   - **Encryption**: `ciphertext = plaintext^e mod n`
   - **Decryption**: `decrypted = ciphertext^d mod n`

All interactions are done via **UART terminal** (e.g., PuTTY or RealTerm).

---

## 💻 Technologies Used

- **C language**
- **8051 Microcontroller**
- **Keil C (or similar)**
- **UART serial communication**
- **Random-based primality check (Fermat Test)**

---

## 📂 Project Structure

- `main.c`: Full implementation of RSA logic, UART functions, modular math.
- All functions are defined in a single file for simplicity.

---

## 🔧 Usage

1. Flash the code to your 8051 microcontroller.
2. Open a UART terminal at **9600 baud**, 8-N-1.
3. Follow on-screen prompts to:
   - Enter prime numbers.
   - Enter public key.
   - Enter plaintext or ciphertext depending on the mode.
4. View results on terminal.

---

## ⚠️ Notes & Limitations

- Prime checking is done using Fermat’s primality test, which is **probabilistic**.
- Works best for **small primes** (e.g., 11, 13, 17, 19, ...).
- Not secure for real-world cryptographic use—intended for educational demonstration only.
- No support for strings or large files—this is **integer-based** RSA only.

---

## 📜 License

This project is developed for educational purposes only.
