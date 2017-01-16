#ifndef UTILITY_H_INCLUDED   // include guard, http://www.cplusplus.com/forum/articles/10627/#msg49679
#define UTILITY_H_INCLUDED 

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <deque>
#include <string>
#include <cstring>  // strlen, string literal of type const char [].
#include <algorithm>
#include <numeric>
#include <cmath>    // pow
#include <valarray>
#include <array>
#include <iterator> // std::iterator_traits<Iterator>::value_type  difference_type  iterator_category  http://stackoverflow.com/a/1107235/3625404
#include <utility>
#include <stdexcept>
#include <cassert>  // assert
#include <climits>
#include <cstdlib>  // strtoul, strtoull, strtol
#include <cctype>   // isspace
#include <ctime>
#include <chrono>
#include <regex>
#include <memory>   // smart pointers

typedef std::vector<int> iVec;
typedef std::vector<long long> llVec;
typedef std::vector<double> dVec;
typedef std::vector<std::string> sVec;
typedef long long llong;

using std::cerr;
using std::cout;
using std::cin;
using std::ios;
using std::ios_base;
char endl = '\n';
using std::string;
using std::vector;
using std::array;

    // auto start = std::chrono::high_resolution_clock::now();
    // auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> diff = end-start;
    // std::cout << diff.count() << " s\n";

void wait() {
    char c; cin.get(c); if (c=='q') exit(1);
}
void wait(string s) {
    cout << s << ' ';
    char c; cin.get(c); if (c=='q') exit(1);
}

inline bool isneg(int x) { return x < 0; }
inline int sgn(int x) { return (x>0) - (x<0); }
// template <typename T> int sgn(T val) {  // http://stackoverflow.com/a/4609795/3625404
//     return (T(0) < val) - (val < T(0));
// }

template <typename Iterator, typename T = typename std::iterator_traits<Iterator>::value_type>
void print(Iterator first, Iterator last, std::string description="", signed char sep=' ') { // char can be either signed or unsigned.
    if (description != "") {
        cout << description << ": ";
        if (sep == '\n') cout << '\n';
    }
    const bool wait_for_inspect = (sep == 'q');
    auto ending_mark = '\n';
    if (sep < 0) {
        sep = ' ';
        if (sep < -1) ending_mark = ' ';
    } else if (!(sep == 0 || sep == ' ' || sep == '\n' || sep == '\t')) {
        sep = ' ';
        if (decltype(last - first)(sep) < last - first) { last -= sep; }
    }
    for(auto it = first; it != last; ++it) {
        std::cout << *it;
        if (wait_for_inspect) {
            char c;
            std::cin.get(c);
            if (c == 'q') {
                break;
            }
        } else {
            std::cout << sep;
        }
    }
    std::cout << ending_mark;
}

template <typename Container>
void print(const Container &vec, char sep=' ', std::string description="") {
    print(std::begin(vec), std::end(vec), description, sep);
}

template <typename It>
void print(It b, int n, char sep = ' ', std::string description="") {
    assert(n >= 0);
    print(b, b + n, description, ' ');
}

    // // example for print (with caveat.)
    // print(v);   // container
    // print(v, '\t');
    // print(v, 6, ""); // ambiguous if leave empty string out.
    // print(v.begin(), 6);
    // print(v.begin(), v.end(), "second", 6);
    // print(v.begin(), v.end(), "separated with newline", '\n');

// void print(const int *p, int n) {
//     for (auto i = 0; i < n; ++i) printf("%9d, ", p[i]); putchar('\n');
// }

/**
template <>     // template specialization, note the <> is needed.
void print<double>(const std::vector<double> & vec) {
    for(auto x: vec) {
        // printf("%8.4f ", x);
        cout << x << ' ';
    }
    // for(auto x: vec) { printf("%6.2f ", x); }
    putchar('\n');
}
*/

template <typename T>
void print(const vector<vector<T>> &mat) {
    for (auto &vec: mat) print(vec);
}

