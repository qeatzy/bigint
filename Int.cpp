#ifndef INT_H_INCLUDED   // include guard, http://www.cplusplus.com/forum/articles/10627/#msg49679
#define INT_H_INCLUDED 
#include "utility.h"
// compared to Int.cpp, change kb_muladd and operator*= wrt memory allocation to minimize
// memory footprint.
// TODO fix Int::square()
// TODO divmod
// TODO float algorithm such as sqrt.

bool debug = 0;

// template <typename ValType, ValType base>
class Int {
    public:
    typedef int ValType;
    typedef int SumType;
    typedef long long ProdType;
    Int (ValType x) {
        this->m_sign = sgn(x);
        x *= this->m_sign;
        this->value = std::vector<ValType> {x};
        if(x > Int::BASE) {
            this->value[0] %= Int::BASE;
            this->value.push_back(x / Int::BASE);
        }
    }
    Int () : Int(0) {   // there is both +0 and -0, though +0 == -0.
    }
    Int (string x) {
        value = rebase(x);
    }
    Int (const Int& x) : value{x.value}, m_sign{x.m_sign} {} // TODO 
    // Int (vector<ValType> x) : value{x} {}   // TODO, disable this? since this should only be allowed for immediate implementer.
    // Int (vector<ValType> x, bool validity_check=true) : value{x} {  // TODO, default argument issue. http://stackoverflow.com/a/20771474/3625404
    //     if (validity_check) { rebase(value); }
    // }

    const vector<ValType> & data() const { return this->value; }
    size_t dlen() const { return this->value.size(); }
    int cmp_abs(const Int& obj) const;
    int compare_(const Int& obj) const { return this->cmp_abs(obj) * this->sign() * obj.sign(); }
    Int pow(int exponent);
    static Int pow(int base, int exponent);  // TODO, how to effectively forbid negative input?
    void negate() { this->m_sign *= -1; }
    Int& operator+=(const Int& obj);
    Int& operator-=(const Int& obj) { *this += -obj; return *this; }
    void operator*=(const Int& obj);
    void operator*=(const Int::ValType val);
    void square();
    const Int operator-() const { auto res = Int(*this); res.negate(); return res; }
    // bool operator<(const Int& obj) const { return compare_(obj) < 0; }
    // bool operator<=(const Int& obj) const { return compare_(obj) <= 0; }
    // bool operator==(const Int& obj) const { return compare_(obj) == 0; }
    // bool operator!=(const Int& obj) const { return compare_(obj) != 0; }
    // bool operator>(const Int& obj) const { return compare_(obj) > 0; }
    // bool operator>=(const Int& obj) const { return compare_(obj) >= 0; }
    // friend Int operator*(const Int& lhs, const Int& rhs);   // TODO, may overflow if lhs.value.size() is too large. -- how large? what is the possibility of overflow in real application?
    friend std::ostream& operator<<(std::ostream& os, const Int& obj);
    int sign() const { return this->m_sign; }

