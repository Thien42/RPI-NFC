#include "CardTerminal.hh"

CardTerminal::CardTerminal(std::string const &name) :
	_name(name)
{
	this->_state.szReader = name.c_str();
	this->_state.dwCurrentState = SCARD_STATE_UNAWARE;
}

CardTerminal::~CardTerminal() {
}
