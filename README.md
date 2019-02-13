# CiFEr - Functional Encryption library [![Build Status](https://travis-ci.org/fentec-project/CiFEr.svg?branch=master)](https://travis-ci.org/fentec-project/CiFEr)

CiFEr (prounounced as _cipher_) is a cryptographic library offering different 
state-of-the-art implementations of functional encryption schemes, specifically 
FE schemes for _linear_ polynomials (e.g. _inner products_). It is implemented 
in C. A [Go version named GoFE](https://github.com/fentec-project/gofe) 
of the library also exists.

To quickly get familiar with FE, read a short and very high-level 
introduction on our [Introductory Wiki page](https://github.com/fentec-project/gofe/wiki/Introduction-to-FE).

The documentation for CiFEr is available on 
[GitHub Pages](https://fentec-project.github.io/CiFEr).

CiFEr is distributed under the dual licenses, 
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

CiFEr relies on GMP for all big integer arithmetic. We recommend familiarizing 
yourself with it before using CiFEr. For the use of pairings in elliptic curves
CiFEr depends on AMCL and its implementation of BN254 curve. To be able to run
cmake file described bellow, AMCL library must be compiled with BN254 curve. This
can be done manually, but in addition we provide a bash script running a modified
python code that installs minimal requirements of AMCL library in the standard
directory `/usr/local/lib` and header files in `/usr/local/include`. For the
latter run:
````
cd external/amcl
sudo ./setup_amcl.sh
cd ../..
````

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
    * Scheme by _Abdalla et. al._ ([paper](https://eprint.iacr.org/2015/017.pdf)). 
        The scheme can be instantiated from DDH (`cfe_ddh`), LWE (`cfe_lwe`) and 
        RingLWE (`cfe_ring_lwe`) primitives.
    * Multi-input scheme based on paper by _Abdalla et.al_ 
        ([paper](https://eprint.iacr.org/2017/972.pdf)) and instantiated from 
        the scheme in the first point (`cfe_ddh_multi`).

* Schemes with **adaptive security under chosen-plaintext attacks** (IND-CPA
security) by 
    * Scheme based on paper by _Agrawal, Libert and Stehlé_ 
        ([paper](https://eprint.iacr.org/2015/608.pdf)). It can be instantiated 
        from Damgard DDH (`cfe_damgard` - similar to `cfe_ddh`, but uses one 
        more group element to achieve full security, similar to how Damgård's 
        encryption scheme is obtained from ElGamal scheme 
        ([paper](https://link.springer.com/chapter/10.1007/3-540-46766-1_36))), 
        LWE (`cfe_lwe_fs`) and Paillier (`cfe_paillier`) primitives.
    * Multi-input scheme based on paper by _Abdalla et.al_ 
    ([paper](https://eprint.iacr.org/2017/972.pdf)) and instantiated from the 
    scheme in the first point (`cfe_damgard_multi`).

### Configure selected scheme
All CiFEr schemes are implemented as C structs + functions which operate on 
them with (at least logically) similar APIs. So the first thing we need to do 
is to create a scheme instance by initializing the appropriate struct. For 
this step, we need to pass in some configuration, e.g. values of parameters for 
the selected scheme.

Let's say we selected a `cfe_ddh` scheme. We create a new scheme instance with:
````c
mpz_t bound;
mpz_init_set_ui(bound, 2 << 14);
cfe_ddh s;
cfe_ddh_init(&s, 3, 128, bound);
````

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
````c
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
````

For matrices, you can set whole rows to contain the same values as a vector.
````c
cfe_mat A;
cfe_mat_init(&A, 2, 3);
cfe_mat_set_vec(&A, &x, 0);
cfe_mat_set_vec(&A, &y, 1);
// A is [[1, 2, 3], [3, 2, 1]]
````

#### Random data
To generate random `mpz_t` values from different probability distributions,
you can use one of our several implementations of random samplers. The samplers
are provided in the `sample` directory. Note that the uniform sampler does not 
require special initialization while other samplers do.
 
You can quickly construct random vectors and matrices by:
1. Configuring the sampler of your choice, for example:
    ````c
    mpf_t sigma;
    mpf_init_set_ui(sigma, 10);
    cfe_normal_cumulative s;    // samples the cumulative normal (Gaussian) probability distribution, centered on 0
    cfe_normal_cumulative_init(&s, sigma, 256, true);
    ````
2. Providing the data structure and sampler as an argument to the relevant 
`_sample_vec` or `_sample_mat` functions. 
    ````c
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
    ````
    
## Use the scheme (examples)
Please note that all the examples below omit error handling. All functions 
which can fail return a `cfe_error` (its definition is in `errors.h` header, 
located in the `internal` directory) which is non-zero if the function 
encountered an error.

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
mpz_init_set_ui(bound, 10); // upper bound for input vector coordinates
modulus_len := 128 // bit length of prime modulus p

cfe_ddh s, encryptor, decryptor;
cfe_ddh_init(&s, l, modulus_len, bound);
cfe_vec msk, mpk, ciphertext, x, y;
cfe_ddh_generate_master_keys(&msk, &mpk, &s);

cfe_vec_init(&y, 2);
mpz_init_set_ui(el, 1);
cfe_vec_set(&y, el, 0);
mpz_set_ui(el, 2);
cfe_vec_set(&y, el, 1); // y is [1, 2]

cfe_ddh_derive_key(fe_key, &s, &msk, &y);

// Simulate instantiation of encryptor 
// Encryptor wants to hide x and should be given
// master public key by the trusted entity
cfe_vec_init(&x, 2);
mpz_init_set_ui(el, 3);
cfe_vec_set(&x, el, 0);
mpz_set_ui(el, 4);
cfe_vec_set(&x, el, 1); // x is [3, 4]

cfe_ddh_copy(&encryptor, &s);
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
 
Here we assume that there are `slots` encryptors (e<sub>i</sub>), each with 
their corresponding input vector x<sub>i</sub>. A trusted entity generates all 
the master keys needed for encryption and distributes appropriate keys to 
appropriate encryptor. Then, encryptor e<sub>i</sub> uses their keys to encrypt 
their data x<sub>i</sub>. The decryptor collects ciphers from all the 
encryptors. It then relies on the trusted entity to derive a decryption key 
based on its own set of vectors y<sub>i</sub>. With the derived key, the 
decryptor is able to compute the result - inner product over all vectors, as 
_Σ <x<sub>i</sub>,y<sub>i</sub>>._

```c
size_t slots = 2;             // number of encryptors
size_t l = 3;                 // length of input vectors
mpz_t bound, prod;
mpz_init_set_ui(bound, 1000); // upper bound for input vectors

// Simulate collection of input data.
// X and Y represent matrices of input vectors, where X are collected
// from slots encryptors (ommitted), and Y is only known by a single decryptor.
// Encryptor i only knows its own input vector X[i].
cfe_mat X, Y;
cfe_mat_inits(slots, l, &X, &Y, NULL);
cfe_uniform_sample_mat(&X, bound);
cfe_uniform_sample_mat(&Y, bound);

// Trusted entity instantiates scheme instance and generates
// master keys for all the encryptors. It also derives the FE
// key derivedKey for the decryptor.
size_t modulus_len = 64;
cfe_ddh_multi m, decryptor;
cfe_ddh_multi_init(&m, slots, l, modulus_len, bound);

cfe_mat mpk;
cfe_ddh_multi_sec_key msk;
cfe_ddh_multi_generate_master_keys(&mpk, &msk, &m);
cfe_ddh_multi_fe_key fe_key;
cfe_ddh_multi_derive_key(&fe_key, &m, &msk, &Y);

// Different encryptors may reside on different machines.
// We simulate this with the for loop below, where slots
// encryptors are generated.
cfe_ddh_multi_enc encryptors[slots];
for (size_t i = 0; i < slots; i++) {
    cfe_ddh_multi_enc_init(&encryptors[i], &m);
}

// Each encryptor encrypts its own input vector X[i] with the
// keys given to it by the trusted entity.
cfe_mat ciphertext;
cfe_mat_init(&ciphertext, slots, l + 1);
for (size_t i = 0; i < slots; i++) {
    cfe_vec ct;
    cfe_vec *pub_key = cfe_mat_get_row_ptr(&mpk, i);
    cfe_vec *otp = cfe_mat_get_row_ptr(&msk.otp_key, i);
    cfe_vec *x_vec = cfe_mat_get_row_ptr(&X, i);
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
 