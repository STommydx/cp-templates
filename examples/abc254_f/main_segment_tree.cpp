#include "../../commons.hpp"
#include "../../functional.hpp"
#include "../../segment_tree.hpp"

#include <cmath>
#include <iostream>
#include <numeric>

using namespace std;

int no_of_test_cases = 1;

int solve() {
	int n, q;
	cin >> n >> q;
	vll a(n), b(n);
	cin >> a >> b;
	vll ad(n), bd(n);
	adjacent_difference(a.begin(), a.end(), ad.begin());
	adjacent_difference(b.begin(), b.end(), bd.begin());
	for (ll &x : ad)
		x = abs(x);
	for (ll &x : bd)
		x = abs(x);
	segment_tree<ll, ll, fn::gcd<>> sta(ad), stb(bd);
	while (q--) {
		int x1, y1, x2, y2;
		cin >> x1 >> x2 >> y1 >> y2;
		x1--, y1--, x2--, y2--;
		ll ans = gcd(gcd(a[x1] + b[y1], a[x1] + b[y2]),
		             gcd(a[x2] + b[y1], a[x2] + b[y2]));
		if (x1 < x2) {
			ans = gcd(ans, sta.query(x1 + 1, x2));
		}
		if (y1 < y2) {
			ans = gcd(ans, stb.query(y1 + 1, y2));
		}
		cout << ans << '\n';
	}
	return 0;
}

int main() {
	ios::sync_with_stdio(false);
	if (no_of_test_cases < 0)
		cin >> no_of_test_cases;
	for (int i = 0; i < no_of_test_cases; ++i) {
		// cout << "Case #" << i + 1 << ": ";
		if (int return_code = solve(); return_code != 0) {
			return return_code;
		}
	}
	return 0;
}
