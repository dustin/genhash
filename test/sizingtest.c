#include <stdio.h>
#include <assert.h>
#include <limits.h>

#include "genhash.h"
#include "genhash_int.h"

static void
assert_size(int est, int expected)
{
    int size = estimate_table_size(est);
    assert(size == expected);
}

void test_sizing()
{
    assert_size(1, 3);
    assert_size(400000, 786433);
    assert_size(INT_MAX, 1610612741);
}

int main(int argc, char **argv)
{
    test_sizing();
    return 0;
}
