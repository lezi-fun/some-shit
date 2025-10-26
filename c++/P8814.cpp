#include <cstdio>
#include <iostream>
#include <string>
#include <cmath>
#include <map>
using namespace std;
//n=p*q
//e*d=(p-1)(q-1)+1
//p <= q
//p+q=n-e*d+2
int main()
{
    int k;
    cin >> k;
    while(k--)
    {
        int n,e,d;
        cin >> n >> e >> d;
        bool flag = false;
        int sqrtn = sqrt(n);
        int PplusQ = n - e*d + 2;
        for (int p = 1 ; p <= sqrtn; p++)
        {
            int q = PplusQ - p;
            if (p > q) break;
            if (p * q == n && (e * d == (p - 1) * (q - 1) + 1))
            {
                cout << p << " " << q << endl;
                flag = true;
                break;
            }
        }
        if (flag == false) cout << "NO" << endl;
    }
    return 0;
}