    private:
    static constexpr ValType BASE = 1000000000;     // BASE + BASE do not overflow ValType.
    static constexpr ValType kdigits = 9;
    static constexpr ValType VAL_MAX = INT_MAX;
    static constexpr ValType kb_cutoff = 16;
    vector<ValType> value;
    int m_sign;
    // ValType sign;
    static int plain_add(const Int::ValType *lhs, const Int::ValType *rhs, int n, Int::ValType *dest);
    static int plain_sub(const Int::ValType *lhs, const Int::ValType *rhs, int n, Int::ValType *dest);
    static void plain_muladd(const Int::ValType *lhs, int llen, const Int::ValType *rhs, int rlen, Int::ValType *dest);
    static int digit_muladd(Int::ValType val, const Int::ValType *rhs, int n, Int::ValType *dest);
    static void kb_muladd(const Int::ValType *lhs, const Int::ValType *rhs, int k_digit, Int::ValType *dest, Int::ValType *buf);
    static void kb_square(const Int::ValType *lhs, int n, Int::ValType *dest, Int::ValType *buf);
    static ValType digits_to_int (const char* p, const char* pend, int base) {
        // caution, **NO** error handling. 
        // assert(p < pend) // TODO, fix same place in 13.cpp, currently misspelled as 11.cpp.
        ValType res = 0;
        for(; p != pend; ++p) {
            res = res * base + (*p - '0');
        }
        return res;
    }
    vector<ValType> rebase(string num) {
        vector<ValType> res;
        auto pbegin = num.c_str();
        auto pend = pbegin + num.size();
        for (; (pbegin < pend) && isspace(*pbegin); ++pbegin) {}
        if (*pbegin == '-') {
            this->m_sign = -1;
            ++pbegin;
        } else if (*pbegin == '+') {
            this->m_sign = 1;
            ++pbegin;
        } else {
            this->m_sign = 1;
        }

        auto pleft = pend - kdigits;
        for (; pleft >= pbegin; pleft -= kdigits, pend -= kdigits) {
            res.push_back(digits_to_int(pleft, pend, 10));
        }
        if (pbegin < pend) { // TODO, fix same place in 13.cpp, currently misspelled as 11.cpp.
            res.push_back(digits_to_int(pbegin, pend, 10));
        }
        return res;
    }
    static void rebase(std::vector<Int::ValType> & value) {
        Int::ValType carry = 0;
        for (auto &x: value) {
            x += carry;
            carry = x / Int::BASE;
            x %= Int::BASE;
        }
        if (carry > 0) {
            value.push_back(carry);
        }
    }
    static vector<Int::ValType> rebase(std::vector<Int::ProdType> & value_prod) {
        // printf("value_prod: "); print(value_prod);
        vector<Int::ValType> res;
        res.reserve(value_prod.size());
        Int::ProdType carry = 0;
        for (auto &x: value_prod) {
            x += carry;
            carry = x / Int::BASE;
            x %= Int::BASE;
            res.push_back(x);
        }
        if (res.back() == 0) {
            res.pop_back();
        }
        // printf("res: "); print(res);
        // wait();
        return res;
    }

    void print(std::ostream& os) const {
        char *output = new char[Int::kdigits * this->value.size() + 1];   // allocate
        auto p = output;
        if (this->sign() < 0) {
            *p++ = '-';
        }
        auto it = this->value.rbegin();
        if (debug)
            p +=  sprintf(p, "%d ", *it);
        else
            p +=  sprintf(p, "%d", *it);
        for (++it; it != this->value.rend(); ++it) {
            if (debug)
                p += sprintf(p, "%09d ", *it);   // caution, Int::ValType is not guaranteed to be int and may not be 1e9, though unlikely.
            else
                p += sprintf(p, "%09d", *it);   // caution, Int::ValType is not guaranteed to be int and may not be 1e9, though unlikely.
        }
        // std::ofstream out("/home/Dell/code/qeatzy/project_euler/tmpc");
        // out << "y = " << output;
        os << "dlen() = " << this->dlen() << ": ";
        os << output << '\n';
        delete[] output;
    }
};

int Int::cmp_abs(const Int& obj) const {
    auto llen = this->dlen(), rlen = obj.dlen();
    int res = sgn(llen > rlen);
    if (res==0) {
        auto rhs = obj.value.rbegin();
        decltype(rhs) lhs = this->value.rbegin();
        for (; (lhs != this->value.rend()) && (*lhs == *rhs); ++lhs, ++rhs) { }
        res = sgn(*lhs - *rhs);
        // printf("*lhs = %d, *rhs = %d\n", *lhs, *rhs);
    }
    // cout << " res = " << res << '\n';
    return res;
}