iVec range(int start, int stop, int step) {
    /* generate a sequence.
     * step must **NOT** be zero.
     * TODO: refactor to generic to allow double parameters. Or restrict range to int, use
     * linspace for double instead?
     */
    // static_assert(step != 0, "step must **NOT** be zero.");
    auto cmp = (step>0) ? [](int left, int right) { return left < right; }
                        : [](int left, int right) { return left > right; };
    iVec res;
    while(cmp(start, stop)) {
        res.push_back(start);
        start += step;
    }
    return res;
}
iVec range(int start) {
    return range(0, start, 1);
}
iVec range(int start, int stop) {
    return range(start, stop, 1);
}

// dVec range(double start, double stop, double step) {
//     dVec vec;
//     if (step == 0) {
//         throw std::runtime_error("step must be non-zero.");
//     } else if (step > 0) {
//         while (start < stop) {
//             vec.push_back(start);
//             start += step;
//         }
//     } else {
//         while (start > stop) {
//             vec.push_back(start);
//             start += step;
//         }
//     }
//     return vec;
// }
// std::vector<double> range(double start, double stop) {
//     return range(start, stop, 1);
// }
// std::vector<double> range(double stop) {
//     return range(0, stop, 1);
// }

bool isPrime(int n) { 
    if (n <= 1) return false;
    if (n <= 3) return true;
    if ((n % 2 == 0) || (n % 3 == 0)) return false;
    for (int i = 5; i * i < n; i += 6) {
        if ((n % i == 0) || (n % (i + 2) == 0)) return false;
    }
    return true;
}

class Prime {
    public:
        Prime()
            : primes {2,3,5,7,11},
            threshold {11}
        { }
        Prime(int threshold) : Prime() {
            if (threshold > this->threshold)
                this->update(threshold);
        }
        iVec operator()(int threshold) {
            if (threshold > this->threshold) {
                this->update(threshold);
            }
            // TODO(efficiency): is this copy approach good enough, could a non-writable view be returned instead?
            // NOT. since this->primes, could be invalidated, returning pointer/reference
            // may result dangling reference. Which do not change for const reference.
            return iVec(primes.begin(), std::upper_bound(primes.begin(), primes.end(), threshold));
        }
        int nth_prime(int n) {
            auto threshold = n;
            while ((size_t)n > primes.size()) {
                if (threshold > this->threshold) {
                    this->update(threshold);
                }
                else {
                    threshold *= 2;
                    // cout << "will update. current size is " << primes.size() << ", threshold is " << threshold << '\n';
                }
            }
            // printf("primes.size() = %zu\n", primes.size());
            return primes[n-1];
        }
        const iVec getall() { return this->primes; }
        iVec::const_iterator get_pos(int threshold) {
            cout << "this->threshold = " << this->threshold << ", threshold = " << threshold << ", primes.size() = " << primes.size() << '\n';
            if (threshold > this->threshold) {
                this->update(threshold);
            }
            return std::upper_bound(primes.cbegin(), primes.cend(), threshold);
        }
    private:
        iVec primes;
        int threshold;
        void update(int threshold) {
            if (primes.capacity() < (size_t) threshold) {
                this->primes.reserve(1.25506 * threshold / log(threshold) );
                printf("reserve size %f\n", 1.25506 * threshold / log(threshold) );
            }
            // cin.get();
            // printf("debug_num = %d\n", ++debug_num);
            auto pos = this->get_pos(sqrt(threshold));
            // cout << "threshold = " << threshold << ", pos is nth: " << pos - primes.cbegin() << '\n';
            auto is_prime = [this, pos](int val) {
                for (auto it = primes.cbegin(); it != pos; ++it) {
                    if (val % (*it) == 0)
                        return false;
                }
                return true;
            };
            auto candidate = 1 + this->threshold;
            candidate += (candidate + 1) % 2;
            for(; candidate <= threshold; candidate += 2) {
                if (is_prime(candidate))
                    this->primes.push_back(candidate);
            }
            this->threshold = threshold;
            cout << "this->threshold = " << this->threshold << ", threshold = " << threshold << ", primes.size() = " << primes.size() << '\n';
        }
};

#endif
// below are vocabulary that aid vim completion.
// extensibility
/**
    out_of_range  overflow_error
    throw std::invalid_argument( "received negative value" );
    catch(const std::invalid_argument& e) {   // And you should always catch exceptions as const? comment in http://stackoverflow.com/a/8480675/3625404
**/



