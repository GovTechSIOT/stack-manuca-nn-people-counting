/**
 * @defgroup crypto_engine_v2 Crypto Engine V2
 * @{
 */

#include "crypto_engine_v2.h"
#include <regex>
#include <sstream> 
#include <vector>
#include "mbed.h"
#include "mbedtls/config.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/platform.h"
#include "mbedtls/rsa.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ssl.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_csr.h"
#include "mbed_trace.h"
#include "global_params.h"
#include "persist_store.h"

#undef TRACE_GROUP
#define TRACE_GROUP  "CryptoEngineV2"

#define MBEDTLS_KEY_SIZE 					(2048)
#define MBEDTLS_EXPONENT 					(65537)

static mbedtls_pk_context 				    mbedtls_private_key_context;
static mbedtls_entropy_context 		        mbedtls_entropy;
static mbedtls_ctr_drbg_context 	        mbedtls_ctrdrbg;
static mbedtls_mpi 						    mbedtls_n;
static mbedtls_mpi 							mbedtls_p;
static mbedtls_mpi 							mbedtls_q;
static mbedtls_mpi 							mbedtls_d;
static mbedtls_mpi 							mbedtls_e;
static mbedtls_mpi 							mbedtls_dp;
static mbedtls_mpi 							mbedtls_dq;
static mbedtls_mpi 							mbedtls_qp;
static const char*							mbedtls_pers = "gen_key";

static mbedtls_x509write_csr 			    mbedtls_csr_request;
static char								    mbedtls_private_key[1024];
static unsigned char 						mbedtls_csr_pem[4096];

/**
 *  @brief  Generate PKI keypair for CSR generation.
 *  @author Goh Kok Boon
 *  @return Return Success/Fail code
 */
int CryptoEngineV2::GenerateRSAKeypair(void)
{
	int rc;
	mbedtls_ctr_drbg_init(&mbedtls_ctrdrbg);
	mbedtls_pk_init(&mbedtls_private_key_context);
	memset(mbedtls_private_key, 0, sizeof(mbedtls_private_key));
	mbedtls_mpi_init(&mbedtls_n);
	mbedtls_mpi_init(&mbedtls_p);
	mbedtls_mpi_init(&mbedtls_q);
	mbedtls_mpi_init(&mbedtls_d);
	mbedtls_mpi_init(&mbedtls_e);
	mbedtls_mpi_init(&mbedtls_dp);
	mbedtls_mpi_init(&mbedtls_dq);
	mbedtls_mpi_init(&mbedtls_qp);

	/* Seeding Random Number */
	mbedtls_entropy_init(&mbedtls_entropy);
	rc = mbedtls_ctr_drbg_seed(&mbedtls_ctrdrbg, mbedtls_entropy_func, &mbedtls_entropy, (const unsigned char *)mbedtls_pers, strlen(mbedtls_pers));
	if (rc == 0)
	{
        /* Generating PKI - Private Key */
		rc = mbedtls_pk_setup(&mbedtls_private_key_context, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA));
		if (rc == 0)
		{
			rc = mbedtls_rsa_gen_key(mbedtls_pk_rsa(mbedtls_private_key_context), mbedtls_ctr_drbg_random, &mbedtls_ctrdrbg, MBEDTLS_KEY_SIZE, MBEDTLS_EXPONENT);
			if (rc == 0)
			{
                unsigned char output_buf[16000];
                mbedtls_pk_write_key_pem(&mbedtls_private_key_context, output_buf, 16000);
                std::string pk = (const char*) output_buf;
                WriteSSLPrivateKey(pk);
				tr_info("Private Key Generation Success");
				return 1;
			}
			else
			{
				tr_warn(" FAILED - mbedtls_rsa_gen_key returned -0x%04x\r\n", -rc);
			}
		}
		else
		{
			tr_warn("FAILED - mbedtls_pk_setup returned -0x%04x\r\n", -rc);
		}
	}
	else
	{
		tr_warn("mbedtls_ctr_drbg_seed returned %d - FAILED\r\n", rc);
	}

	/* Free Resources */
	mbedtls_mpi_free(&mbedtls_n);
	mbedtls_mpi_free(&mbedtls_p);
	mbedtls_mpi_free(&mbedtls_q);
	mbedtls_mpi_free(&mbedtls_d);
	mbedtls_mpi_free(&mbedtls_e);
	mbedtls_mpi_free(&mbedtls_dp);
	mbedtls_mpi_free(&mbedtls_dq);
	mbedtls_mpi_free(&mbedtls_qp);
	mbedtls_ctr_drbg_free(&mbedtls_ctrdrbg);
	mbedtls_entropy_free(&mbedtls_entropy);

	return 0;
}

/**
 *  @brief  Generate CSR for retrieving client certificate from decada.
 *  @author Goh Kok Boon, Lau Lee Hong
 *  @param  timestamp   Current unix epoch timestamp (in miliseconds)
 *  @return Return PEM-formatted CSR
 */