Int& Int::operator+=(const Int& obj) {
    // TODO handle subtraction with complement, http://www.sonoma.edu/users/w/wilsonst/courses/math_300/groupwork/altsub/3d.html
    // http://www.cplusplus.com/forum/lounge/32041/2/#msg173890
    int m = this->value.size(), n = obj.value.size();
    bool signdiff = (this->sign() * obj.sign()) == -1;
    bool this_shorter = m < n;
    bool swap_operand = false;
    // cout << signdiff << this_shorter << '\n';
    // wait("+=");
    if (signdiff && (this_shorter || (this->cmp_abs(obj) < 0))) {
        this->m_sign = obj.m_sign;
        swap_operand = true;
        // wait();
    }
    if (this_shorter) {
        this->value.reserve(1 + obj.value.size());
        this->value.insert(this->value.end(), obj.value.begin() + m, obj.value.end());
        std::swap(m, n);
    }
    // cout << *this;
    const Int::ValType *lhs = &(this->value[0]), *rhs = &(obj.value[0]);
    Int::ValType *dest = &(this->value[0]), *dest_end = dest + this->value.size();
    int carry = 0;
    if (signdiff) { // subtraction
        if (swap_operand) {
            std::swap(lhs, rhs);
        }
        carry = plain_sub(lhs, rhs, n, dest);
        if (carry) {
            // wait("update carry");
            dest += n;
            for(; (dest < dest_end) && *dest == 0; ++dest) {
                *dest = Int::BASE - 1;
            }
            *dest += carry; // since we subtract smaller from larger, here carry cannot propagate upwards.
            // --(*dest);
        }
        // wait("pre-normalize");
        auto tmp_size = this->value.size();
        for (auto p = this->value.rbegin(); (p+1 < this->value.rend()) && (*p == 0); ++p) {
            // wait("normalize");
            --tmp_size;
        }
        this->value.resize(tmp_size);
    } else {    // addition
        carry = plain_add(lhs, rhs, n, dest);
        if (carry) {
            dest += n;
            for(; (dest < dest_end) && *dest == Int::BASE - 1; ++dest) {
                *dest = 0;
            }
            if (dest == dest_end) {
                this->value.push_back(carry);
            } else {
                *dest += carry;
                // ++(*dest);
            }
        }
    }
    return *this;
}

int Int::plain_add(const Int::ValType *lhs, const Int::ValType *rhs, int n, Int::ValType *dest) {
    // dest = lhs + rhs
    // dest could be either lhs or rhs
    // return carry (0 or 1).
    Int::SumType carry = 0;
    for (auto lhs_end = lhs + n; lhs < lhs_end; ++lhs, ++rhs, ++dest) {
        carry += *lhs + *rhs;
        *dest = carry % Int::BASE;
        carry /= Int::BASE;
    }
    // wait("add");
    return carry;
}

int Int::plain_sub(const Int::ValType *lhs, const Int::ValType *rhs, int n, Int::ValType *dest) {
    // dest = lhs - rhs
    // dest could be either lhs or rhs
    // return carry (0 or -1).
    Int::SumType carry = 0;
    for (auto lhs_end = lhs + n; lhs < lhs_end; ++lhs, ++rhs, ++dest) {
        carry += *lhs - *rhs;
        carry += Int::BASE;
        *dest = carry % Int::BASE;
        carry = carry / Int::BASE - 1;
    }
    // cout << "carry = " << carry << '\n';
    // wait("sub");
    return carry;
}

