#include "modmul.h"

size_t mpn_lop(const mp_limb_t* x, size_t l_x) {
  while ((l_x > 1) && (x[l_x - 1] == 0 )) {
    l_x--;
  }
  return l_x; 
}

// produces t + t mod N, given 0 <= t < N
void my_addmod(mpz_t r, const mpz_t t, const mpz_t N) {
  size_t l_t = t->_mp_size;
  size_t l_N = N->_mp_size;
  if (l_t == 0) {        // 0 has size 0, and 0 + 0 mod N = 0
    mpz_set_ui(r, 0);
    return;
  }
  mp_limb_t rs[l_t + 1];
  rs[l_t] = mpn_lshift(rs, t->_mp_d, l_t, 1); // t + t or 2 * t or t << 1
  size_t l_r = mpn_lop(rs, l_t + 1);
  if (l_r == l_N) {
    if (mpn_cmp(rs, N->_mp_d, l_r) >= 0) {  // if r >= N
      mpn_sub_n(rs, rs, N->_mp_d, l_r);     // r - N
    }
  } else if (l_r > l_N) { // r > N
      mpn_sub(rs, rs, l_r, N->_mp_d, l_N);  // r - N
  } // else r < N -> do nothing
  mpz_import(r, mpn_lop(rs, l_r), -1, sizeof(mp_limb_t), -1, 0, rs); // import rs into r
}

void montRhoSquared(mpz_t r, const mpz_t N) {
  if (mpz_cmp_ui(N, 1) == 0) {  // if N = 1, anything mod N = 0
    mpz_set_ui(r, 0);
    return;
  }
  mpz_t t;
  mpz_init_set_ui(t, 1);
  int n = 2 * N->_mp_size * mp_bits_per_limb;
  for (int i = 1; i <= n; i++)
    my_addmod(t, t, N);
  mpz_swap(r, t);
  mpz_clear(t);
}

mp_limb_t montOmega(const mpz_t N) {
  // Since b is 2^w, gcd(N, b) != 1 if N is an even number
  if (mpz_tstbit(N, 0) == 0) {
    gmp_fprintf(stderr, "Montgomery omega function: %Zd has no inverse mod 2^%d\n", N, mp_bits_per_limb);
    return 0;
  }
  mp_limb_t t = 1;
  mp_limb_t N_0 = N->_mp_d[0]; // limb[0] is equivalent to N mod b
  for (int i = 1; i < mp_bits_per_limb; i++) {
    t = t * t;      // t * t mod b
    t = t * N_0;    // t * t * N mod b
  }
  return -t;        // - t mod b
}

/*
 To avoid recomputing omega, takes in omega as input
 */
void montRed_omega(mpz_t r, const mpz_t t, const mpz_t N, mp_limb_t omega) {
  size_t l_N = N->_mp_size;
  mp_limb_t temp_r[2 * l_N + 1], temp_N[3 * l_N];
  // N is put in the middle of temp_N surrounded by l_N empty limbs on both sides
  // This allows to freely "compute" N * b^i, but need to be careful with indexing

  mpn_zero(temp_r, 2 * l_N + 1);
  mpn_zero(temp_N, 3 * l_N);

  mpz_export(temp_r, NULL, -1, sizeof(mp_limb_t), -1, 0, t);
  mpz_export(&temp_N[l_N], NULL, -1, sizeof(mp_limb_t), -1, 0, N); // temp_N effectively starts from temp_N[l_N]

  mp_limb_t u;

  for (int i = 0; i < l_N; i++) {
    u = temp_r[i] * omega;
    temp_r[2 * l_N] += mpn_addmul_1(temp_r, &temp_N[l_N - i], 2 * l_N, u);  // r <- r + u * N * b^i
  }

  // division by b^(l_N) by essentially assuming r starts from r[l_N] and thus ignoring the first l_N limbs
  if (mpn_cmp(&temp_r[l_N], &temp_N[l_N], l_N + 1) >= 0)          // if r >= N
    mpn_sub_n(&temp_r[l_N], &temp_r[l_N], &temp_N[l_N], l_N + 1); // r <- r - N
  mpz_import(r, l_N, -1, sizeof(mp_limb_t), -1, 0, &temp_r[l_N]); // import temp_r into r
}

/*
Computes r = t * rho^(-1) mod N
*/
void montRed(mpz_t r, const mpz_t t, const mpz_t N) {
  montRed_omega(r, t, N, montOmega(N));
}

/*
To avoid recomputing omega, takes in omega as input
*/
void montMul_omega(mpz_t r, const mpz_t x, const mpz_t y, const mpz_t N, mp_limb_t omega) {
  size_t l_N = N->_mp_size;
  mp_limb_t temp_r[l_N + 2], temp_x[l_N + 1], temp_y[l_N + 1], temp_N[l_N + 1];

  // Initialise to 0
  mpn_zero(temp_r, l_N + 2);
  mpn_zero(temp_x, l_N + 1);
  mpn_zero(temp_y, l_N + 1);
  mpn_zero(temp_N, l_N + 1);

  mpz_export(temp_x, NULL, -1, sizeof(mp_limb_t), -1, 0, x);
  mpz_export(temp_y, NULL, -1, sizeof(mp_limb_t), -1, 0, y);
  mpz_export(temp_N, NULL, -1, sizeof(mp_limb_t), -1, 0, N);

  //mp_limb_t omega = montOmega(N);
  mp_limb_t u;

  for (int i = 0; i < l_N; i++) {
    u = (temp_r[0] + temp_y[i] * temp_x[0]) * omega;                      // u <- (r_0 + y_i + x_0) * omega (mod b)
    
    temp_r[l_N + 1]  = mpn_addmul_1(temp_r, temp_x, l_N + 1, temp_y[i]);  // r <- r + x * y_i
    temp_r[l_N + 1] += mpn_addmul_1(temp_r, temp_N, l_N + 1, u);          // r <- r + u * N

    for (int j = 0; j < l_N + 1; j++) { // divide by b
      temp_r[j] = temp_r[j + 1];
    }
    temp_r[l_N + 1] = 0;
  }
  if (mpn_cmp(temp_r, temp_N, l_N + 1) >= 0)    // if r >= N
    mpn_sub_n(temp_r, temp_r, temp_N, l_N + 1); // r <- r - N
  mpz_import(r, l_N, -1, sizeof(mp_limb_t), -1, 0, temp_r); // import temp_r into r
}