std::string CryptoEngineV2::GenerateCertificateSigningRequest(std::string timestamp)
{
    std::string csr_info = "C=SG, ST=Singapore, L=Singapore, O=DECADA, OU=DECADA CA, CN=" + device_uuid + timestamp;
	const char* mbedtls_subject_name = csr_info.c_str(); 

	if (GenerateRSAKeypair() == 1)
	{
		mbedtls_x509write_csr_init(&mbedtls_csr_request);
		mbedtls_x509write_csr_set_md_alg(&mbedtls_csr_request, MBEDTLS_MD_SHA256);
		mbedtls_ctr_drbg_init(&mbedtls_ctrdrbg);
		memset(mbedtls_private_key, 0, sizeof(mbedtls_private_key));

        /* Seeding Random Number */
		mbedtls_entropy_init(&mbedtls_entropy);
		int rc = mbedtls_ctr_drbg_seed(&mbedtls_ctrdrbg, mbedtls_entropy_func, &mbedtls_entropy, (const unsigned char *)mbedtls_pers, strlen(mbedtls_pers));

		if (rc == 0)
		{
			/* Check the subject name for validity */
			rc = mbedtls_x509write_csr_set_subject_name(&mbedtls_csr_request, mbedtls_subject_name);
			if(rc == 0)
			{
				mbedtls_x509write_csr_set_key(&mbedtls_csr_request, &mbedtls_private_key_context);

				/* Covert CSR in PEM format */
				memset(mbedtls_csr_pem, 0, sizeof(mbedtls_csr_pem));
				rc = mbedtls_x509write_csr_pem(&mbedtls_csr_request, mbedtls_csr_pem, sizeof(mbedtls_csr_pem), mbedtls_ctr_drbg_random, &mbedtls_ctrdrbg);

				if (rc >= 0)
				{
					tr_info("CSR PEM Generation Successful");
					mbedtls_x509write_csr_free(&mbedtls_csr_request);
					mbedtls_pk_free(&mbedtls_private_key_context);
					mbedtls_ctr_drbg_free(&mbedtls_ctrdrbg);
					mbedtls_entropy_free(&mbedtls_entropy);
					return (char*)mbedtls_csr_pem;
				}
				else
				{
					tr_warn("mbedtls_x509write_csr_pem returned %d - FAILED\r\n", rc);
				}
			}
			else
			{
				tr_warn("mbedtls_x509write_csr_set_subject_name returned %d - FAILED\r\n", rc);
			}
		}
		else
		{
			tr_warn("mbedtls_ctr_drbg_seed returned %d - FAILED\r\n", rc);
		}

		mbedtls_x509write_csr_free(&mbedtls_csr_request);
		mbedtls_pk_free(&mbedtls_private_key_context);
		mbedtls_ctr_drbg_free(&mbedtls_ctrdrbg);
		mbedtls_entropy_free(&mbedtls_entropy);
	}
    return "";
}

/**
 *  @brief  Format SSL CA Certificate attained via HTTPS to PEM.
 *  @author Lau Lee Hong
 *  @param  s   Unformatted SSL CA Certificate
 *  @return Return PEM-formatted SSL-CA Certificate
 */
std::string CryptoEngineV2::CertificateAuthorityPEMFormatter(std::string s)
{
    std::string ssl_ca = s;
    std::string ssl_ca_header = "-----BEGIN CERTIFICATE-----";
    std::string ssl_ca_footer = "-----END CERTIFICATE-----";
    
    std::regex r_header(ssl_ca_header);
    std::regex r_footer(ssl_ca_footer);
    std::regex r_whitespace(" ");
    
    /* Remove header and footer to attain certificate body*/
    ssl_ca = std::regex_replace(ssl_ca, r_header, "");
    ssl_ca = std::regex_replace(ssl_ca, r_footer, "");
    
    /* Replace whitespaces in the body with '\n' */
    ssl_ca = std::regex_replace(ssl_ca, r_whitespace, "\n");
    
    /* Rebuild our formatted SSL_CA_PEM */
    ssl_ca = ssl_ca_header + ssl_ca + ssl_ca_footer + "\n";
    
    return ssl_ca;
}

/**
 *  @brief  Extracts issuer's info from X509 CA Certificate into a buffer.
 *  @author Lau Lee Hong
 *  @param  buf     Output buffer to store result
 *  @param  size    Size of buffer
 *  @param  crt     Pointer to x509 certificate struct
 *  @return true (success) / false (failure)
 */
