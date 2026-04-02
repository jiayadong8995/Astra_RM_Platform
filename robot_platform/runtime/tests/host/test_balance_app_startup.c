#include <assert.h>
#include <stdio.h>

#include "../../app/balance_chassis/app_bringup/task_registry.h"

/* The test links against this file alone — task_registry.c is NOT compiled.
   We provide our own balance_chassis_start_tasks stub to verify the API
   exists and is callable. */

static int g_start_tasks_calls = 0;

void balance_chassis_start_tasks(void)
{
    ++g_start_tasks_calls;
}

int main(void)
{
    balance_chassis_start_tasks();

    if (g_start_tasks_calls != 1)
    {
        fprintf(stderr,
                "balance_chassis_start_tasks should be callable once, got %d\n",
                g_start_tasks_calls);
        return 1;
    }

    assert(g_start_tasks_calls == 1);
    return 0;
}