void Int::plain_muladd(const Int::ValType *lhs, int llen, const Int::ValType *rhs, int rlen, Int::ValType *dest) {
    // dest = dest + (lhs * rhs);
    // carry is added ahead, instead of being returned. thus dest should be at least size 1 + llen + rlen.
    // try this, dest = 5432, lhs = 98, rhs = 76, Int::ValType [0,9], Int::ProdType [0,99].
    // print(lhs, llen);
    // print(rhs, rlen);
    // print(dest, llen + rlen + 1);
    // print("input check");
    // auto echo = [=]() { print(dest, llen + rlen + 1); };
    Int::ProdType prod = 0;
    auto p = dest;
    auto lhs_end = lhs + llen, rhs_begin = rhs, rhs_end = rhs + rlen;
    for(; lhs < lhs_end; ++lhs) {
        // cout << "*lhs = " << *lhs << '\n';
        p = dest++;
        prod = 0;
        // cout << "rhs_end - rhs = " << rhs_end - rhs << '\n';
        for(rhs = rhs_begin; rhs < rhs_end; ++rhs) {
            prod += *p;   // redundant if dest == 0.
            prod += (Int::ProdType)(*lhs) * (*rhs);
            *p = prod % Int::BASE;
            // echo(); wait("check dest inner loop");
            prod /= Int::BASE;
            ++p;
        }
        prod += *p;
        *p = prod % Int::BASE;
        // echo(); wait("check dest outer loop 1");
        prod /= Int::BASE;
        p[1] += prod;
        // echo(); wait("check dest outer loop 2");
        // if (prod) p[1] += prod;   // add carry
    }
    // int carry = *p / Int::BASE;   // the most significant digit
    // *p %= Int::BASE;
    // p[1] += carry;
    // return carry;
}

int Int::digit_muladd(Int::ValType val, const Int::ValType *rhs, int n, Int::ValType *dest) {
    Int::ProdType prod = 0;
    for (auto rhs_end = rhs + n; rhs < rhs_end; ++rhs) {
        prod += *dest;
        prod += (Int::ProdType)val * (*rhs);
        *dest = prod % Int::BASE;
        prod /= Int::BASE;
        ++dest;
    }
    // caught by this update carry portion, here choose to return (which make client code
    // tend to off by one error), whereas add directly in plain_muladd, but ask for extra
    // high digit (though not hard).
    prod += *dest;
    *dest = prod % Int::BASE;
    prod /= Int::BASE;
    return (int)prod;
}

void Int::kb_muladd(const Int::ValType *lhs, const Int::ValType *rhs, int n, Int::ValType *dest, Int::ValType *buf) {
    // buf must be set to zero before this function is called, and size no less than 3*n+1.
    // reduce buf to of size equal dest, as GMP does. and allocate only once.
    // https://en.wikipedia.org/wiki/Karatsuba_algorithm
    // also check how GMP implemented. https://gmplib.org/devel/
    // dest = dest + lhs * rhs.  (spurious) carry is added ahead, instead of being returned.
    // at top level, dest might be nonzero, otherwise, dest is zero.
    int carry;
    if (n < kb_cutoff) {
        plain_muladd(lhs, n, rhs, n, dest);
        return;
    }
    // cout << "n = " << n << '\n';
    bool is_odd = (n%2) != 0;
    auto sz = n/2;
    n -= is_odd;
    auto shifted_dest = dest + n;

    // pre recursion, prepare work.
    auto buf_delegate = buf + n;
    
    kb_muladd(lhs, rhs, sz, buf, buf_delegate);   // lower part
    carry = plain_add(dest, buf, n, dest);
    dest[n] += carry;
    carry = plain_add(dest + sz, buf, n, dest + sz);
    dest[sz + n] += carry;

    memset(buf, 0, (3*n+1) * sizeof(Int::ValType));
    kb_muladd(lhs + sz, rhs + sz, sz, buf, buf_delegate);   // high part
    carry = plain_add(dest + sz, buf, n, dest + sz);
    dest[sz + n] += carry;
    carry = plain_add(dest + n, buf, n, dest + n);
    dest[n + n] += carry;

    memset(buf, 0, (3*n+1) * sizeof(Int::ValType));
    int sign = -1;  // do subtraction if -1, addition if 1, after pmid been calculated.
    auto mid_left = buf + 3*n + 1, mid_right = mid_left + sz;
    carry = plain_sub(lhs, lhs+sz, sz, mid_left);
    if (carry) {
        sign *= -1;
        plain_sub(lhs+sz, lhs, sz, mid_left);
        // print(mid_left, sz);
        // std::for_each(mid_left, mid_left+sz, [](Int::ValType &x) { x = (Int::BASE - 1) - x; });
        // print(mid_left, sz);
        // wait("complement");
    }
    carry = plain_sub(rhs, rhs+sz, sz, mid_right);
    if (carry) {
        sign *= -1;
        plain_sub(rhs+sz, rhs, sz, mid_right);
        // std::for_each(mid_right, mid_right+sz, [](Int::ValType &x) { x = (Int::BASE - 1) - x; });
    }
    kb_muladd(mid_left, mid_right, sz, buf, buf_delegate);
    carry = (sign == -1) ? plain_sub(dest + sz, buf, n, dest + sz) : plain_add(dest + sz, buf, n, dest + sz);
    dest[sz + n] += carry;

    // post recursion, cleaning work (if needed).

    if (is_odd) {
        carry = digit_muladd(lhs[n], rhs, n+1, shifted_dest);
        shifted_dest[1+n+1] += carry;
        carry = digit_muladd(rhs[n], lhs, n, shifted_dest);
        shifted_dest[1+n] += carry;
    }
}

