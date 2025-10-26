#include <cstdio>
#include <iostream>
#include <string>
#include <map>
using namespace std;
//n=p*q
//e*d=(p-1)(q-1)+1
int main()
{
    int k;
    cin >> k;
    while(k--)
    {
        int n,e,d;
        cin >> n >> e >> d;
        bool flag = false;
        for (int p = 1 ; p <= n; p++)
        {
            int q = n / p;
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