/*
Computes r = x * y * rho^(-1) mod N
*/
void montMul(mpz_t r, const mpz_t x, const mpz_t y, const mpz_t N) {
  montMul_omega(r, x, y, N, montOmega(N));
}

/*
Mainly for CRT: does Montgomery Reduction and multiplies by rho^3
to produce result t * rho mod N
*/
void montRedAndMulByRho_3(mpz_t r, const mpz_t t, const mpz_t N, const mpz_t rhoSquared, mp_limb_t omega) {
  mpz_t temp_r, rhoCubed;
  mpz_init(temp_r);
  montRed_omega(temp_r, t, N, omega);
  mpz_init_set(rhoCubed, rhoSquared);
  // computing rho^3 from rho^2
  for (int i = 1; i <= N->_mp_size * mp_bits_per_limb; i++) {
    my_addmod(rhoCubed, rhoCubed, N);
  }
  montMul_omega(temp_r, temp_r, rhoCubed, N, omega);
  mpz_swap(r, temp_r);
  mpz_clear(temp_r);
  mpz_clear(rhoCubed);
}

/*
To avoid recomputing rho^2, one and omega, takes them in as input
*/
void my_mulmod_rho_2_one_omega(mpz_t r, const mpz_t x, const mpz_t y, const mpz_t N, const mpz_t rhoSquared, const mpz_t one, mp_limb_t omega) {
  mpz_t temp_r, temp_x, temp_y;
  mpz_init(temp_r);
  mpz_init(temp_x);
  mpz_init(temp_y);

  montMul_omega(temp_x, x, rhoSquared, N, omega);  // x^ = x  * rho          <- montMul(x, rho^2)
  montMul_omega(temp_y, y, rhoSquared, N, omega);  // y^ = y  * rho          <- montMul(y, rho^2)
  montMul_omega(temp_r, temp_x, temp_y, N, omega); // r^ = x^ * y^ * rho(-1) <- montMul(x^, y^)
  montMul_omega(temp_r, temp_r, one, N, omega);    // r  = r^ * rho(-1)      <- montMul(r^, 1)
  
  mpz_swap(r, temp_r);
  mpz_clear(temp_r);
  mpz_clear(temp_x);
  mpz_clear(temp_y);
}

void my_mulmod(mpz_t r, const mpz_t x, const mpz_t y, const mpz_t N) {
  mpz_t rhoSquared, one;
  mpz_init(rhoSquared);
  mpz_init_set_ui(one, 1);
  mp_limb_t omega = montOmega(N);
  montRhoSquared(rhoSquared, N);

  my_mulmod_rho_2_one_omega(r, x, y, N, rhoSquared, one, omega);
  
  mpz_clear(rhoSquared);
  mpz_clear(one);
}

void my_pow_fixed_rho_2_one_omega(mpz_t r, const mpz_t x, const mpz_t y, const mpz_t N, const mpz_t rhoSquared, const mpz_t one, mp_limb_t omega) {
  mpz_t temp_r, temp_x;
  mpz_init_set_ui(temp_r, 1);
  mpz_init(temp_x);
  
  // Converting into Montgomery form
  montMul_omega(temp_r, temp_r, rhoSquared, N, omega);
  if (mpz_cmp(x, N) >= 0) { // mainly for CRT: does Montgomery Reduction and multiplies by rho^3
    montRedAndMulByRho_3(temp_x, x, N, rhoSquared, omega);
  } else {
    montMul_omega(temp_x, x, rhoSquared, N, omega);
  }

  unsigned int k = 6;
  unsigned int m = 1 << k; // m = 2^k
  //unsigned int blocks_per_limb = mp_bits_per_limb / k;
  size_t n = mpz_sizeinbase(y, m);
  size_t l_y = y->_mp_size;
  unsigned int y_m[n];

  // Recoding y into base-m representation
  mp_limb_t y_limbs[l_y];
  mpz_export(y_limbs, NULL, -1, sizeof(mp_limb_t), -1, 0, y);
  unsigned int mask = m - 1; // (1<<k) - 1

  y_m[0] = 0; // initialising least significant block of y to 0
  if (l_y != 0) { // if y = 0, l_y is 0, so mpn_rshift seg faults
    for (int i = 0; i < n; i++) {
      y_m[i] = y_limbs[0] & mask;
      mpn_rshift(y_limbs, y_limbs, l_y, k);
    }
  }

  // Very fast as doesn't use any shifting
  // but only works for k that divides mp_bits_per_limb (e.g. 4)
  /*
  for (int i = 0, index = 0; i < l_y; i++) {
    mp_limb_t limb = y->_mp_d[i];
    //printf("%dth limb %lu\n", i, limb);
    for (int j = 0; j < blocks_per_limb; j++) {
      index = i * blocks_per_limb + j;
      //printf("accessing block %d - %u\n", index, (unsigned int) (limb & (m-1)));
      if (index < n) {
        y_m[index] = limb & (m - 1);
        //printf("wrote block %d - %u\n", index, y_m[index]);
        limb = limb >> k;
      } else {
        goto endNestedLoop;
      }
    }
  }
  endNestedLoop:;
  */

  // Pre-computing T = x^i, i in {1, 2, 3, .., m - 1}
  mpz_t T[m-1];
  mpz_init_set(T[0], temp_x); // T[0] <- x
  for (int i = 1; i < m - 1; i++) {
    mpz_init(T[i]);
    montMul_omega(T[i], T[i-1], temp_x, N, omega); // x^i mod N <- x^(i-1) * x
  }

  for (int i = n - 1; i >= 0; i--) {
    for (int j = 0; j < k; j++)
      montMul_omega(temp_r, temp_r, temp_r, N, omega);
    if (y_m[i] != 0) // extracts ith block of y
      montMul_omega(temp_r, temp_r, T[y_m[i] - 1], N, omega);
  }

  // Converting from Montgomery form
  montMul_omega(temp_r, temp_r, one, N, omega);

  mpz_swap(r, temp_r);
  mpz_clear(temp_r);
  mpz_clear(temp_x);
  for (int i = 0; i < m - 1; i++) {
    mpz_clear(T[i]);
  }
}

