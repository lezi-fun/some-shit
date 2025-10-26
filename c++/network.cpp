#include <iostream>
#include <string>
#include <algorithm>
#include <cstdio>
#include <map>
using namespace std;
map<string, int> server;
int main()
{
    int n;
    cin >> n;
    for (int i = 1; i <= n; i++)
    {
        string op;
        cin >> op;
        int a,b,c,d,e;
        scanf("%d.%d.%d.%d:%d", &a, &b, &c, &d, &e);
        if(a<0 || a>255 || b<0 || b>255 || c<0 || c>255 || d<0 || d>255 || e<0 || e>65535)
        {
            cout << "ERR" << endl;
            continue;
        }
        string ad = to_string(a) + "." + to_string(b) + "." + 
                    to_string(c) + "." + to_string(d) + ":" + 
                    to_string(e);
        if(op == "Server")
        {
            if(server[ad])
            {
                cout << "FAIL" << endl;
                continue;
            }
            server[ad] = i;
            cout << "OK" << endl;
        }
        else if(op == "Client")
        {
            if(server[ad])
            {
                cout << server[ad] << endl;
            }
            else
            {
                cout << "FAIL" << endl;
            }
        }
    }
    return 0;
}
