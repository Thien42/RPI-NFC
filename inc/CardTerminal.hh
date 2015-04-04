#ifndef __CARD_TERMINAL_LIST_HH__
#define __CARD_TERMINAL_LIST_HH__

#include <iostream>
#include <stdlib.h>
#ifdef __APPLE__
	#include <PCSC/wintypes.h>
	#include <PCSC/winscard.h>
#else
	#include <winscard.h>
#endif
#include "Term.hh"
#include "Options.hh"

#define TIMEOUT 100
#define ATR_PARSER "ATR_analysis"
#define test_rv(fct, rv, hContext, term) \
do { \
	if (rv != SCARD_S_SUCCESS) \
	{ \
		std::cout << term.getRed() << fct << " : " << pcsc_stringify_error(rv) <<  term.getColorEnd() << std::endl; \
		(void)SCardReleaseContext(hContext); \
		exit(-1); \
	} \
} while(0)

class CardTerminalList {
public:
	CardTerminalList(const Term &term, Options&);
	~CardTerminalList();
	void list(void);
	void loop(void);
private:
	const Term &_term;
	Options &_options;
	SCARDCONTEXT hContext;
	LPSTR mszReaders;
	DWORD dwReaders;
	DWORD dwReadersOld;
	SCARD_READERSTATE rgReaderStates[1];
	SCARD_READERSTATE *rgReaderStates_t;
	LONG rv;
	char **_readers;
	char *ptr;
	int nbReaders;
};

#endif /* __CARD_TERMINAL_LIST_HH__ */