void my_pow_slide_rho_2_one_omega(mpz_t r, const mpz_t x, const mpz_t y, const mpz_t N, const mpz_t rhoSquared, const mpz_t one, mp_limb_t omega) {
  mpz_t temp_r, temp_x;
  mpz_init_set_ui(temp_r, 1);
  mpz_init(temp_x);
  
  // Converting into Montgomery form
  montMul_omega(temp_r, temp_r, rhoSquared, N, omega);
  if (mpz_cmp(x, N) >= 0) { // mainly for CRT: does Montgomery Reduction and multiplies by rho^3
    montRedAndMulByRho_3(temp_x, x, N, rhoSquared, omega);
  } else {
    montMul_omega(temp_x, x, rhoSquared, N, omega);
  }

  unsigned int k = 6;
  unsigned int m = 1 << k; // m = 2^k
  size_t n = mpz_sizeinbase(y, 2);

  // Pre-computing T = x^i, i in {1, 3, 5, .., m - 1}
  mpz_t T[m>>1]; // 2^k / 2 <- m >> 1. Size of T is 2^(k-1)
  mpz_init_set(T[0], temp_x); // T[0] <- x
  mpz_t x_squared;
  mpz_init(x_squared);
  montMul_omega(x_squared, temp_x, temp_x, N, omega); // x^2 mod N
  for (int i = 1; i < m>>1; i++) {
    mpz_init(T[i]);
    montMul_omega(T[i], T[i-1], x_squared, N, omega); // T[prev] * x^2 <- x^(2*i+1) <- x^(2*(i-1)+1) * x^2
  }
  mpz_clear(x_squared);
  
  int i = n - 1, l;
  mp_limb_t u;
  while (i >= 0) {
    if (mpz_tstbit(y, i) == 0) {
      l = i;
      u = 0;
    } else {
      l = MAX(i - k + 1, 0);
      while (mpz_tstbit(y, l) == 0) { // same as y_i..l = 0 (mod 2)
        l++;
      }
      //extract bits i..l and assign to u
      if (i / mp_bits_per_limb == l / mp_bits_per_limb) { // if same limb
        u = y->_mp_d[i / mp_bits_per_limb];
        u >>= l % mp_bits_per_limb; // shift limb by l positions to the right, so the window starts at the LSB
        u &= (1<<(i - l + 1)) - 1;  // only keep the window via bitwise AND with a run of 1's of window size
      } else {
        mp_limb_t limb_l, limb_r;
        limb_l = y->_mp_d[i / mp_bits_per_limb]; // left limb
        limb_r = y->_mp_d[l / mp_bits_per_limb]; // right limb
        // mod is used to make the indices i and l relative to positions in a single window
        limb_r >>= l % mp_bits_per_limb; // shift right limb by l positions to the right, so the right part of the window starts at the LSB
        limb_l <<= mp_bits_per_limb - ((i % mp_bits_per_limb) + 1); // place left part of the window at the left end of the limb, which is essentially rotate shifting it to the right by (i + 1)
        limb_l >>= (l % mp_bits_per_limb) - ((i % mp_bits_per_limb) + 1); // now shift by the remaining l-(i+1) bits to the right so it's aligned with the right part
        u = limb_l | limb_r; // join the two parts via bitwise OR
      }
    }
    for (int j = 0; j < (i - l + 1); j++) { // temp_r ^ (2^(i-l+1))
      montMul_omega(temp_r, temp_r, temp_r, N, omega); // temp_r^2
    }
    if (u != 0) {
      montMul_omega(temp_r, temp_r, T[(u - 1)/2], N, omega); // temp_r * x^u <- temp_r * T[floor((u - 1)/2)]
    }
    i = l - 1;
  }

  // Converting from Montgomery form
  montMul_omega(temp_r, temp_r, one, N, omega);

  mpz_swap(r, temp_r);
  mpz_clear(temp_r);
  mpz_clear(temp_x);
  for (int i = 0; i < m>>1; i++) {
    mpz_clear(T[i]);
  }
}

void my_pow(mpz_t r, const mpz_t x, const mpz_t y, const mpz_t N) {
  mpz_t rhoSquared, one;
  mpz_init(rhoSquared);
  mpz_init_set_ui(one, 1);
  montRhoSquared(rhoSquared, N);
  mp_limb_t omega = montOmega(N);

  my_pow_slide_rho_2_one_omega(r, x, y, N, rhoSquared, one, omega);
  //my_pow_fixed_rho_2_one_omega(r, x, y, N, rhoSquared, one, omega);

  mpz_clear(rhoSquared);
  mpz_clear(one);
}

void genKey(unsigned char *key, int byteLength) {
  FILE *f = fopen("/dev/random", "r");
  size_t readResult = fread(key, sizeof(unsigned char), byteLength, f);
  fclose(f);
  
  if (readResult != byteLength) {
    fprintf(stderr, "Failed to read from /dev/random.");
  }
}

