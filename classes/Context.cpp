#include "Context.h"
#include <mutex>

using namespace std;

mutex m;

// Constructor

Context :: Context(context_t c)
{
    context = c;
}

// Getters

context_t Context :: getContext()
{
    lock_guard<mutex> guard(m);
    return context;
}

// Setters

void Context :: setContext(context_t c)
{
    lock_guard<mutex> guard(m);
    context = c;
}
