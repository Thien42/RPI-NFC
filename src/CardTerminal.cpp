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
}

void CardTerminal::displayATR(const Term &term, const Options &opts) {
	char atr[MAX_ATR_SIZE * 3 + 1];
	char atr_command[sizeof(atr) + sizeof(ATR_PARSER) + 2 + 1];

	if (this->_state.cbAtr > 0) {
		std::cout << "  ATR: ";
		if (this->_state.cbAtr) {
			unsigned int i = 0;
			for (i = i; i < this->_state.cbAtr; i++)
				sprintf(&atr[i * 3], "%02X ", this->_state.rgbAtr[i]);
			atr[i * 3 - 1] = '\0';
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
	static const struct s_reader states[] = {
		{SCARD_STATE_IGNORE, "Ignore this reader"},
		{SCARD_STATE_UNKNOWN, "Reader unknown"},
		{SCARD_STATE_UNAVAILABLE, "Status unavailable"},
		{SCARD_STATE_EMPTY, "Card removed"},
		{SCARD_STATE_PRESENT, "Card inserted"},
		{SCARD_STATE_ATRMATCH, "ATR matches card"},
		{SCARD_STATE_EXCLUSIVE, "Exclusive mode"},
		{SCARD_STATE_INUSE, "Shared mode"},
		{SCARD_STATE_MUTE, "Unresponsive card - more than one card on the reader ?"},
		{0, NULL}
	};
	std::cout << term.getMagenta() << "Reader " << this->getName() << term.getColorEnd() << std::endl;
	std::cout << "Card state : " << term.getRed();
	for (int i = 0; states[i].name != NULL; i++)
		if (states[i].state & this->_state.dwCurrentState)
			std::cout << states[i].name << std::endl;
	std::cout << term.getColorEnd() << std::endl;
	if (opts.getAnalyseATR()) this->displayATR(term, opts);
}
