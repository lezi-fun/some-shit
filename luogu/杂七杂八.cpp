#include <iostream>
#include <string>
using namespace std;
int main()
{
    int n;
    cin >> n;
    string s;
    cin >> s;
    n = n % 26;
    for (int i = 0 ; i < s.size(); i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
        {
            char x = (s[i] - 'a' + n) % 26 + 'a';
            cout << x;
        }
    }
    return 0;
}
