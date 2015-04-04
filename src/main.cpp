#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#ifdef __APPLE__
	#include <PCSC/wintypes.h>
	#include <PCSC/winscard.h>
#else
	#include <winscard.h>
#endif


/* command used to parse (on screen) the ATR */

#ifndef SCARD_E_NO_READERS_AVAILABLE
#define SCARD_E_NO_READERS_AVAILABLE 0x8010002E
#endif

#include "Term.hh"
#include "Options.hh"
#include "CardTerminalList.hh"

int main(int argc, char *argv[])
{
	Term term;
	Options opts;
	CardTerminalList list(term, opts);

	if (opts.init(argc, argv) == false) return (1);
	list.list();
	list.loop();
	return 0;
}