int getBytes(EVP_CIPHER_CTX *ctx, unsigned char *bytes, unsigned char *counter, int bitLength) {
  int index = 0, bytesEncrypted = 0;
  int n = bitLength / 128; // number of iterations
  // if there is a remainder, add one more round
  if (bitLength % 128)
    n++;
  for (int i = 0; i < n; i++) {
    EVP_EncryptUpdate(ctx, &bytes[index], &bytesEncrypted, counter, 16);
    index += bytesEncrypted;
    // incrementing the counter
    for (int j = 0; j < 16; j++) {
      if (++counter[j])
        break;
    }
  }
  return index;
}

void genPrime(mpz_t p, EVP_CIPHER_CTX *ctx, unsigned char *counter) {
  unsigned char bytes[80] = {0}; // 64 + 16 (one extra block just in case) to store 512 bit numbers
  int bytesEncrypted;
  do {
    bytesEncrypted = getBytes(ctx, bytes, counter, 512);
    // ensure it's congruent to 3 mod 4 by setting the first 2 LSB to 11
    bytes[0] |= 0x3;
    mpz_import(p, bytesEncrypted, -1, sizeof(unsigned char), -1, 0, bytes); // import bytes into p
  } while (!mpz_probab_prime_p (p, 200)); // while not prime
}

void genS(mpz_t s, const mpz_t N, EVP_CIPHER_CTX *ctx, unsigned char *counter) {
  unsigned char bytes[144] = {0}; // 128 + 16 (one extra block just in case) to store 1024 bit numbers
  size_t n = mpz_sizeinbase(N, 2); // number of bits in N
  int bytesEncrypted;
  mpz_t gcd;
  mpz_init(gcd);
  
  do {
    bytesEncrypted = getBytes(ctx, bytes, counter, n);
    // ensuring new number doesn't have more bits than N
    for (int i = bytesEncrypted * 8 - 1; i >= n; i--) {
      bytes[i/8] &= ~(1 << (i % 8)); // turning off bit i in byte[i/8].
    }
    mpz_import(s, bytesEncrypted, -1, sizeof(unsigned char), -1, 0, bytes); // import bytes into s
    if (mpz_cmp(s, N) < 0) // i.e. s = 0 .. N-1
      mpz_gcd(gcd, s, N);
    else
      continue;
  } while (mpz_cmp_ui(gcd, 1) != 0); // which gcd(s, N) != 1

  mpz_clear(gcd);
}

/*
 Generates initial state (s, N)
 */
void bbs_seed(mpz_t s, mpz_t N) {
  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  unsigned char key[16] = {0};
  unsigned char counter[16] = {0};
  
  genKey(key, 16);

  EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL);

  mpz_t p, q;
  mpz_init(p);
  mpz_init(q);

  genPrime(p, ctx, counter);
  genPrime(q, ctx, counter);

  mpz_mul(N, p, q);
  
  genS(s, N, ctx, counter);

  mpz_powm_ui(s, s, 2, N); // s^2 mod N

  EVP_CIPHER_CTX_free(ctx);
  mpz_clear(p);
  mpz_clear(q);
}

/*
 Updates the state (s, N) and returns a random bit
 */
int bbs_update(mpz_t s, const mpz_t N) {
  mpz_powm_ui(s, s, 2, N); // s^2 mod N
  return s->_mp_d[0] & 1;  // return LSB of s
}

/*
 Assigns to r a random number in the range 0 to n-1, inclusive
 */
void bbs_genRandom(mpz_t r, mpz_t s, const mpz_t N, const mpz_t n) {
  mpz_t temp_r, temp;
  mpz_init(temp_r);
  mpz_init(temp);
  
  size_t nPrime = mpz_sizeinbase(n, 2);
  
  // Check if n is a power of two, i.e. if (n & (n - 1)) == 0
  mpz_set(temp_r, n);
  mpz_sub_ui(temp, n, 1);
  mpz_and(temp, temp_r, temp);
  if (mpz_cmp_ui(temp, 0) == 0)
    nPrime--;
  do {
    mpz_set_ui(temp_r, 0); // reset to 0
    for (int i = 0; i < nPrime; i++) {
      if (bbs_update(s, N)) // if b == 1
        mpz_setbit(temp_r, i);
    }
  } while (mpz_cmp(temp_r, n) >= 0); // while r >= n
  
  mpz_swap(r, temp_r);
  mpz_clear(temp_r);
  mpz_init(temp);
}

/*
Perform stage 1:

- read each 3-tuple of N, e and m from stdin,
- compute the RSA encryption c,
- then write the ciphertext c to stdout.
*/

void stage1() {

  // fill in this function with solution
  mpz_t N, e, m, c;
  
  mpz_init(N);
  mpz_init(e);
  mpz_init(m);
  mpz_init(c);
  
  while (gmp_scanf("%ZX %ZX %ZX", N, e, m) == 3) {
    my_pow(c, m, e, N); // c = (m^e) mod N

    gmp_printf("%ZX\n", c);
  }

  mpz_clear(N);
  mpz_clear(e);
  mpz_clear(m);
  mpz_clear(c);
}

/*
Perform stage 2:

- read each 9-tuple of N, d, p, q, d_p, d_q, i_p, i_q and c from stdin,
- compute the RSA decryption m,
- then write the plaintext m to stdout.
*/

