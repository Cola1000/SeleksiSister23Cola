#include <stdio.h>

/* -------- basic types -------- */
typedef unsigned long long u64;
typedef __uint128_t u128;

/* -------- NTT modulus (2^64 - 2^32 + 1) and primitive root -------- */
static const u64 MOD = 18446744069414584321ULL; /* 0xFFFFFFFF00000001 */
static const u64 G   = 7ULL;

/* -------- limits (fit 1e6 digits per operand) --------
   For digit-wise convolution:
   If A,B have up to 1,000,000 digits, we need N >= next_pow2(lenA+lenB).
   lenA+lenB <= 2,000,000 -> next pow2 = 2,097,152 = 1<<21.
*/
#define MAXD   1000005              /* input buffer per number (+1 for '\0') */
#define MAXN   (1u<<21)             /* 2,097,152 */
static u64 A[MAXN], B[MAXN];        /* digit arrays for NTT */
static char SA[MAXD], SB[MAXD];     /* input buffers */

/* -------- utility: next power of two -------- */
static unsigned int next_pow2(unsigned int x){
    if (x <= 1u) return 1u;
    x--; x |= x>>1; x |= x>>2; x |= x>>4; x |= x>>8; x |= x>>16; return x+1;
}

/* -------- fast modular add/sub -------- */
static inline u64 add_mod(u64 a, u64 b){
    u64 s = a + b;
    /* if overflow OR s >= MOD, subtract MOD */
    if (s < a || s >= MOD) s -= MOD;
    return s;
}
static inline u64 sub_mod(u64 a, u64 b){
    return (a >= b) ? (a - b) : (a + MOD - b);
}

/* -------- fast reduction for 128-bit -> modulo MOD
   Using 2^64 ≡ 2^32 - 1 (mod MOD).
   Do two folds to land in [0, 2*MOD), then conditional subtracts.
*/
static inline u64 mul_mod(u64 a, u64 b){
    u128 t  = (u128)a * (u128)b;
    u64 lo  = (u64)t;
    u64 hi  = (u64)(t >> 64);

    /* first fold: lo + (hi<<32) - hi */
    u128 t1 = (u128)lo + ((u128)hi << 32) - (u128)hi;
    u64 lo1 = (u64)t1;
    u64 hi1 = (u64)(t1 >> 64);

    /* second fold: lo1 + (hi1<<32) - hi1 */
    u128 t2 = (u128)lo1 + ((u128)hi1 << 32) - (u128)hi1;
    u64 r   = (u64)t2;

    if (r >= MOD) r -= MOD;
    if (r >= MOD) r -= MOD;
    return r;
}

/* -------- mod exponent / inverse -------- */
static u64 pow_mod(u64 a, u64 e){
    u64 r = 1, base = a;
    while (e){
        if (e & 1u) r = mul_mod(r, base);
        base = mul_mod(base, base);
        e >>= 1;
    }
    return r;
}
static inline u64 inv_mod(u64 a){
    /* MOD is prime => a^(MOD-2) */
    return pow_mod(a, (u64)(MOD - 2ULL));
}

/* -------- in-place bit reversal permutation -------- */
static void bit_reverse(u64 *a, unsigned int n){
    unsigned int i = 1, j = 0;
    unsigned int bit;
    main_loop:
        if (i >= n) goto done;
        bit = n >> 1;
        inner:
            if (j & bit){
                j ^= bit;
            } else {
                j |= bit;
                goto inner_done;
            }
            bit >>= 1;
            goto inner;
        inner_done:
        if (i < j){
            u64 t = a[i]; a[i] = a[j]; a[j] = t;
        }
        i++;
        goto main_loop;
    done: ;
}

/* -------- iterative NTT (invert = 0 for forward, 1 for inverse) -------- */
static void ntt(u64 *a, unsigned int n, int invert){
    bit_reverse(a, n);

    unsigned int len = 2;
    outer:
        if (len > n) goto finished;

        u64 wlen = pow_mod(G, (u64)((MOD - 1ULL) / len));
        if (invert) wlen = inv_mod(wlen);

        unsigned int i = 0;
        block_loop:
            if (i >= n) goto next_len;
            u64 w = 1;
            unsigned int j = 0, half = (len >> 1);
            butterfly:
                if (j >= half) goto block_done;
                u64 u = a[i + j];
                u64 v = mul_mod(a[i + j + half], w);
                a[i + j]         = add_mod(u, v);
                a[i + j + half]  = sub_mod(u, v);
                w = mul_mod(w, wlen);
                j++;
                goto butterfly;
            block_done:
            i += len;
            goto block_loop;
        next_len:
        len <<= 1;
        goto outer;
    finished:

    if (invert){
        u64 inv_n = inv_mod((u64)n);
        unsigned int k = 0;
        scale:
            if (k >= n) goto scaledone;
            a[k] = mul_mod(a[k], inv_n);
            k++;
            goto scale;
        scaledone: ;
    }
}

