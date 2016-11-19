
#include <kern/system.hpp>

/* This will keep track of how many ticks that the system
*  has been running for */
volatile unsigned int timer_ticks = 0;

void timer_handler(struct regs *r)
{
    /* Every 18 clocks (approximately 1 second), we will
    *  display a message on the screen */
    if (timer_ticks % 18 == 0)
    {
        updateClock();

    }
    //multi tasking goodness
    Sched->YieldIfNotBusy();

    /* Increment our 'tick count' */
    timer_ticks++;
}

/* This will continuously loop until the given time has
*  been reached */
void timer_wait(int ticks)
{
    unsigned long eticks;
    eticks = timer_ticks + ticks;
    while(timer_ticks < eticks)
    {
    	 __asm__ __volatile__ ("hlt");
    }
}

/* Sets up the system clock by installing the timer handler
*  into IRQ0 */
void timer_install()
{
	/* Installs 'timer_handler' to IRQ0 */
    irq_install_handler(0, timer_handler);
}
