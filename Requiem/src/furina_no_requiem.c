#include <stdio.h>

/* -------- basic types -------- */
typedef unsigned int  u32;
typedef unsigned char u8;

/* -------- capacities (safe for 1000-digit operands) --------
   10^1000 ~ 2^3322.  We’ll store operands in 8192 bits (256 u32 words).
   The product fits in 16384 bits (512 u32 words).
   Decimal output <= 2000 digits; we allocate a bit extra.
*/
#define IN_WORDS   256      /* 256 * 32 = 8192 bits per operand */
#define RES_WORDS  (IN_WORDS*2) /* product buffer = 512 words */
#define DEC_DIGITS 2100     /* BCD digits for Double-Dabble */

/* -------- helpers: zero/copy/logic/shift -------- */
static void arr_zero_u32(u32 *a, int L){
    int i=0;
start_zero:
    if (i < L) { a[i] = 0u; i++; goto start_zero; }
}

static void arr_copy_u32(u32 *dst, const u32 *src, int L){
    int i=0;
start_copy:
    if (i < L) { dst[i] = src[i]; i++; goto start_copy; }
}

/* dst = a & b */
static void arr_and_u32(u32 *dst, const u32 *a, const u32 *b, int L){
    int i=0;
and_loop:
    if (i < L) { dst[i] = (a[i] & b[i]); i++; goto and_loop; }
}

/* dst = a ^ b */
static void arr_xor_u32(u32 *dst, const u32 *a, const u32 *b, int L){
    int i=0;
xor_loop:
    if (i < L) { dst[i] = (a[i] ^ b[i]); i++; goto xor_loop; }
}

/* dst = src << 1 (whole-array shift by one bit) */
static void arr_shl1_u32(const u32 *src, u32 *dst, int L){
    u32 carry = 0u;
    int i=0;
shl1_loop:
    if (i < L) {
        u32 s = src[i];
        dst[i] = (s << 1) | carry;
        carry = (s >> 31);
        i++;
        goto shl1_loop;
    }
}

/* return 1 if all words are zero, else 0 */
static int arr_is_zero_u32(const u32 *a, int L){
    int i=0;
check_loop:
    if (i < L) {
        if (a[i] != 0u) return 0;
        i++;
        goto check_loop;
    }
    return 1;
}

/* Get bit length (index of highest set bit + 1). 0 if zero. */
static int bit_length_u32(const u32 *a, int L){
    int i = L - 1;
find_top:
    if (i >= 0) {
        if (a[i] == 0u) { i--; goto find_top; }
        /* find position of MSB within a[i] */
        u32 x = a[i];
        int k = 0;
        /* count bits by shifting right */
        while (x) { k++; x >>= 1; }
        /* index = i*32 + k */
        return ((i << 5) | (k & 31));
    }
    return 0;
}

/* Get bit value at absolute bit index idx (0 = LSB), returns 0 or 1 */
static u32 get_bit_u32(const u32 *a, int L, int idx){
    (void)L; /* L unused here but kept for symmetry */
    int w = (idx >> 5);
    int b = (idx & 31);
    return (a[w] >> b) & 1u;
}

/* dst (Ldst) = src (Lsrc) << k  (k >= 0).  Safe if dst is larger. */
static void arr_shlK_u32(const u32 *src, int Lsrc, int k, u32 *dst, int Ldst){
    /* zero dst first */
    arr_zero_u32(dst, Ldst);

    int ws = (k >> 5);
    int bs = (k & 31);

    int j=0;
outer:
    if (j < Lsrc) {
        int di = j + ws;                /* base word index in dst */
        if (di < Ldst) {
            dst[di] |= (src[j] << bs);
        }
        if (bs && (di + 1) < Ldst) {
            dst[di + 1] |= (src[j] >> (32 - bs));
        }
        j++;
        goto outer;
    }
}

/* -------- bitwise ADD: A = A + B (in-place), using carry-walk
   Uses only &, ^, << and array ops. Needs two scratch buffers of length L. */
static void big_add_inplace(u32 *A, const u32 *B, int L, u32 *tmp, u32 *carry){
    /* tmp := B */
    arr_copy_u32(tmp, B, L);

add_round:
    if (!arr_is_zero_u32(tmp, L)) {
        /* carry = A & tmp */
        arr_and_u32(carry, A, tmp, L);
        /* A = A ^ tmp */
        arr_xor_u32(A, A, tmp, L);
        /* tmp = carry << 1 */
        arr_shl1_u32(carry, tmp, L);
        goto add_round;
    }
    /* done */
}

/* -------- decimal helpers (small 8-bit add, used in BCD adjust) -------- */
static u8 add8(u8 a, u8 b){
    /* bitwise adder for a+b (8-bit) */
    while (b) {
        u8 c = (u8)(a & b);
        a = (u8)(a ^ b);
        b = (u8)(c << 1);
    }
    return a;
}

