#ifndef __CARD_TERMINAL_HH__
#define __CARD_TERMINAL_HH__

#include <string>
#ifdef __APPLE__
	#include <PCSC/wintypes.h>
	#include <PCSC/winscard.h>
#else
	#include <winscard.h>
#endif

class CardTerminal {
public:
	CardTerminal(std::string const&);
	~CardTerminal();
private:
	const std::string _name;
	SCARD_READERSTATE _state;
};

#endif /* __CARD_TERMINAL_HH__ */
