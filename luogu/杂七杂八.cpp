#incluude <iostream>
using namespace std;
int main()
{
    int n;
    cin >> n;
    int a[1000005] = {};
    for (int i = 1 ; i <= n  ; i++)
    {
        cin >> a[i];
    }
    int ans = 1;
    int cnt = 0;
    for (int i = 2 ; i <= n ; i++)
    {
        if(a[i] = a[i - 1] + 1)cnt++;
        else cnt = 0;
        ans = max(ans , cnt + 1);
    }
    cout << ans;
    return 0;
}