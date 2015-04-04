#include <iostream>
#include "CardTerminal.hh"

CardTerminal::CardTerminal(std::string const &name) :
	_name(name)
{
	this->_state.szReader = name.c_str();
	this->_state.dwCurrentState = SCARD_STATE_UNAWARE;
}

CardTerminal::~CardTerminal() {
}

const SCARD_READERSTATE &CardTerminal::getState(void) const{
	return (this->_state);
}

std::string const &CardTerminal::getName(void) const {
	return (this->_name);
}

void CardTerminal::setCurrentState(SCARD_READERSTATE &state) {
	this->_state = state;
	this->_state.dwCurrentState = this->_state.dwEventState;
}

void CardTerminal::displayATR(const Term &term, const Options &opts) {
	char atr[MAX_ATR_SIZE*3+1];/* ATR in ASCII */
	char atr_command[sizeof(atr)+sizeof(ATR_PARSER)+2+1];

	if (this->_state.cbAtr > 0) {
		std::cout << "  ATR: ";
		if (this->_state.cbAtr) {
			int i;
			for (i = 0; i < this->_state.cbAtr; i++)
				sprintf(&atr[i*3], "%02X ", this->_state.rgbAtr[i]);
			atr[i*3-1] = '\0';
		} else atr[0] = '\0';
		printf("%s%s%s\n\n", term.getMagenta(), atr, term.getColorEnd());
		fflush(stdout);
		if (opts.getAnalyseATR()) {
			sprintf(atr_command, ATR_PARSER " '%s'", atr);
			if (system(atr_command)) perror(atr_command);
		}
	}
}

void CardTerminal::displayState(const Term &term, const Options &opts) {
	time_t t = time(NULL);
	std::cout << term.getMagenta() << "Reader " << this->getName() << term.getColorEnd() << std::endl;
	std::cout << "Card state : " << term.getRed();
	if (this->_state.dwCurrentState & SCARD_STATE_IGNORE)
		std::cout << "Ignore this reader, ";
	if (this->_state.dwCurrentState & SCARD_STATE_UNKNOWN)
		std::cout << "Reader unknown" << std::endl;
	if (this->_state.dwCurrentState & SCARD_STATE_UNAVAILABLE)
		std::cout << "Status unavailable, ";
	if (this->_state.dwCurrentState & SCARD_STATE_EMPTY)
		std::cout << "Card removed, ";
	if (this->_state.dwCurrentState & SCARD_STATE_PRESENT)
		std::cout << "Card inserted, ";
	if (this->_state.dwCurrentState & SCARD_STATE_ATRMATCH)
		std::cout << "ATR matches card, ";
	if (this->_state.dwCurrentState & SCARD_STATE_EXCLUSIVE)
		std::cout << "Exclusive Mode, ";
	if (this->_state.dwCurrentState & SCARD_STATE_INUSE)
		std::cout << "Shared Mode, ";
	if (this->_state.dwCurrentState & SCARD_STATE_MUTE)
		std::cout << "Unresponsive card - more than one card on reader?, ";
	std::cout << term.getColorEnd() << std::endl;
	if (opts.getAnalyseATR()) this->displayATR(term, opts);
}