void Int::operator*=(const Int& obj) {
    // http://www.bodrato.it/software/toom.html      https://gmplib.org/devel/
    // if (&obj == this) {
    //     this->square();
    //     return;
    // }
    this->m_sign *= obj.m_sign;
    const Int::ValType *lhs = &(this->value[0]), *rhs = &(obj.value[0]);
    int m = this->value.size(), n = obj.value.size();
    if (m<n) {
        std::swap(lhs, rhs);
        std::swap(m, n);
    }
    // auto product = std::vector<Int::ValType>(m+n);
    auto product = std::vector<Int::ValType>(m+n+1);   // extra one to allow spurious carry be add directly.
    auto dest = &(product[0]);
    if (n < Int::kb_cutoff) {
        // wait("*= plain_muladd, pre");
        plain_muladd(lhs, m, rhs, n, dest);
    } else {
        auto lhs_end = lhs + m;
        auto buf = new Int::ValType[4*n + 1]; // allocate 
        for(; lhs + n <= lhs_end; lhs += n) {   // caught by this error: equality was omitted. /for[^;]*;[^;+]*+
            // caught by memset error, forget that the third argument is in byte, thus need sizeof always, since memset has no info of type, thus must be some raw size.
            memset(buf, 0, (4*n + 1) * sizeof(Int::ValType));    // initialize to zero before every invocation of kb_muladd.
            kb_muladd(lhs, rhs, n, dest, buf);
            dest += n;
        }
        delete[] buf;  // deallocate
        // cout << " remnant of *= is " << lhs_end - lhs << ", ";
            // printf("m = %d, n = %d\n", m,n);
        if (lhs < lhs_end) {
            plain_muladd(lhs, lhs_end - lhs, rhs, n, dest);
        }
    }
    product.pop_back();     // remove extra one
    // for (auto it = product.rbegin(); (it < product.rend()) && (*it == 0); ++it) {   // '<' instead of '!=', there is at least one slot
    for (auto it = product.rbegin(); (it+1 < product.rend()) && (*it == 0); ++it) {   // 'it+1 <' instead of 'it !=', there is at least one slot
        // wait("normalize");
        product.pop_back();
    }
    this->value = product;
}

void Int::square() {
    int n = this->value.size();
    auto lhs = &(this->value[0]);
    auto rhs = lhs;
    std::vector<Int::ValType> product(2*n+1);
    auto dest = &(product[0]);
    if (n < Int::kb_cutoff) {
        plain_muladd(lhs, n, rhs, n, dest);
    }

    auto buf = new Int::ValType[3*n+1](); // allocate and initialize
    kb_square(lhs, n, dest, buf);
    delete[] buf;

    product.pop_back();     // remove extra one
    for (auto it = product.rbegin(); (it+1 < product.rend()) && (*it == 0); ++it) {   // 'it+1 <' instead of 'it !=', there is at least one slot
        // wait("normalize");
        product.pop_back();
    }
    this->value = product;
}