void stage2() {

  // fill in this function with solution
  mpz_t N, d, p, q, d_p, d_q, i_p, i_q, c, m, m_p, m_q;
  
  mpz_init(N);
  mpz_init(d);
  mpz_init(p);
  mpz_init(q);
  mpz_init(d_p);
  mpz_init(d_q);
  mpz_init(i_p);
  mpz_init(i_q);
  mpz_init(c);
  mpz_init(m);
  mpz_init(m_p);
  mpz_init(m_q);

  while (gmp_scanf("%ZX %ZX %ZX %ZX %ZX %ZX %ZX %ZX %ZX", N, d, p, q, d_p, d_q, i_p, i_q, c) == 9) {
    // naive implementation
    // my_pow(m, c, d, N);      // m = (c^d) mod N
    // CRT
    my_pow(m_p, c, d_p, p);   // m_p = c^(d (mod p-1)) mod p
    my_pow(m_q, c, d_q, q);   // m_q = c^(d (mod q-1)) mod q
    // combine m_p and m_q into m
    mpz_mul(m_p, m_p, q);    //   m_p * q
    mpz_mul(m_q, m_q, p);    //                                m_q * p
    mpz_mul(m_p, m_p, i_q);  //  (m_p * q) * (q^(-1) mod p)
    mpz_mul(m_q, m_q, i_p);  //                               (m_q * p) * (p^(-1) mod q)
    mpz_add(m, m_p, m_q);    //  (m_p * q) * (q^(-1) mod p) + (m_q * p) * (p^(-1) mod q)
    mpz_mod(m, m, N);        // ((m_p * q) * (q^(-1) mod p) + (m_q * p) * (p^(-1) mod q)) mod N
    gmp_printf("%ZX\n", m);
  }

  mpz_clear(N);
  mpz_clear(d);
  mpz_clear(p);
  mpz_clear(q);
  mpz_clear(d_p);
  mpz_clear(d_q);
  mpz_clear(i_p);
  mpz_clear(i_q);
  mpz_clear(c);
  mpz_clear(m);
  mpz_clear(m_p);
  mpz_clear(m_q);
}

/*
Perform stage 3:

- read each 5-tuple of p, q, g, h and m from stdin,
- compute the ElGamal encryption c = (c_1,c_2),
- then write the ciphertext c to stdout.
*/

void stage3() {

  // fill in this function with solution
  mpz_t p, q, g, h, m, k, c_1, c_2, q_1, rhoSquared, one, s, N;
//  gmp_randstate_t state;
//  unsigned long int seed;

  mpz_init(p);
  mpz_init(q);
  mpz_init(g);
  mpz_init(h);
  mpz_init(m);
  mpz_init(k);
  mpz_init(c_1);
  mpz_init(c_2);
  mpz_init(q_1);
  mpz_init(rhoSquared);
  mpz_init_set_ui(one, 1);
  mpz_init(s);
  mpz_init(N);

//  gmp_randinit_default(state);
//
//  FILE *f = fopen("/dev/random", "r");
//  size_t readResult = fread(&seed, sizeof(seed), 1, f); // function may block if there isn't enough entropy
//  fclose(f);
//  if (readResult != 1) {
//    fprintf(stderr, "Failed to read from /dev/random. Going to use current time as seed instead");
//    //sleep(1);          // sleeps for 1 second to ensure seed is always different
//    seed = time(NULL);    
//  }
//
//  gmp_randseed_ui(state, seed);

  bbs_seed(s, N);
  while (gmp_scanf("%ZX %ZX %ZX %ZX %ZX", p, q, g, h, m) == 5) {
    // exponent range is 1 .. q-1, mpz_urandomm(k,state,n) returns a value in the range 0 .. n-1, we want 0 .. q-2, n = q-1
    mpz_sub_ui(q_1, q, 1);        // q_1 = q-1
//    mpz_urandomm(k, state, q_1);
    bbs_genRandom(k, s, N, q_1);  // k = 0 .. q-2
    mpz_add_ui(k, k, 1);          // adding 1 brings it to 1 .. q-1 as required
    //mpz_set_ui(k, 1);           // k = 1 for testing stage3
    montRhoSquared(rhoSquared, p);
    mp_limb_t omega = montOmega(p);
    my_pow_slide_rho_2_one_omega(c_1, g, k, p, rhoSquared, one, omega); // c_1 = (g^k) mod p
    my_pow_slide_rho_2_one_omega(c_2, h, k, p, rhoSquared, one, omega); //             (h^k) mod p
    mpz_mul(c_2, m, c_2);         //        m * ((h^k) mod p)
    mpz_mod(c_2, c_2, p);         // c_2 = (m * h^k) mod p
    gmp_printf("%ZX\n%ZX\n", c_1, c_2);
  }

  mpz_clear(p);
  mpz_clear(q);
  mpz_clear(g);
  mpz_clear(h);
  mpz_clear(m);
  mpz_clear(k);
  mpz_clear(c_1);
  mpz_clear(c_2);
  mpz_clear(q_1);
  mpz_clear(rhoSquared);
  mpz_clear(one);
  mpz_clear(s);
  mpz_clear(N);

//  gmp_randclear(state);
}

/*
Perform stage 4:

- read each 5-tuple of p, q, g, x and c = (c_1,c_2) from stdin,
- compute the ElGamal decryption m,
- then write the plaintext m to stdout.
*/

void stage4() {

  // fill in this function with solution
  mpz_t p, q, g, x, c_1, c_2, m;
  
  mpz_init(p);
  mpz_init(q);
  mpz_init(g);
  mpz_init(x);
  mpz_init(c_1);
  mpz_init(c_2);
  mpz_init(m);
  
  while (gmp_scanf("%ZX %ZX %ZX %ZX %ZX %ZX", p, q, g, x, c_1, c_2) == 6) {
    /*
    mpz_sub_ui(m, p, 1);    // p - 1
    mpz_sub(m, m, x);       // p - 1 - x
    mpz_mod(m, m, q);       // p - 1 - x mod q
    */
    
    mpz_sub(m, q, x);        //      q-x
    my_pow(m, c_1, m, p);    // c_1^(q-x) mod p
    mpz_mul(m, m, c_2);      // c_1^(q-x) * c_2
    mpz_mod(m, m, p);        // c_1^(q-x) * c_2 mod p
    
    gmp_printf("%ZX\n", m);
  }
  
  mpz_clear(p);
  mpz_clear(q);
  mpz_clear(g);
  mpz_clear(x);
  mpz_clear(c_1);
  mpz_clear(c_2);
  mpz_clear(m);
}

