#include <mutex>
#include "server.h"

mutex mstdout, mstderr;

using namespace std;

void write_stdout(string message)
{
    lock_guard<mutex> guard(mstdout);
    cout << message << endl;
}

void write_stderr(string message)
{
    lock_guard<mutex> guard(mstderr);
    cerr << message << endl;
}