/* -------- read a line of digits into SA/SB; return length -------- */
static int read_digits(char *buf, int maxn){
    int c, n = 0;
    /* skip non-digits */
    skip:
        c = getchar();
        if (c == EOF) goto end;
        if (c >= '0' && c <= '9') { goto got_first; }
        if (c == '\n' || c == '\r') goto skip;
        goto skip;
    got_first:
        buf[n++] = (char)c;
        more:
            c = getchar();
            if (c == EOF || c == '\n' || c == '\r') goto end;
            if (c >= '0' && c <= '9'){
                if (n < maxn - 1) { buf[n++] = (char)c; }
                goto more;
            }
            goto more;
    end:
        buf[n] = '\0';
        return n;
}

/* -------- carry propagation in base 10 with 64-bit ops -------- */
static int carry_to_base10(u64 *src, unsigned int n, unsigned char *out){
    /* src[i] holds nonnegative integers (convolution coefficients) */
    u128 carry = 0;
    unsigned int i = 0;
    prop:
        if (i >= n) goto after_main;
        carry += (u128)src[i];
        /* digit = carry % 10; carry /= 10; */
        unsigned int digit = (unsigned int)((u64)(carry % 10u));
        carry /= 10u;
        out[i] = (unsigned char)digit;
        i++;
        goto prop;
    after_main:
        while (carry){
            unsigned int digit = (unsigned int)((u64)(carry % 10u));
            carry /= 10u;
            out[i++] = (unsigned char)digit;
        }
        /* trim leading zeros */
        while (i > 1u && out[i-1] == 0u) i--;
        return (int)i;
}

/* -------- main -------- */
int main(void){
    /* 1) Read inputs */
    int la = read_digits(SA, MAXD);
    int lb = read_digits(SB, MAXD);

    /* Handle empty input gracefully */
    if (la <= 0) { puts("0"); return 0; }
    if (lb <= 0) { puts("0"); return 0; }

    /* 2) Special-case zero */
    int i = 0, allz = 1;
    checkA:
        if (i >= la) goto doneA;
        if (SA[i] != '0') allz = 0;
        i++; goto checkA;
    doneA:
    if (allz){ puts("0"); return 0; }
    i = 0; allz = 1;
    checkB:
        if (i >= lb) goto doneB;
        if (SB[i] != '0') allz = 0;
        i++; goto checkB;
    doneB:
    if (allz){ puts("0"); return 0; }

    /* 3) Prepare digit-wise arrays (least significant first) */
    unsigned int nA = (unsigned int)la, nB = (unsigned int)lb;
    unsigned int need = nA + nB;
    unsigned int N = next_pow2(need);

    /* zero-fill first N slots */
    unsigned int z = 0;
    zeroloop:
        if (z >= N) goto zero_done;
        A[z] = 0; B[z] = 0; z++; goto zeroloop;
    zero_done: ;

    /* reverse digits into A,B as 0..9 */
    unsigned int p = 0;
    fillA:
        if (p >= nA) goto fillA_done;
        A[p] = (u64)(SA[nA - 1 - p] - '0');
        p++; goto fillA;
    fillA_done:
    p = 0;
    fillB:
        if (p >= nB) goto fillB_done;
        B[p] = (u64)(SB[nB - 1 - p] - '0');
        p++; goto fillB;
    fillB_done: ;

    /* 4) NTT-based convolution */
    ntt(A, N, 0);
    ntt(B, N, 0);

    /* pointwise multiply */
    unsigned int k = 0;
    pw:
        if (k >= N) goto pw_done;
        A[k] = mul_mod(A[k], B[k]);
        k++; goto pw;
    pw_done:

    ntt(A, N, 1); /* inverse => integer coefficients (each < MOD) */

    /* 5) Carry to base-10 and print */
    /* We’ll reuse B[] as a scratch-free cast for printing */
    static unsigned char DIG[MAXN + 8];
    int used = carry_to_base10(A, N, DIG);

    /* print most significant to least */
    int idx = used - 1;
    print_loop:
        if (idx < 0) goto print_done;
        putchar((int)('0' + DIG[idx]));
        idx--; goto print_loop;
    print_done:
        putchar('\n');
    return 0;
}