/*
The main function acts as a driver for the assignment by simply invoking
the correct function for the requested stage.
*/

int main( int argc, char* argv[] ) {
  if     ( !strcmp( argv[ 1 ], "stage1" ) ) {
    stage1();
  }
  else if( !strcmp( argv[ 1 ], "stage2" ) ) {
    stage2();
  }
  else if( !strcmp( argv[ 1 ], "stage3" ) ) {
    stage3();
  }
  else if( !strcmp( argv[ 1 ], "stage4" ) ) {
    stage4();
  }
  else if ( !strcmp( argv[ 1 ], "test" ) ) {
    testExponentiation();
    testMontgomeryFunctions();
    testMontMul();
  } else if ( !strcmp( argv[ 1 ], "recode" ) ) {
    testRecode();
  } else if ( !strcmp( argv[ 1 ], "extract" ) ) {
    testExtractBits();
  } else if ( !strcmp( argv[ 1 ], "rand" ) ) {
    testRand();
  } else if ( !strcmp( argv[ 1 ], "write" ) ) {
    writeRandomBytesToFile();
  }

  return 0;
}

/*****************************
 * AUXILIARY FUNCTIONS BELOW *
 *****************************/

void mpz_printBinary(mpz_t x) {
  size_t n = mpz_sizeinbase(x, 2);
  for (int i = n - 1; i >= 0; i--) {
    printf("%d", mpz_tstbit(x, i));
    if (i % mp_bits_per_limb == 0)
      printf("|");
  }
  printf("\n");
}

void mpz_printBinaryRange(mpz_t x, int i, int l) {
  size_t n = mpz_sizeinbase(x, 2);
  for (int j = n - 1; j >= 0; j--) {
    if (j == i)
      printf(">");
    printf("%d", mpz_tstbit(x, j));
    if (j % mp_bits_per_limb == 0)
      printf("|");
    if (j == l)
      printf("<");
  }
  printf("\n");
}

void limb_printBinary(mp_limb_t x) {
  for (int i = mp_bits_per_limb - 1; i >= 0; i--) {
    printf("%lu", (x>>i) & 1);
  }
  printf("\n");
}

void limb_printBinaryRange(mp_limb_t x, int i, int l) {
  for (int j = mp_bits_per_limb - 1; j >= 0; j--) {
    if (j == i)
      printf(">");
    printf("%lu", (x>>j) & 1);
    if (j % mp_bits_per_limb == 0)
      printf("|");
    if (j == l)
      printf("<");
  }
  printf("\n");
}

/*
Tests the montMul function for a fixed modulus but random x
by converting x into Montgomery representation and then back again
*/
void testMontMul() {
  mpz_t r, x, N, rhoSquared, one;
  mpz_init(r);
  mpz_init(x);
  mpz_init_set_str(N, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16);
  mpz_init(rhoSquared);
  mpz_init_set_ui(one, 1);
  mp_limb_t omega = montOmega(N);
  
  gmp_randstate_t state;
  unsigned long int seed = time(NULL);
  gmp_randinit_default(state);
  
  gmp_randseed_ui(state, seed);
  
  montRhoSquared(rhoSquared, N);
  
  for (int i = 0; i < 10000000; i++) {
    mpz_urandomm(x, state, N);
    
    montMul_omega(r, x, rhoSquared, N, omega);
    montMul_omega(r, r, one, N, omega);
    
    if (mpz_cmp(r, x) != 0) {
      gmp_printf("Result %ZX != %ZX (original)\n", r, x);
    }
  }
  
  mpz_clear(r);
  mpz_clear(x);
  mpz_clear(N);
  mpz_clear(rhoSquared);
  mpz_clear(one);
  gmp_randclear(state);
}

