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
#include <vector>
#include "Term.hh"
#include "Options.hh"
#include "CardTerminal.hh"

#define TIMEOUT 100
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
	void waitForReader(void);
	void setupReaders(void);
private:
	SCARD_READERSTATE *buildReaderStates(void);
	const Term &_term;
	Options &_options;
	SCARDCONTEXT hContext;
	LPSTR mszReaders;
	DWORD dwReaders;
	DWORD dwReadersOld;
	SCARD_READERSTATE rgReaderStates;
	LONG rv;
	std::vector<CardTerminal*> _list;
};

#endif /* __CARD_TERMINAL_LIST_HH__ */
