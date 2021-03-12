#ifndef CONTEXT_H
#define CONTEXT_H

#include "../types.h"

class Context {
    private:
        context_t context;
    
    public:
        Context(context_t c = DISCONNECTED);
        context_t getContext();
        void setContext(context_t c);
};

#endif