void Int::kb_square(const Int::ValType *lhs, int n, Int::ValType *dest, Int::ValType *buf) {
    // buf size must be at least 2*n+1 array.
    auto rhs = lhs;
    int carry;
    if (n < kb_cutoff) {
        plain_muladd(lhs, n, rhs, n, dest);
        return;
    }
    bool is_odd = (n%2) != 0;
    auto sz = n/2;
    n -= is_odd;
    auto shifted_dest = dest + n;

    // pre recursion, prepare work.
    auto buf_delegate = buf + n;
    
    kb_square(lhs, sz, buf, buf_delegate);   // lower part
    carry = plain_add(dest, buf, n, dest);
    dest[n] += carry;

    memset(buf, 0, (6*sz+1) * sizeof(Int::ValType));
    kb_square(lhs + sz, sz, buf, buf_delegate);   // high part
    carry = plain_add(dest + n, buf, n, dest + n);
    dest[n + n] += carry;

    memset(buf, 0, (6*sz+1) * sizeof(Int::ValType));
    kb_muladd(lhs, lhs + sz, sz, buf, buf_delegate);
    carry = plain_add(dest + sz, buf, n, dest + sz);
    carry += plain_add(dest + sz, buf, n, dest + sz);
    dest[sz + n] += carry;

    // post recursion, cleaning work (if needed).

    if (is_odd) {
        carry = digit_muladd(lhs[n], rhs, n+1, shifted_dest);
        shifted_dest[1+n+1] += carry;
        carry = digit_muladd(rhs[n], lhs, n, shifted_dest);
        shifted_dest[1+n] += carry;
    }
}

// void Int::kb_square(const Int::ValType *lhs, int n, Int::ValType *dest, Int::ValType *buf) {
//     // buf size must be at least 2*n+1 array.
//     auto rhs = lhs;
//     int carry;
//     if (n < kb_cutoff) {
//         plain_muladd(lhs, n, rhs, n, dest);
//         return;
//     }
//     bool is_odd = (n%2) != 0;
//     auto sz = n/2;
//     n -= is_odd;
//     auto shifted_dest = dest + n;
//
//     // pre recursion, prepare work.
//     auto buf_delegate = buf + n;
//     
//     kb_square(lhs, sz, buf, buf_delegate);   // lower part
//     carry = plain_add(dest, buf, n, dest);
//     dest[n] += carry;
//     carry = plain_add(dest + sz, buf, n, dest + sz);
//     dest[sz + n] += carry;
//
//     memset(buf, 0, (5*sz+1) * sizeof(Int::ValType));
//     kb_square(lhs + sz, sz, buf, buf_delegate);   // high part
//     carry = plain_add(dest + sz, buf, n, dest + sz);
//     dest[sz + n] += carry;
//     carry = plain_add(dest + n, buf, n, dest + n);
//     dest[n + n] += carry;
//
//     memset(buf, 0, (5*sz+1) * sizeof(Int::ValType));
//     auto mid_left = buf + 5*sz + 1;
//     carry = plain_sub(lhs, lhs+sz, sz, mid_left);
//     if (carry) {
//         plain_sub(lhs+sz, lhs, sz, mid_left);
//     }
//     kb_square(mid_left, sz, buf, buf_delegate);
//     carry = plain_add(dest + sz, buf, n, dest + sz);
//     dest[sz + n] += carry;
//
//     // post recursion, cleaning work (if needed).
//
//     if (is_odd) {
//         carry = digit_muladd(lhs[n], rhs, n+1, shifted_dest);
//         shifted_dest[1+n+1] += carry;
//         carry = digit_muladd(rhs[n], lhs, n, shifted_dest);
//         shifted_dest[1+n] += carry;
//     }
// }

void Int::operator*=(const Int::ValType val) {
    auto p = this->value.begin();
    Int::ProdType prod = 0;  // as both carry and prod
    for(; p < this->value.end(); ++p) {
        prod += (Int::ProdType) p[0] * val;
        p[0] = prod % Int::BASE;
        prod /= Int::BASE;
    }
    if (prod > 0) {
        this->value.push_back(prod);
    }
    // return (*this);
}

