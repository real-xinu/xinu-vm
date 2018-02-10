/* ttyputc.c - ttyputc */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  ttyputc  -  Write one character to a tty device (interrupts disabled)
 *------------------------------------------------------------------------
 */
devcall	ttyputc(
	struct	dentry	*devptr,	/* Entry in device switch table	*/
	char	ch			/* Character to write		*/
	)
{
	struct	ttycblk	*typtr;		/* Pointer to tty control block	*/

	typtr = &ttytab[devptr->dvminor];

	lock(typtr->tylock);

	/* Handle output CRLF by sending CR first */

    if ( ch==TY_NEWLINE && typtr->tyocrlf ) {
                ttyputc(devptr, TY_RETURN);
	}
	
	unlock(typtr->tylock);		/* in case wait() needs to resched */

	wait(typtr->tyosem);		/* Wait	for space in queue */

	lock(typtr->tylock);

	*typtr->tyotail++ = ch;

	/* Wrap around to beginning of buffer, if needed */

	if (typtr->tyotail >= &typtr->tyobuff[TY_OBUFLEN]) {
		typtr->tyotail = typtr->tyobuff;
	}
	unlock(typtr->tylock);

	/* Start output in case device is idle */

	ttykickout((struct uart_csreg *)devptr->dvcsr);

	return OK;
}
