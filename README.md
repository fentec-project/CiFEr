# CiFEr - Functional Encryption library [![Build Status](https://circleci.com/gh/fentec-project/CiFEr.svg?style=svg)](https://circleci.com/gh/fentec-project/CiFEr) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/e086336e31854374a98c4554a111b8f4)](https://www.codacy.com/gh/fentec-project/CiFEr?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=fentec-project/CiFEr&amp;utm_campaign=Badge_Grade)

CiFEr (prounounced as _cipher_) is a cryptographic library offering different 
state-of-the-art implementations of functional encryption schemes, specifically 
FE schemes for _linear_ polynomials (e.g. _inner products_). It is implemented 
in C. A [Go version named GoFE](https://github.com/fentec-project/gofe) 
of the library also exists.

To quickly get familiar with FE, read a short and very high-level 
introduction on our [Introductory Wiki page](https://github.com/fentec-project/gofe/wiki/Introduction-to-FE).

The documentation for CiFEr is available on 
[GitHub Pages](https://fentec-project.github.io/CiFEr).

CiFEr is distributed under the
[Apache 2 license](https://www.apache.org/licenses/LICENSE-2.0). It uses
[GMP](https://gmplib.org/), which is distributed under the dual licenses,
[GNU LGPL v3](https://www.gnu.org/licenses/lgpl.html) and
[GNU GPL v2](https://www.gnu.org/licenses/gpl-2.0.html).

<!-- toc -->
- [Building CiFEr](#building-cifer)
    * [Requirements](#requirements)
    * [Build and install](#build-and-install)
    * [Test](#test)
- [Using CiFEr in your project](#using-cifer-in-your-project)
    * [Select the FE scheme](#select-the-fe-scheme)
    * [Configure selected scheme](#configure-selected-scheme)
    * [Prepare input data](#prepare-input-data)
    * [Use the scheme (examples)](#use-the-scheme-(examples))
<!-- tocstop -->

### Before using the library
Please note that the library is a work in progress and has not yet
reached a stable release. Code organization and APIs are **not stable**.
You can expect them to change at any point.

The purpose of CiFEr is to support research and proof-of-concept
implementations. It **should not be used in production**.

## Building CiFEr

### Requirements
The requirements have to be installed manually (via package manager or building 
the source code).
- [CMake](https://cmake.org/download/) (version 3.11+)
- [GMP](https://gmplib.org/)
- [libsodium](https://download.libsodium.org/doc/)
- [AMCL](https://github.com/miracl/amcl)
- [Protobuf](https://github.com/protocolbuffers/protobuf)

CiFEr relies on GMP for all big integer arithmetic. We recommend familiarizing 
yourself with it before using CiFEr.

To be able to build CiFEr as described below, AMCL must be compiled with BN254
curve. This can be done manually, but for convenience, we provide a Bash script
that runs a modified AMCL setup (a Python script) and installs a minimal version
 of AMCL in the standard directory `/usr/local/lib` and header files in
`/usr/local/include`. These default values can be changed in
`external/amcl/setup_amcl.sh`. To use the script, run:
```
cd external/amcl
sudo ./setup_amcl.sh
cd ../..
```

Alternatively, if you do not like to pollute `/usr/local/` with unmanaged
files, you can use a locally compiled AMCL through a submodule:
```
external/amcl/setup_local_amcl.sh
```
The above script takes care of compiling AMCL, and places it where `cmake` can
find it later.

### Build and install
To build and install, first download it, then run the following commands in the 
source code directory:
```
mkdir build
cd build
cmake ..
make
sudo make install
```
This builds the shared library (`libcifer.so`) and installs it.
By default, it places the shared library in `/usr/local/lib` and the header 
files in `/usr/local/include` (For this, you will need to run the command as 
superuser). To set a custom install directory (e.g. an `install` directory 
in the root of the repo) instead of `/usr/local`, pass it to the cmake 
command, e.g.:
```
cmake .. -DCMAKE_INSTALL_PREFIX=../install
```

### Test
The build commands also create an executable which runs all unit tests. 
To make sure the library works as expected, run
```
make test
```
Note that this command also builds the library and test executable if they 
have not been built yet.

### Try it out with Docker
We provide a simple Docker build for trying out the library without worrying 
about the installation and the dependencies. You can build a Docker image
yourself by running (possibly with sudo)
```
docker build . -t fentec/cifer
```
or downloading it from Docker Hub
```
docker pull fentec/cifer
```
In the file `example/example.c` you will find a dummy code using CiFEr library.
Modify it as you wish and then run
```
docker run -v $PATHTOCIFER/example:/CiFEr/example fentec/cifer
```
where `$PATHTOCIFER` is your absolute path to CiFEr library (something like `/home/username/CiFEr`).
This will link the `example` folder in your repository with the one in the Docker image.
Then it will compile `example.c` code and execute it. See the instructions bellow
on how to use schemes implemented in CiFEr or check out any of the tests implemented
in `test` folder.

## Using CiFEr in your project
After you have successfuly built and installed the library, you can use it in 
your project. Instructions below provide a brief introduction to the most 
important parts of the library, and guide you through a sequence of steps that 
will quickly get your FE example up and running.  

### Including and linking
To use the library, you must `#include` its headers in your source code and 
link it with the `-lcifer` flag when building your code.

### Select the FE scheme
You can choose from the following set of schemes:

#### Inner product schemes
You will need to include headers from `innerprod` directory.

We organized implementations in two categories based on their security 
assumptions:

* Schemes with **selective security under chosen-plaintext attacks** (s-IND-CPA 
security):
    * Schemes by _Abdalla, Bourse, De Caro, Pointcheval_ ([paper](https://eprint.iacr.org/2015/017.pdf)). 
        The scheme can be instantiated from DDH (`cfe_ddh`) and LWE (`cfe_lwe`).
    * Experimental Ring-LWE scheme whose security will be argued in a future paper (`cfe_ring_LWE`).        
    * Multi-input scheme based on paper by _Abdalla, Catalano, Fiore, Gay, Ursu_ 
        ([paper](https://eprint.iacr.org/2017/972.pdf)) and instantiated from 
        the scheme in the first point (`cfe_ddh_multi`).

* Schemes with stronger **adaptive security under chosen-plaintext attacks** (IND-CPA
security)  or **simulation based security** (SIM-Security for IPE):
    * Scheme based on paper by _Agrawal, Libert and Stehlé_ 
        ([paper](https://eprint.iacr.org/2015/608.pdf)). It can be instantiated 
        from Damgard DDH (`cfe_damgard` - similar to `cfe_ddh`, but uses one 
        more group element to achieve full security, similar to how Damgård's 
        encryption scheme is obtained from ElGamal scheme 
        ([paper](https://link.springer.com/chapter/10.1007/3-540-46766-1_36))), 
        LWE (`cfe_lwe_fs`) and Paillier (`cfe_paillier`) primitives.
    * Multi-input scheme based on paper by _Abdalla, Catalano, Fiore, Gay, Ursu_ 
    ([paper](https://eprint.iacr.org/2017/972.pdf)) and instantiated from the 
    scheme in the first point (`cfe_damgard_multi`).
    * Decentralized scheme based on paper by _Chotard, Dufour Sans, Gay, Phan and Pointcheval_
     ([paper](https://eprint.iacr.org/2017/989.pdf)). This scheme does not require a trusted
     party to generate keys. It is built on pairings  (`cfe_dmcfe`).
    * Decentralized scheme based on paper by _Abdalla, Benhamouda, Kohlweiss, Waldner_
     ([paper](https://eprint.iacr.org/2019/020.pdf)). Similarly as above this scheme
     this scheme does not require a trusted party to generate keys and is based on a general 
    procedure for decentralization of an inner product scheme, in particular the
    decentralization of a Damgard DDH scheme (``cfe_damgard_dec_multi``).
    * Function hiding inner product scheme by _Kim, Lewi, Mandal, Montgomery, Roy, Wu_
    ([paper](https://eprint.iacr.org/2016/440.pdf)). The scheme allows the decryptor to
decrypt the inner product of x and y without reveling (ciphertext) x or (function) y (`cfe_fhipe`).
    * Function hiding multi-input scheme based on paper by _Datta, Okamoto, Tomida_
    ([paper](https://eprint.iacr.org/2018/061.pdf)). This scheme allows clients to encrypt vectors and derive 
functional key that allows a decrytor to decrypt an inner product without revealing the ciphertext or the function (`cfe_fh_multi_ipe`).


#### Quadratic scheme
You will need to include headers from `quadratic` directory.

It contains an implementation of an efficient FE scheme for quadratic
multi-variate polynomials by Sans, Gay and Pointcheval ([paper](https://eprint.iacr.org/2018/206.pdf))
which is based on bilinear pairings, and offers adaptive security
under chosen-plaintext attacks (IND-CPA security).

#### Attribute based encryption (ABE) schemes
You will need to include headers from `abe` directory. There are three implemented
schemes:
* A ciphertext policy (CP) ABE scheme named FAME by _Agrawal and Chase_
([paper](https://eprint.iacr.org/2017/807.pdf)) allowing encrypting a
message based on a boolean expression defining a policy which attributes
are needed for the decryption. The functions needed in this scheme have prefix
`cfe_fame`.

* A key policy (KP) ABE scheme by _Goyal, Pandey, Sahai, and Waters_
([paper](https://eprint.iacr.org/2006/309.pdf)) allowing a distribution of keys
following a boolean expression defining a policy which attributes are needed for
the decryption. The functions needed in this scheme have prefix `cfe_gpsw`.

* A decentralized inner product predicate scheme by _Michalevsky, Joye_ ([paper](https://eprint.iacr.org/2018/753.pdf)) allowing encryption
with policy described as a vector, and a decentralized distribution of keys based on users' vectors so that
only users with  vectors orthogonal to the encryption vector posses a key that can decrypt the ciphertext.
The functions needed in this scheme have prefix `cfe_dippe`.


These schemes allow to specify a decryption policy defining which attributes are
needed to be able to decrypt. For the latter we implemented a policy converter
which accepts a boolean expression defining the policy and outputs a monotone
span program (MSP) which can be used as an input for the ABE schemes.

### Configure selected scheme
All CiFEr schemes are implemented as C structs + functions which operate on 
them with (at least logically) similar APIs. So the first thing we need to do 
is to create a scheme instance by initializing the appropriate struct. For 
this step, we need to pass in some configuration, e.g. values of parameters for 
the selected scheme.

Let's say we selected a `cfe_ddh` scheme. We create a new scheme instance with:
```c
mpz_t bound;
mpz_init_set_ui(bound, 2 << 14);
cfe_ddh s;
cfe_ddh_init(&s, 3, 128, bound);
```

In the last line above, the first argument is length of input vectors **x**
and **y**, the second argument is bit length of prime modulus _p_
(because this particular scheme operates in the &#8484;<sub>p</sub> group), and
the last argument represents the upper bound for elements of input vectors.

However, configuration parameters for different FE schemes vary quite a bit.
Please refer to [library documentation](https://fentec-project.github.io/CiFEr) 
regarding the meaning of parameters for specific schemes. For now, examples and 
reasonable defaults can be found in the test code. 
 
After you successfully created a FE scheme instance, you can call the relevant 
 functions for:
* generation of (secret and public) master keys,
* derivation of functional encryption key,
* encryption, and
* decryption. 

### Prepare input data
#### Vectors and matrices
All CiFEr chemes rely on vectors (or matrices) of big integer (`mpz_t`) 
components. 

CiFEr schemes use the library's own vector (`cfe_vec`) and matrix (`cfe_mat`) 
types. They are available in the `data` directory. A `cfe_vec` is basically a 
wrapper around an array of `mpz_t` integers, while a `cfe_mat` is a wrapper 
around an array of `cfe_vec` vectors.

In general, you only have to worry about providing input data (usually
vectors **x** and **y**). Each element in a vector or matrix can be set by 
calling their respective `_set` function, for example:
```c
cfe_vec x, y;
cfe_vec_init(&x, 3);
cfe_vec_init(&y, 3);
mpz_t el;
mpz_init(el);
for (size_t i = 0; i < 3; i++) {
    mpz_set_ui(el, i+1);
    cfe_vec_set(&x, el, i);
    cfe_vec_set(&y, el, 2-i);
}
// x is [1, 2, 3], y is [3, 2, 1]
```

For matrices, you can set whole rows to contain the same values as a vector.
```c
cfe_mat A;
cfe_mat_init(&A, 2, 3);
cfe_mat_set_vec(&A, &x, 0);
cfe_mat_set_vec(&A, &y, 1);
// A is [[1, 2, 3], [3, 2, 1]]
```

#### Random data
To generate random `mpz_t` values from different probability distributions,
you can use one of our several implementations of random samplers. The samplers
are provided in the `sample` directory. Note that the uniform sampler does not 
require special initialization while other samplers do. Before performing any
random sampling, the function `cfe_init` needs to be called to ensure that the
system's random number generator has been properly seeded.
 
You can quickly construct random vectors and matrices by:
1. Configuring the sampler of your choice, for example:
    ```c
    cfe_init();
    mpf_t sigma;
    mpf_init_set_ui(sigma, 10);
    cfe_normal_cumulative s;    // samples the cumulative normal (Gaussian) probability distribution, centered on 0
    cfe_normal_cumulative_init(&s, sigma, 256, true);
    ```
2. Providing the data structure and sampler as an argument to the relevant 
`_sample_vec` or `_sample_mat` functions. 
    ```c
    cfe_vec v;
    cfe_mat m;
    cfe_vec_init(&v, 5);
    cfe_mat_init(&m, 2, 3);
    cfe_normal_cumulative_sample_vec(&v, &s); // sets all elements of the vector to random elements
    cfe_normal_cumulative_sample_mat(&m, &s); // sets all elements of the matrix to random elements
    
    // Uniform sampler (does not need to be initialized)
    mpz_t max;
    mpz_init_set_ui(max, 10);
    cfe_uniform_sample_vec(&v, max);
    ```
    
## Use the scheme (examples)
In the following we give some examples how to use the schemes. Note that every
scheme has an implemented test in the folder `test`, in which you can see how
to use the scheme and modify it to your needs.

Please remember that all the examples below omit error handling. All functions
which can fail return a `cfe_error` (its definition is in `errors.h` header, 
located in the `internal` directory) which is non-zero if the function 
encountered an error.

Additionally, all examples also omit memory freeing. In CiFEr, all functions
which allocate memory for their results (passed as input parameters) have the
suffix `_init` and have a corresponding function with the suffix `_free`.
All other functions expect their inputs to be already initialized and do not
allocate any memory the user would need to free manually.

##### Using a single input scheme
The example below demonstrates how to use single input scheme instances.
Although the example shows how to use the `cfe_ddh` scheme from directory 
`simple`, the usage is similar for all single input schemes, regardless
of their security properties (s-IND-CPA or IND-CPA) and instantiation
 (DDH or LWE).
 
You will see that three `cfe_ddh` structs are instantiated to simulate the
 real-world scenarios where each of the three entities involved in FE
 are on separate machines.
 
```c
// Instantiation of a trusted entity that
// will generate master keys and FE key
size_t l = 2; // length of input vectors
mpz_t bound, fe_key, xy, el;
mpz_inits(bound, fe_key, xy, el, NULL);
mpz_set_ui(bound, 10); // upper bound for input vector coordinates
modulus_len = 1024; // bit length of prime modulus p

cfe_ddh s, encryptor, decryptor;
cfe_ddh_init(&s, l, modulus_len, bound);
cfe_vec msk, mpk, ciphertext, x, y;
cfe_ddh_master_keys_init(&msk, &mpk, &s);
cfe_ddh_generate_master_keys(&msk, &mpk, &s);

cfe_vec_init(&y, 2);
mpz_set_ui(el, 1);
cfe_vec_set(&y, el, 0);
mpz_set_ui(el, 2);
cfe_vec_set(&y, el, 1); // y is [1, 2]

cfe_ddh_derive_fe_key(fe_key, &s, &msk, &y);

// Simulate instantiation of encryptor 
// Encryptor wants to hide x and should be given
// master public key by the trusted entity
cfe_vec_init(&x, 2);
mpz_set_ui(el, 3);
cfe_vec_set(&x, el, 0);
mpz_set_ui(el, 4);
cfe_vec_set(&x, el, 1); // x is [3, 4]

cfe_ddh_copy(&encryptor, &s);
cfe_ddh_ciphertext_init(&ciphertext, &encryptor);
cfe_ddh_encrypt(&ciphertext, &encryptor, &x, &mpk);

// Simulate instantiation of decryptor that decrypts the cipher 
// generated by encryptor.
cfe_ddh_copy(&decryptor, &s);
// decrypt to obtain the result: inner prod of x and y
// we expect xy to be 11 (e.g. <[1,2],[3,4]>)
cfe_ddh_decrypt(xy, &decryptor, &ciphertext, fe_key, &y);
```

##### Using a multi input scheme
This example demonstrates how multi input FE schemes can be used.
 
Here we assume that there are `numClients` encryptors (e<sub>i</sub>), each with 
their corresponding input vector x<sub>i</sub>. A trusted entity generates all 
the master keys needed for encryption and distributes appropriate keys to 
appropriate encryptor. Then, encryptor e<sub>i</sub> uses their keys to encrypt 
their data x<sub>i</sub>. The decryptor collects ciphers from all the 
encryptors. It then relies on the trusted entity to derive a decryption key 
based on its own set of vectors y<sub>i</sub>. With the derived key, the 
decryptor is able to compute the result - inner product over all vectors, as 
_Σ <x<sub>i</sub>,y<sub>i</sub>>._

```c
size_t numClients = 2;             // number of encryptors
size_t l = 3;                 // length of input vectors
mpz_t bound, prod;
mpz_init(prod);
mpz_init_set_ui(bound, 1000); // upper bound for input vectors

// Simulate collection of input data.
// X and Y represent matrices of input vectors, where X are collected
// from numClients encryptors (ommitted), and Y is only known by a single decryptor.
// Encryptor i only knows its own input vector X[i].
cfe_mat X, Y;
cfe_mat_inits(numClients, l, &X, &Y, NULL);
cfe_uniform_sample_mat(&X, bound);
cfe_uniform_sample_mat(&Y, bound);

// Trusted entity instantiates scheme instance and generates
// master keys for all the encryptors. It also derives the FE
// key derivedKey for the decryptor.
size_t modulus_len = 1024;
cfe_ddh_multi m, decryptor;
cfe_ddh_multi_init(&m, numClients, l, modulus_len, bound);

cfe_mat mpk;
cfe_ddh_multi_sec_key msk;
cfe_ddh_multi_master_keys_init(&mpk, &msk, &m);
cfe_ddh_multi_generate_master_keys(&mpk, &msk, &m);
cfe_ddh_multi_fe_key fe_key;
cfe_ddh_multi_fe_key_init(&fe_key, &m);
cfe_ddh_multi_derive_fe_key(&fe_key, &m, &msk, &Y);

// Different encryptors may reside on different machines.
// We simulate this with the for loop below, where numClients
// encryptors are generated.
cfe_ddh_multi_enc encryptors[numClients];
for (size_t i = 0; i < numClients; i++) {
    cfe_ddh_multi_enc_init(&encryptors[i], &m);
}

// Each encryptor encrypts its own input vector X[i] with the
// keys given to it by the trusted entity.
cfe_mat ciphertext;
cfe_mat_init(&ciphertext, numClients, l + 1);
for (size_t i = 0; i < numClients; i++) {
    cfe_vec ct;
    cfe_vec *pub_key = cfe_mat_get_row_ptr(&mpk, i);
    cfe_vec *otp = cfe_mat_get_row_ptr(&msk.otp_key, i);
    cfe_vec *x_vec = cfe_mat_get_row_ptr(&X, i);
    cfe_ddh_multi_ciphertext_init(&ct, &encryptors[i]);
    cfe_ddh_multi_encrypt(&ct, &encryptors[i], x_vec, pub_key, otp);
    cfe_mat_set_vec(&ciphertext, &ct, i);
    cfe_vec_free(&ct);
}

// Ciphers are collected by decryptor, who then computes
// inner product over vectors from all encryptors.
cfe_ddh_multi_copy(&decryptor, &m);
cfe_ddh_multi_decrypt(prod, &decryptor, &ciphertext, &fe_key, &Y);
```
Note that above we instantiate multiple encryptors - in reality, 
different encryptors will be instantiated on different machines.

##### Using a quadratic scheme
In the example below, we omit instantiation of different entities
(encryptor and decryptor).
```c
// set the parameters
size_t l = 5;
mpz_t b;
mpz_set_si(b, 8);

// create a scheme
cfe_sgp s;
err = cfe_sgp_init(&s, l, b);

// create a master secret key
cfe_sgp_sec_key msk;
cfe_sgp_sec_key_init(&msk, &s);
cfe_sgp_generate_sec_key(&msk, &s);

// take random vectors x, y
cfe_vec x, y;
cfe_vec_inits(s.l, &x, &y, NULL);
cfe_uniform_sample_vec(&x, b);
cfe_uniform_sample_vec(&y, b);

// encrypt the vectors
cfe_sgp_cipher cipher;
cfe_sgp_cipher_init(&cipher, &s);
cfe_sgp_encrypt(&cipher, &s, &x, &y, &msk);

// derive keys and decrypt the value x*m*y for a
// random matrix m
cfe_mat m;
cfe_mat_init(&m, l, l);
cfe_uniform_sample_mat(&m, b);
ECP2_BN254 key;
cfe_sgp_derive_fe_key(&key, &s, &msk, &m);
mpz_t dec;
mpz_init(dec);
cfe_sgp_decrypt(dec, &s, &cipher, &key, &m);
```

##### Using ABE schemes

In the example below we demonstrate a usage of ABE scheme FAME. We
omit instantiation of different entities (encryptor and decryptor).
We want to encrypt the following message msg so that only those
who own the attributes satisfying a boolean expression 'policy'
can decrypt.
```c
// create a new FAME struct
cfe_fame fame;
cfe_fame_init(&fame);

// initialize and generate a public key and a secret key for the scheme
cfe_fame_pub_key pk;
cfe_fame_sec_key sk;
cfe_fame_sec_key_init(&sk);
cfe_fame_generate_master_keys(&pk, &sk, &fame);

// create a message to be encrypted
FP12_BN254 msg;
FP12_BN254_one(&msg);

// create a msp structure out of a boolean expression representing the
// policy specifying which attributes are needed to decrypt the ciphertext
char bool_exp[] = "(5 OR 3) AND ((2 OR 4) OR (1 AND 6))";
cfe_msp msp;
cfe_boolean_to_msp(&msp, bool_exp, false);

// initialize a ciphertext and encrypt the message based on the msp structure
// describing the policy
cfe_fame_cipher cipher;
cfe_fame_cipher_init(&cipher, &msp);
cfe_fame_encrypt(&cipher, &msg, &msp, &pk, &fame);

// produce keys that are given to an entity with a set
// of attributes in owned_attrib
int owned_attrib[] = {1, 3, 6};
cfe_fame_attrib_keys keys;
cfe_fame_attrib_keys_init(&keys, 3); // the number of attributes needs to be specified
cfe_fame_generate_attrib_keys(&keys, owned_attrib, 3, &sk, &fame);

// decrypt the message with owned keys
FP12_BN254 decryption;
cfe_fame_decrypt(&decryption, &cipher, &keys, &fame);
```

##### Serialize ABE material

In the example below we demonstrate how to serialize public key for ABE scheme GPSW.
The serialization is done by using [Protobuf](https://github.com/protocolbuffers/protobuf)
library, converting CiFEr structures to a single array of bytes.
The serialization is currently available for ABE schemes FAME and GPSW.
```c
// create GPSW structure
cfe_gpsw gpsw;
cfe_gpsw_init(&gpsw, 10);

// create and init GPSW master keys
cfe_gpsw_pub_key pk;
cfe_vec sk;
cfe_gpsw_master_keys_init(&pk, &sk, &gpsw);
cfe_gpsw_generate_master_keys(&pk, &sk, &gpsw);

// serialize public key into a buffer of bytes
cfe_ser buf;
cfe_gpsw_pub_key_ser(&pk, &buf);
```