Int Int::pow(int exponent) {
    if (exponent < 0) {
        // TODO 
    } else if (exponent == 0) {
        this->value.resize(1);
        this->value[0] = 1;
    } else { // positive exponent
        auto res = Int(1);
        int flag = 1<<30;
        // wait("pow");
        for(; (flag & exponent) == 0; flag >>= 1) {}
        auto nbase = 0;
        for(; flag > 0; flag >>= 1) {
            res *= res;
            nbase *= 2;
            // cout << " square, nbase = " << nbase << ",  " << res;
            // wait();
            if (flag & exponent) {
                res *= *this;
                nbase += 1;
                // cout << " addition, nbase = " << nbase << ",  " << res;
            }
        }
        return res;
    }
    return Int(*this);
}

Int Int::pow(int base, int exponent) {
    // TODO move most part to Int::pow, and make this non member, just as operator+.
    if (base == 0) return Int(0);
    if (exponent > 0) {
        int sign = 1;
        if (base < 0) {
            base *= -1;
            sign = ((exponent % 2) == 0) ? 1 : -1;
        }
        double tmp_pow = std::pow(base, exponent);
        if (tmp_pow < Int::BASE) {
            tmp_pow *= sign;
            auto res = Int((Int::ValType) tmp_pow);
            return res;
        }

        auto res = Int(1);
        Int::ValType new_base = base, k_base = 1;
        for(; (Int::ProdType)new_base * base < Int::BASE; new_base *= base) {
            ++k_base;
        }
        auto remnant = (Int::ValType)std::pow(base, exponent % k_base);
        // printf("base = %d, exponent = %d\n", base, exponent);
        exponent /= k_base;
        // printf("new_base = %d, exponent = %d, k_base = %d\n", new_base, exponent, k_base);
        auto base_Int = Int(new_base);
        int flag = 1<<30;
        // wait("pow");
        for(; (flag & exponent) == 0; flag >>= 1) {}
        auto nbase = 0;
        for(; flag > 0; flag >>= 1) {
            res *= res;
            nbase *= 2;
            // cout << " square, nbase = " << nbase << ",  " << res;
            // wait();
            if (flag & exponent) {
                res *= new_base;
                nbase += 1;
            // cout << " addition, nbase = " << nbase << ",  " << res;
            }
        }
        res *= remnant;
        if (sign == -1) res.negate();
        return res;
    } else if (exponent < 0) {
    }
    return Int(-1);
}

std::ostream& operator<<(std::ostream& os, const Int& obj) {
    obj.print(os);
    return os;
}

inline const Int operator+(Int lhs, const Int& rhs) {   // return const to avoid code such as a+b=c. http://www.gotw.ca/gotw/004.htm
    lhs += rhs;     // left operand is passed by value. http://stackoverflow.com/questions/4421706/operator-overloading/4421719#4421719
    return lhs;
}

inline const Int operator-(Int lhs, const Int& rhs) {
    lhs -= rhs;
    return lhs;
}

const Int operator*(const Int& lhs, const Int& rhs) {
    // b := Int:BASE, m = lhs.value.size(), n = rhs.value.size(), 
    // lhs = x0, ..., x{m-1}
    // rhs = y0, ..., y{n-1}
    // assert m>n
    // for k in [0,n), sum((y{k} * lhs) << k) == result.
    // TODO, should be parallel or not, that is possibly parallel sum a collection of
    // vectors vs only use two vectors and just add the newly computed to result
    // candidate, or put it another way, which way should we choose, in the tradeoff of
    // parallel but more allocation vs less allocation but no parallel. -- note, their is
    // two step of aggregation, what we are talking is the second, the first one could always be parallelized.
    // FFT? Strassen’s Matrix Multiplication? could be applied if m != n?
    // http://www.geeksforgeeks.org/multiply-two-polynomials-2/
    // http://www.cs.cmu.edu/afs/cs/academic/class/15451-s10/www/lectures/lect0423.txt
    // The idea of using 3 multiplications instead of 4 is used in large-integer multiplications. A similar idea is the basis of the classic Str assen matr ix multiplication algor ithm (CLRS, Chapter 28).  http://www.cse.ust.hk/~dekai/271/notes/L03/L03.pdf
    // search for "large-integer multiplications"
    // my idea: prod as 2 digit(on base) sum, thus if ValType be int, ProdType be llong,
    // thus can handle pow(Int::BASE, Int::BASE) product without ever consider carry, thus
    // could be easily parallelized. further more, locally, faster algorithm such as
    // Karatsuba algorithm could be used, but under this context, is the boost obvious?
    auto res = Int(lhs);
    res *= rhs;
    return res;
}

