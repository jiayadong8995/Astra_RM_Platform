#include <assert.h>
#include <stdio.h>

#include "../../app/balance_chassis/app_startup/balance_chassis_app_startup.h"

static int g_balance_chassis_start_tasks_calls = 0;

void balance_chassis_start_tasks(void)
{
    ++g_balance_chassis_start_tasks_calls;
}

int main(void)
{
    balance_chassis_app_startup();

    if (g_balance_chassis_start_tasks_calls != 1)
    {
        fprintf(stderr,
                "balance_chassis_app_startup should reach task registration once, got %d\n",
                g_balance_chassis_start_tasks_calls);
        return 1;
    }

    assert(g_balance_chassis_start_tasks_calls == 1);
    return 0;
}