/* -------- parse decimal string into big binary (array of u32 words) --------
   out := 0
   For each digit d: out = out*10 + d, with:
     out*10 = (out<<3) + (out<<1)
   All adds via big_add_inplace.
*/
static void big_from_decstr(u32 *out, int L, const char *s){
    u32 t1[IN_WORDS], t2[IN_WORDS], dsmall[IN_WORDS];
    u32 tmp[IN_WORDS], carry[IN_WORDS];

    arr_zero_u32(out, L);
    arr_zero_u32(t1,  L);
    arr_zero_u32(t2,  L);
    arr_zero_u32(dsmall, L);
    arr_zero_u32(tmp, L);
    arr_zero_u32(carry, L);

    int i = 0;
read_loop:
    if (s[i] != 0) {
        char c = s[i];
        if (c >= '0' && c <= '9') {
            /* t1 = out << 3; t2 = out << 1; out = t1 + t2 */
            arr_shlK_u32(out, L, 3, t1, L);
            arr_shlK_u32(out, L, 1, t2, L);
            arr_copy_u32(out, t1, L);
            big_add_inplace(out, t2, L, tmp, carry);

            /* add digit d (0..9) */
            arr_zero_u32(dsmall, L);
            dsmall[0] = (u32)(c - '0');  /* small literal; parsing step ok */
            big_add_inplace(out, dsmall, L, tmp, carry);
        }
        i++;
        goto read_loop;
    }
}

/* -------- big multiply: R = A * B via shift-and-add over bits -------- */
static void big_mul(const u32 *A, const u32 *B, u32 *R){
    u32 tmpShift[RES_WORDS];
    u32 tmpAdd [RES_WORDS];
    u32 tmpCarry[RES_WORDS];

    arr_zero_u32(R, RES_WORDS);
    arr_zero_u32(tmpShift, RES_WORDS);
    arr_zero_u32(tmpAdd, RES_WORDS);
    arr_zero_u32(tmpCarry, RES_WORDS);

    int nbits = bit_length_u32(B, IN_WORDS);
    int i = 0;
mul_loop:
    if (i < nbits) {
        if (get_bit_u32(B, IN_WORDS, i)) {
            /* tmpShift = (A << i) into RES_WORDS space, then R += tmpShift */
            arr_shlK_u32(A, IN_WORDS, i, tmpShift, RES_WORDS);
            big_add_inplace(R, tmpShift, RES_WORDS, tmpAdd, tmpCarry);
        }
        i++;
        goto mul_loop;
    }
}

/* -------- binary -> decimal using Double-Dabble (shift/add-3) --------
   Converts big binary R[RES_WORDS] to decimal digits bcd[0..used-1] (LSB first).
*/
static int bin_to_decimal_bcd(const u32 *R, u8 *bcd){
    /* clear BCD */
    int j=0;
clean_bcd:
    if (j < DEC_DIGITS) { bcd[j]=0u; j++; goto clean_bcd; }

    int used = 1;  /* at least one digit */
    int total_bits = bit_length_u32(R, RES_WORDS);

    if (total_bits == 0) {
        /* number is zero */
        bcd[0] = 0u;
        return 1;
    }

    int bit = total_bits - 1;
bit_loop:
    if (bit >= 0) {
        /* 1) For all used digits: if >=5 add 3 */
        int d=0;
adjust_loop:
        if (d < used) {
            if (bcd[d] >= 5u) {
                bcd[d] = add8(bcd[d], (u8)3);
            }
            d++;
            goto adjust_loop;
        }

        /* 2) Shift left by 1 across BCD, injecting current binary bit */
        u8 inject = (u8)get_bit_u32(R, RES_WORDS, bit);
        u8 carry = inject;
        int k=0;
shift_loop:
        if (k < used) {
            u8 t = (u8)(((u8)(bcd[k] << 1)) | carry);
            bcd[k] = (u8)(t & 0x0Fu);
            carry = (u8)((t >> 4) & 1u);
            k++;
            goto shift_loop;
        }
        if (carry) {
            if (used < DEC_DIGITS) {
                bcd[used] = carry; /* 0 or 1 */
                used++;
            }
        }

        bit--;
        goto bit_loop;
    }

    return used;
}

/* -------- print decimal from BCD (LSB-first digits) -------- */
static void print_bcd_decimal(const u8 *bcd, int used){
    /* find most significant non-zero digit */
    int i = used - 1;
    while (i > 0 && bcd[i] == 0u) { i--; }

    /* print from MS down to 0 */
print_loop:
    if (i >= 0) {
        printf("%c", (int)('0' + bcd[i]));
        i--;
        goto print_loop;
    }
    printf("\n");
}

/* -------- main -------- */
int main(void){
    char sa[1105], sb[1105]; /* allow ~1000-digit inputs comfortably */
    u32 A[IN_WORDS], B[IN_WORDS], R[RES_WORDS];
    u8  BCD[DEC_DIGITS];

    /* Read two lines/tokens as decimal strings */
    printf("Enter two non-negative integers (up to 1000 digits each):\n");
    if (scanf("%1100s", sa) != 1) return 0;
    if (scanf("%1100s", sb) != 1) return 0;

    /* Parse to big binary */
    big_from_decstr(A, IN_WORDS, sa);
    big_from_decstr(B, IN_WORDS, sb);

    /* Multiply via bitwise shift-and-add */
    big_mul(A, B, R);

    /* Convert to decimal and print */
    int used = bin_to_decimal_bcd(R, BCD);
    print_bcd_decimal(BCD, used);

    return 0;
}
