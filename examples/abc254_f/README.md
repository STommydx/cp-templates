# ABC 254F: Rectangle GCD

- Contest: [AtCoder ABC 254](https://atcoder.jp/contests/abc254)
- Problem: [F - Rectangle GCD](https://atcoder.jp/contests/abc254/tasks/abc254_f)

## Solution

Observe that `gcd(a, b) = gcd(b, a - b)`. Therefore, we can compute the adjacent difference matrix and use sparse table for range GCD queries.
