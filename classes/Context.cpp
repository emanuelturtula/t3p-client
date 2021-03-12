#include "Context.h"
#include <mutex>

using namespace std;

mutex m;

/**
 * Member functions for class Context
 * */

Context :: Context(context_t c)
{
    context = c;
}

context_t Context :: getContext()
{
    std::lock_guard<std::mutex> guard(m);
    return context;
}

void Context :: setContext(context_t c)
{
    std::lock_guard<std::mutex> guard(m);
    context = c;
}
