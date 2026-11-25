#include "policy.h"

#include <string.h>

int policy_parse(const char *text, Policy *policy)
{
    if (strcmp(text, "fifo") == 0) {
        *policy = POLICY_FIFO;
        return 0;
    }
    if (strcmp(text, "lru") == 0) {
        *policy = POLICY_LRU;
        return 0;
    }
    return -1;
}

const char *policy_name(Policy policy)
{
    return policy == POLICY_LRU ? "lru" : "fifo";
}