bool CryptoEngineV2::X509IssuerInfo(char* buf, size_t size, const mbedtls_x509_crt* crt)
{
    int ret;
    size_t n;
    char *p;
    
    p = buf;
    n = size;
    
    /* Safety check for uninitialized x509 struct */
    if(NULL == crt)
    {
        ret = mbedtls_snprintf(p, n, "\nCertificate is uninitialised!\n");

        return false;
    }

    ret = mbedtls_x509_dn_gets(p, n, &crt->issuer);
    
    if (ret < 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 *  @brief  Decodes SSL CA certificate.
 *  @author Lau Lee Hong
 *  @param  ssl_ca  SSL CA Certificate
 *  @param  ca_params   Address of struct holding country_name, state_name and org_name
 *  @return true (success) / false (failure)
 */
bool CryptoEngineV2::X509CertificateAuthorityDecoder(std::string ssl_ca, ssl_ca_params &ca_params)
{    
    std::string ca_country_name, ca_state_name, ca_org_name;

    int n = ssl_ca.length();
    char ssl_ca_buf[n+1];
    strcpy(ssl_ca_buf, ssl_ca.c_str());
    
    const uint32_t buf_size = 512;
    char *buf = new char[buf_size];
    
    mbedtls_x509_crt x509_root_ca;
    mbedtls_x509_crt_init(&x509_root_ca);
    mbedtls_x509_crt_parse(&x509_root_ca, (const unsigned char*) ssl_ca_buf, sizeof(ssl_ca_buf));
    
    bool rc = X509IssuerInfo(buf, buf_size, &x509_root_ca);
    mbedtls_x509_crt_free(&x509_root_ca);
    
    if (!rc)
    {
        tr_warn("X509 Root CA Parsing Error");
		return false;;
    }

    std::string issuer_info = buf;
    delete buf;
    tr_debug("%s", issuer_info.c_str());      // C=xxx, ST=xxx, L=xxx, O=xxx, OU=xxx, CN=xxx
    
    /* Tokenized issuer_info parameters */
    std::vector<std::string> tokens;
    std::stringstream ss(issuer_info);
    while (ss.good())
    {
        std::string substr;
        getline(ss, substr, ',');
        tokens.push_back(substr);
    }
    
    /* Extract C, ST, O parameters from multi-tokens*/
    ca_country_name = tokens.at(0);
    ca_country_name = ca_country_name.substr(ca_country_name.find("=") + 1);
    ca_params.country_name = ca_country_name;
    
    ca_state_name = tokens.at(1);
    ca_state_name = ca_state_name.substr(ca_state_name.find("=") + 1);
    ca_params.state_name = ca_state_name;
    
    ca_org_name = tokens.at(3);
    ca_org_name = ca_org_name.substr(ca_org_name.find("=") + 1);
    ca_params.org_name = ca_org_name;
    
    return true;
}

/**
 *  @brief  Generates SHA256 Signature.
 *  @author Goh Kok Boon
 *  @param  params  Content required to generate signature 
 *  @return C++ string containing 64-character hexadecimal representation of signature
 */
std::string CryptoEngineV2::SignatureGenerator(std::string params)
{
    std::string signing_params = MBED_CONF_APP_DECADA_ACCESS_KEY + params + MBED_CONF_APP_DECADA_ACCESS_SECRET;
    std::string converted = GenericSHA256Generator(signing_params); 

    return converted;
}

/**
 *  @brief  Generic SHA256 Signature Generator.
 *  @author Lau Lee Hong
 *  @param  input  Content required to generate signature 
 *  @return C++ string containing 64-character hexadecimal representation of signature
 */
std::string CryptoEngineV2::GenericSHA256Generator(std::string input)
{
    char signing[input.size()];
    strcpy(signing, input.c_str());

    unsigned char *signing_buffer = (unsigned char *) signing;

    const size_t buffer_len = strlen(signing);

    unsigned char output[32];
    mbedtls_sha256(signing_buffer, buffer_len, output, 0);

    char converted[32*2 + 1];
    for(int i=0;i<32;i++) 
    {
        snprintf(&converted[i*2], sizeof(converted)-(i*2), "%02X", output[i]);
    }
    converted[64] = '\0';

    return converted;
}

/**
 *  @brief  Generic SHA1 Signature Generator.
 *  @author Lau Lee Hong
 *  @param  input  Content required to generate signature 
 *  @return C++ string containing 40-character hexadecimal representation of signature
 */
std::string CryptoEngineV2::GenericSHA1Generator(std::string input)
{
    char signing[input.size()];
    strcpy(signing, input.c_str());

    unsigned char *signing_buffer = (unsigned char *) signing;

    const size_t buffer_len = strlen(signing);

    unsigned char output[20];
    mbedtls_sha1(signing_buffer, buffer_len, output);

    char converted[20*2 + 1];
    for(int i=0;i<20;i++) 
    {
        snprintf(&converted[i*2], sizeof(converted)-(i*2), "%02X", output[i]);
    }
    converted[40] = '\0';

    return converted;
}

 /** @}*/