bool operator<(const Int& lhs, const Int& rhs) { return lhs.compare_(rhs) < 0; }
bool operator<=(const Int& lhs, const Int& rhs) { return lhs.compare_(rhs) <= 0; }
bool operator==(const Int& lhs, const Int& rhs) { return lhs.compare_(rhs) == 0; }
bool operator!=(const Int& lhs, const Int& rhs) { return lhs.compare_(rhs) != 0; }
bool operator>(const Int& lhs, const Int& rhs) { return lhs.compare_(rhs) > 0; }
bool operator>=(const Int& lhs, const Int& rhs) { return lhs.compare_(rhs) >= 0; }

int main() {
    // TODO change to a static method of Int, then just call Int::main() to test. and make this file header only.
    cout << std::boolalpha;
    auto a = Int(), b = Int(-2123456789), c = Int("-999999999549123491234912349123491234912349123491234912349123499994912349"), d=c;
    cout << "c = " << c;
    // cout << a << b << c << d << c+d << (c*d) * (c*d);
    // cout << a << b << c << c*c;
    // cout << a-b << b-a;
    // auto tmp = Int::pow(2,42*29);
    // auto tmp = Int::pow(2,21*29);
    // auto tmp = Int("2124551971267068394758352826209874509318372470908127692797776552801614239443408970956650009060917142675557317944986004061386317350610828957638079915066349407775325083341572876126912512"); // 2**(21*29)
    // cout << " tmp: " << tmp;
    // tmp *= tmp;
    // cout << " tmp: " << tmp;
    // auto f42 = Int("4513721078614786210022654658754022076207575792470901370842837631998555767764876163651334186529206088049982418198714538469250807179723842787735400581098036879598420878073788073478633526793353330401336902135069154012389883318957655774958078857324256712388820015707438180916154489315708470767711885060380524427604580826158560154020090008127121505888111464555809702150144"); // 2**(42*29)
    // cout << " f42: " << f42;
    // cout << "\ntmp.cmp_abs(f42) = " << tmp.cmp_abs(f42) << '\n';
    // auto diff = f42 - f42;
    // cout << diff;
    // cout << (a == diff) << (a < diff) << (a > diff);
    // cout << Int::pow(2,10000);
    // tmp = 2**(29*21) is right, whereas tmp * tmp get wrong
    // auto f = [](int base, int exponent) { cout << std::pow(base, exponent) << '\n'; };
    // f(2,10);
    // f(2,100000);
    // f(INT_MAX, INT_MAX);
    auto start = std::chrono::high_resolution_clock::now();
    auto tmp = Int::pow(11,1000000);
    // auto tmp = Int("2423285551989543969259886147306320615721694717012975552426444448158985017722789267546553034738712987127346362442309271495645764807314487385596126924659433020959638410571315406303196994043324038030803068668509323897700215957022383545283810557899591580902307500436706661372076856211818662627186819885605667216486349283517459646495626188985295134800963771933733229691796170211328");
    // cout << " tmp: " << tmp;
    // tmp *= tmp;
    cout << " tmp: " << tmp;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end-start;
    std::cout << diff.count() << " s\n";
}


#endif