/*
Tests all Montgomery-related functions
*/
void testMontgomeryFunctions() {
  mpz_t x, y, N, b, rhoSquared, one, myRes, gmpRes;
  mpz_init(x);
  mpz_init(y);
  mpz_init(N);
  mpz_init_set_ui(b, 2);
  mpz_pow_ui(b, b, mp_bits_per_limb);
  mpz_init(rhoSquared);
  mpz_init_set_ui(one, 1);
  mpz_init(myRes);
  mpz_init(gmpRes);
  
  gmp_randstate_t state;
  unsigned long int seed = 1;//time(NULL);
  gmp_randinit_default(state);
  
  gmp_randseed_ui(state, seed);
  
  int debug = 0;
  int loop = 1;
  
  if (!loop) {
    mpz_set_ui(x, 421);
    mpz_set_ui(y, 422);
    mpz_set_ui(N, 667);
    my_mulmod(myRes, x, y, N);
    gmp_printf("%Zd\n", myRes);
  } else {
    
    for (int i = 0; i < 10000; i++) {
      mpz_urandomb(N, state, 0xFFF);
      mpz_setbit(N, 0); // makes sure gcd(N, b) is 1
      
      mpz_urandomm(x, state, N);
      mpz_urandomm(y, state, N);
      
      //Testing my_addmod
      mpz_add(gmpRes, x, x);
      mpz_mod(gmpRes, gmpRes, N);
      my_addmod(myRes, x, N);
      
      if (debug || mpz_cmp(myRes, gmpRes) != 0) {
        gmp_printf("%d: %Zd + %Zd mod %Zd\n", i, x, x, N);
        gmp_printf("myRes:  %Zd (%s)\ngmpRes: %Zd\n", myRes, ((mpz_cmp(myRes, gmpRes) == 0) ? "Correct!" : "Wrong"), gmpRes);
      }
      
      //Testing omega
      mpz_invert(gmpRes, N, b);
      mpz_neg(gmpRes, gmpRes);
      mpz_mod(gmpRes, gmpRes, b);
      
      if (debug || mpz_cmp_ui(gmpRes, montOmega(N)) != 0) {
        gmp_printf("%d: omega for %Zd mod 2^%d\n", i, N, mp_bits_per_limb);
        gmp_printf("myRes:  %lu (%s)\ngmpRes: %Zd\n", montOmega(N), ((mpz_cmp_ui(gmpRes, montOmega(N)) == 0) ? "Correct!" : "Wrong"), gmpRes);
      }
      
      // Testing rho^2
      montRhoSquared(myRes, N);
      mpz_set_ui(gmpRes, 2);
      mpz_powm_ui(gmpRes, gmpRes, 2 * N->_mp_size * mp_bits_per_limb, N);
      
      if (debug || mpz_cmp(myRes, gmpRes) != 0) {
        gmp_printf("%d: rho^2 for %Zd\n", i, N);
        gmp_printf("myRes:  %Zd (%s)\ngmpRes: %Zd\n", myRes, ((mpz_cmp(myRes, gmpRes) == 0) ? "Correct!" : "Wrong"), gmpRes);
      }
      
      // Testing montMul
      mpz_mul(gmpRes, x, y);
      mpz_mod(gmpRes, gmpRes, N);
      my_mulmod(myRes, x, y, N);
      if (debug || mpz_cmp(myRes, gmpRes) != 0) {
        gmp_printf("%d: %Zd * %Zd mod %Zd\n", i, x, y, N);
        gmp_printf("myRes:  %Zd (%s)\ngmpRes: %Zd\n", myRes, ((mpz_cmp(myRes, gmpRes) == 0) ? "Correct!" : "Wrong"), gmpRes);
      }
      
      //      // Testing montRed
      //      montRed(myRes, x, N);
      //      //-----------
      //      montMul(myRes, myRes, rhoCubed, N);
      //      montMul(gmpRes, x, rhoSquared, N);
      //      //----or-----
      //      //montMul(gmpRes, x, one, N);
      //      //-----------
      //      if (debug || mpz_cmp(myRes, gmpRes) != 0) {
      //        gmp_printf("%d: montRed: %Zd * rho mod %Zd\n", i, x, N);
      //        gmp_printf("myRes:  %Zd (%s)\ngmpRes: %Zd\n", myRes, ((mpz_cmp(myRes, gmpRes) == 0) ? "Correct!" : "Wrong"), gmpRes);
      //      }
    }
    
  }
  
  mpz_clear(x);
  mpz_clear(y);
  mpz_clear(N);
  mpz_clear(b);
  mpz_clear(rhoSquared);
  mpz_clear(one);
  mpz_clear(myRes);
  mpz_clear(gmpRes);
  gmp_randclear(state);
}

/*
This was to check extracting bits from an mpz_t number
*/
void testExtractBits() {
  mpz_t a;
  
  mpz_init_set_str(a, "13245234567A4567854BBBCFFFFFFFFFFFFF", 16);
  int i = 126, l = 63;
  //extract bits i..l
  mpz_printBinaryRange(a, i, l);
  mp_limb_t extracted;
  if (i / mp_bits_per_limb == l / mp_bits_per_limb) { // if same limb
    extracted = a->_mp_d[i / mp_bits_per_limb];
    limb_printBinaryRange(extracted, i % mp_bits_per_limb, l % mp_bits_per_limb);
    extracted >>= l % mp_bits_per_limb;
    printf("x>>:");
    limb_printBinary(extracted);
    extracted &= (1<<(i - l + 1)) - 1;
    printf("res:");
    limb_printBinary(extracted);
  } else {
    mp_limb_t limb_l, limb_r;
    limb_l = a->_mp_d[i / mp_bits_per_limb];
    limb_r = a->_mp_d[l / mp_bits_per_limb];
    printf("left limb: %lX\tright limb: %lX\n", limb_l, limb_r);
    limb_r >>= l % mp_bits_per_limb;
    printf("r>>:");
    limb_printBinary(limb_r);
    limb_l <<= mp_bits_per_limb - ((i % mp_bits_per_limb) + 1);
    printf("l<<:");
    limb_printBinary(limb_l);
    limb_l >>= (l % mp_bits_per_limb) - ((i % mp_bits_per_limb) + 1);
    printf("l>>:");
    limb_printBinary(limb_l);
    extracted = limb_l | limb_r;
    printf("res:");
    limb_printBinary(extracted);
  }
  mpz_clear(a);
}

