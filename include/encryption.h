#ifndef _ENCRYPTION_
#define _ENCRYPTION_
//utitlies function for encryption

#include <iostream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string>
#include <cstring>
#include <mutex> // to manage access to shared variable in multi thread 

#define KEY_SIZE 32 // Size of the derived key

using namespace std;

size_t generateRandomKey() //to generateRandomKey for encryption
{
    string public_key = "www.amnafzar.ir";
    hash<string> hash_obj;
    return hash_obj(public_key);
}


std::string deriveKey(const std::string &password) // deriveKey for decryption
{
    unsigned char key[KEY_SIZE];
    EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), NULL,
                   (const unsigned char *)password.c_str(), password.length(), 1, key, NULL);

    return std::string(reinterpret_cast<char *>(key), KEY_SIZE);
}

std::string encryptAES(const std::string &message, const std::string &password) //encrypt msg with password at server side
{
    std::string derivedKey = deriveKey(password);

    // Encryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (const unsigned char *)derivedKey.c_str(), NULL);

    int ciphertextLength = message.length() + EVP_CIPHER_block_size(EVP_aes_256_cbc());
    unsigned char *ciphertext = new unsigned char[ciphertextLength];

    int finalCiphertextLength = 0;
    EVP_EncryptUpdate(ctx, ciphertext, &ciphertextLength, (const unsigned char *)message.c_str(), message.length());
    EVP_EncryptFinal_ex(ctx, ciphertext + ciphertextLength, &finalCiphertextLength);
    ciphertextLength += finalCiphertextLength;

    std::string encryptedMessage(reinterpret_cast<char *>(ciphertext), ciphertextLength);

    EVP_CIPHER_CTX_free(ctx);
    delete[] ciphertext;

    return encryptedMessage;
}

std::string decryptAES(const std::string &encryptedMessage, const std::string &password) //decrypt msg with password at server side
{
    std::string derivedKey = deriveKey(password);

    // Decryption
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (const unsigned char *)derivedKey.c_str(), NULL);

    int ciphertextLength = encryptedMessage.length();
    unsigned char *ciphertext = new unsigned char[ciphertextLength];
    memcpy(ciphertext, encryptedMessage.c_str(), ciphertextLength);

    int plaintextLength = ciphertextLength;
    unsigned char *plaintext = new unsigned char[plaintextLength];

    int finalPlaintextLength = 0;
    EVP_DecryptUpdate(ctx, plaintext, &plaintextLength, ciphertext, ciphertextLength);
    EVP_DecryptFinal_ex(ctx, plaintext + plaintextLength, &finalPlaintextLength);
    plaintextLength += finalPlaintextLength;

    std::string decryptedMessage(reinterpret_cast<char *>(plaintext), plaintextLength);

    EVP_CIPHER_CTX_free(ctx);
    delete[] ciphertext;
    delete[] plaintext;

    return decryptedMessage;
}

#endif
