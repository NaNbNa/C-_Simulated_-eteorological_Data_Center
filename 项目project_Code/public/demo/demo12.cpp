#include "_public.h"

using namespace idc;

int main()
{
    ctimer timer;

    printf("elapsed=%lf\n",timer.elapsed());
    sleep(1);
    printf("elapsed=%lf\n",timer.elapsed());
    sleep(1);
    printf("elapsed=%lf\n",timer.elapsed());
    usleep(1000);
    printf("elapsed=%lf\n",timer.elapsed());
    usleep(100);
    printf("elapsed=%lf\n",timer.elapsed());
    sleep(10);
    printf("elapsed=%lf\n",timer.elapsed());
}