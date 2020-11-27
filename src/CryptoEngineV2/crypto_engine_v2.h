/*******************************************************************************************************
 * Copyright (c) 2018-2020 Government Technology Agency of Singapore (GovTech)
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and limitations under the License.
 *******************************************************************************************************/
#ifndef CRYPTO_ENGINE_V2_H
#define CRYPTO_ENGINE_V2_H

#include <string>
#include "mbed.h"

/** CryptoEngineV2 class.
 *  @brief      Handles X509 Exchange-related cryptographic processes.
 *  @details    Use this object as a base-class to build your cloud client.
 *
 *  Example:
 *  @code{.cpp}
 *  #include "mbed.h"
 *  #include "crypto_engine_v2.h"
 *
 *  int main() 
 *  {
 *      CryptoEngineV2 crypto;
 *      int success = crypto.GenerateRSAKeypair();
 *      printf("\r\nRSA Keypair Generation Code: %d\r\n", success);
 *  }
 *  @endcode
 */
        
typedef struct {
    std::string country_name;
    std::string state_name;
    std::string org_name;
} ssl_ca_params;

class CryptoEngineV2 
{
    public:
        int GenerateRSAKeypair(void);
        std::string GenerateCertificateSigningRequest(std::string timestamp);
        std::string CertificateAuthorityPEMFormatter(std::string s);
        bool X509IssuerInfo (char* buf, size_t size, const mbedtls_x509_crt* crt);
        bool X509CertificateAuthorityDecoder (std::string ssl_ca, ssl_ca_params& ca_params);
        std::string SignatureGenerator(std::string params);
        std::string GenericSHA256Generator(std::string input);
        std::string GenericSHA1Generator(std::string input);
};

 #endif  // CRYPTO_ENGINE_V2_H