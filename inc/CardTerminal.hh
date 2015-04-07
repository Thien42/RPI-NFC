#ifndef __CARD_TERMINAL_HH__
#define __CARD_TERMINAL_HH__

#include <string>
#ifdef __APPLE__
	#include <PCSC/wintypes.h>
	#include <PCSC/winscard.h>
#else
	#include <winscard.h>
#endif
#include "Options.hh"
#include "Term.hh"

#define ATR_PARSER "ATR_analysis"

class CardTerminal {
public:
	CardTerminal(std::string const&);
	~CardTerminal();
	const SCARD_READERSTATE &getState(void) const;
	void setCurrentState(SCARD_READERSTATE&);
	std::string const &getName(void) const;
	void displayState(const Term&, const Options&);
	void displayATR(const Term&, const Options&);
private:
	const std::string _name;
	SCARD_READERSTATE _state;
	struct s_reader {
		DWORD state;
		const char *name;
	};
};

#endif /* __CARD_TERMINAL_HH__ */