/*
This was to check recoding into base m
*/
void testRecode() {
  mpz_t y;
  mpz_init(y);
  mpz_set_str(y, "45242134249284709", 16);
  
  //gmp_printf("y is %ZX\n", y);
  unsigned int k = 5;
  unsigned int m = 1 << k; // m = 2^k
  unsigned int blocks_per_limb = mp_bits_per_limb / k;
  size_t n = mpz_sizeinbase(y, m);
  size_t l_y = y->_mp_size;
  unsigned int y_m[n];
  mp_limb_t y_limbs[l_y];
  mpz_export(y_limbs, NULL, -1, sizeof(mp_limb_t), -1, 0, y);
  unsigned int mask = m - 1; // (1<<k) - 1
  
  y_m[0] = 0; // initialise least significant bit to zero (prob not needed)
  
  printf("limb num %lu, blocks_per_limb %u, size in base %d: %lu\n", l_y, blocks_per_limb, m, n);
  
  //for (int z = 0; z < 1000000; z++) { // Fairly fast
  //mpz_export(y_limbs, NULL, -1, sizeof(mp_limb_t), -1, 0, y);
  for (int i = 0; i < n; i++) {
    y_m[i] = y_limbs[0] & mask;
    mpn_rshift(y_limbs, y_limbs, l_y, k);
    printf("%d: %02X\n", i, y_m[i]);
  }
  //}
  
  /*
   //for (int z = 0; z < 1000000; z++) { // to compare with the standard approach
   // limb-level implementation
   // Fastest but only works for k that divides mp_bits_per_limb
   for (int i = 0, index = 0; i < l_y; i++) {
   mp_limb_t limb = y->_mp_d[i];
   //printf("%dth limb %lu\n", i, limb);
   
   for (int j = 0; j < blocks_per_limb; j++) {
   index = i * blocks_per_limb + j;
   //printf("accessing block %d - %u\n", index, (unsigned int) (limb & (m-1)));
   if (index < n) {
   y_m[index] = limb & (m - 1);
   //printf("wrote block %d - %u\n", index, y_m[index]);
   limb = limb >> k;
   } else {
   goto endNestedLoop;
   }
   }
   }
   endNestedLoop:;
   //}
   */
  
  
  // Slowest as uses mpz functions
  /*
   mpz_t y_m_i, mask2;
   mpz_init(y_m_i);
   mpz_init_set_ui(mask2, m - 1);
   //for (int z = 0; z < 1000000; z++) {
   //mpz_set_str(y, "35465476546546546546546546", 10);
   int i = 0;
   while (mpz_cmp_ui(y, 0) != 0) {
   //if (i >= n) {
   //printf("WARNING i(%d) >= size in base(%lu)", i, n);
   //}
   mpz_and(y_m_i, y, mask2);
   y_m[i] = mpz_get_ui(y_m_i);
   
   mpz_tdiv_q_2exp(y, y, k);
   i++;
   }
   //}
   
   mpz_clear(y_m_i);
   mpz_clear(mask2);
   */
  /*
   for (int i = n - 1; i >= 0; i--) {
   printf("%X ", y_m[i]);
   if (i % blocks_per_limb == 0)
   printf("| ");
   }
   printf("\n");
   */
  mpz_clear(y);
}

/*
Tests exponentiation
*/
void testExponentiation() {
  mpz_t x, y, N, myRes, gmpRes;
  mpz_init(x);
  mpz_init(y);
  mpz_init(N);
  mpz_init(myRes);
  mpz_init(gmpRes);
  
  gmp_randstate_t state;
  unsigned long int seed = time(NULL);
  gmp_randinit_default(state);
  
  
  gmp_randseed_ui(state, seed);
  
  int debug = 0;
  int loop = 1;
  
  if (!loop) {
    mpz_set_ui(x, 1234);
    mpz_set_ui(y, 3094);
    if (debug)
      mpz_printBinary(y);
    mpz_set_ui(N, 256027);
    my_pow(myRes, x, y, N);
    mpz_powm(gmpRes, x, y, N);
    if (debug || mpz_cmp(myRes, gmpRes) != 0)
      gmp_printf("%Zd (%s)\n", myRes, ((mpz_cmp(myRes, gmpRes) == 0) ? "Correct!" : "Wrong"));
  } else {
    
    for (int i = 0; i < 10000; i++) {
      mpz_urandomb(N, state, 0xFF);
      //mpz_add_ui(N, N, 1);
      mpz_setbit(N, 0); // makes sure gcd(N, b) is 1
      mpz_urandomm(x, state, N);
      mpz_urandomm(y, state, N);
      my_pow(myRes, x, y, N);
      mpz_powm(gmpRes, x, y, N);
      
      if (debug || mpz_cmp(myRes, gmpRes) != 0) {
        gmp_printf("%d: %Zd ^ %Zd mod %Zd\n", i, x, y, N);
        gmp_printf("myRes:  %Zd (%s)\ngmpRes: %Zd\n", myRes, ((mpz_cmp(myRes, gmpRes) == 0) ? "Correct!" : "Wrong"), gmpRes);
      }
    }
  }
  
  mpz_clear(x);
  mpz_clear(y);
  mpz_clear(N);
  mpz_clear(myRes);
  mpz_clear(gmpRes);
  gmp_randclear(state);
}

/*
Tests pseudo random number generation
*/
void testRand() {
  mpz_t r, n, s, N;
  mpz_init(r);
  mpz_init_set_ui(n, 4056545334);
  mpz_init(s);
  mpz_init(N);
  
  bbs_seed(s, N);
  
  for (int i = 0; i < 10; i++) {
    bbs_genRandom(r, s, N, n);
    gmp_printf("r: %Zd\n", r);
  }
  
  mpz_clear(r);
  mpz_clear(n);
  mpz_clear(s);
  mpz_clear(N);
}


/*
This is for the NIST Statistical Test Suite
*/
void writeRandomBytesToFile() {
  int sequenceLength = 1000000;
  int numOfBitStreams = 20;
  mpz_t s, N;
  mpz_init(s);
  mpz_init(N);
  
  bbs_seed(s, N);
  
  FILE *f = fopen("randomBytes.txt", "wb");
  for (int i = 0; i < numOfBitStreams; i++) {
    for (int j = 0; j < sequenceLength/8; j++) {
      unsigned char byte = 0;
      for (int k = 0; k < 8; k++) {
        if(bbs_update(s, N))
          byte |= 1 << k; // set j'th bit
        else
          byte &= ~(1 << k); // turn off j'th bit
      }
      fwrite(&byte, sizeof(unsigned char), 1, f);
    }
  }
  
  fclose(f);
  mpz_clear(s);
  mpz_clear(N);
}
