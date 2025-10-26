#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;
int  main()
{
    int a,b,c;
    cin >> a >> b >> c;
    double alice = a * a,bob = b * c;
    if(alice > bob)
        cout << "Alice" << endl;
    else if(alice < bob)
        cout << "Bob" << endl;
    return 0;
}