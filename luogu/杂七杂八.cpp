#include <iostream>
#include <string>
using namespace std;
int main()
{
    int n;
    cin >> n;
    string s;
    cin >> s;
    for (int i = 0 ; i < s.size(); i++)
    {
        if(s[i] + n > 'z')
            cout << char(s[i] + n - 26);
        else
            cout << char(s[i] + n);
    }
    return 0;
}
