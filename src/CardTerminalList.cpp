#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include "CardTerminalList.hh"

CardTerminalList::CardTerminalList(const Term &term, Options &opts) :
	_term(term),
	_options(opts),
	mszReaders(NULL),
	dwReaders(0)
{
	this->rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	this->test_rv("SCardEstablishContext");
	this->rgReaderStates.szReader = "\\\\?PnP?\\Notification";
	this->rgReaderStates.dwCurrentState = SCARD_STATE_UNAWARE;
	this->rv = SCardGetStatusChange(hContext, 0, &rgReaderStates, 1);
	if (this->rgReaderStates.dwEventState && SCARD_STATE_UNKNOWN) {
		std::cout << "PnP reader name not supported. Using polling." << std::endl;
		this->_options.setPnp(false);
	}
}

CardTerminalList::~CardTerminalList() {
}

void CardTerminalList::test_rv(std::string const &fct) {
	do {
		if (this->rv != SCARD_S_SUCCESS) {
			std::cout << this->_term.getRed() << fct << " : " << pcsc_stringify_error(this->rv) <<  this->_term.getColorEnd() << std::endl;
			(void) SCardReleaseContext(this->hContext);
			exit(-1);
		}
	} while(0);
}

void CardTerminalList::list() {
	this->_list.clear();
	std::cout << this->_term.getRed() << "Scanning present readers..." << this->_term.getColorEnd() << std::endl;
	this->rv = SCardListReaders(this->hContext, NULL, NULL, &this->dwReaders);
	if (this->rv != SCARD_E_NO_READERS_AVAILABLE) this->test_rv("SCardListReaders");
	this->dwReadersOld = this->dwReaders;
	if (this->mszReaders) {
		free(this->mszReaders);
		this->mszReaders = NULL;
	}
	this->mszReaders = reinterpret_cast<LPSTR> (malloc(sizeof(char) * this->dwReaders));
	if (this->mszReaders == NULL) {
		std::cout << "malloc: not enough memory" << std::endl;
		exit(1);
	}
	*(this->mszReaders) = 0;
	this->rv = SCardListReaders(this->hContext, NULL, this->mszReaders, &this->dwReaders);
	char *ptr = this->mszReaders;
	while (*ptr) {
		this->_list.push_back(new CardTerminal(ptr));
		ptr += strlen(ptr) + 1;
	}
}

void CardTerminalList::waitForReader(void) {
	if (SCARD_E_NO_READERS_AVAILABLE == rv || this->_list.size() == 0) {
		std::cout << this->_term.getRed() << "Waiting for the first reader..." << this->_term.getColorEnd() << std::endl;
		if (this->_options.getPnp()) {
			this->rv = SCardGetStatusChange(this->hContext, INFINITE, &rgReaderStates, 1);
			this->test_rv("SCardGetStatusChange");
		} else {
			this->rv = SCARD_S_SUCCESS;
			while ((SCARD_S_SUCCESS == this->rv) && (this->dwReaders == this->dwReadersOld)) {
				this->rv = SCardListReaders(this->hContext, NULL, NULL, &this->dwReaders);
				if (SCARD_E_NO_READERS_AVAILABLE == this->rv) this->rv = SCARD_S_SUCCESS;
				sleep(1);
			}
		}
		this->list();
	} else this->test_rv("SCardListReader");
}

void CardTerminalList::setupReaders(void) {
	int nbReaders = 0;
	char *ptr = mszReaders;
	this->_list.clear();
	while (*ptr) {
		printf("%s%d: %s%s\n", this->_term.getBlue(), nbReaders, ptr, this->_term.getColorEnd());
		this->_list.push_back(new CardTerminal(ptr));
		ptr += strlen(ptr) + 1;
		nbReaders++;
	}
}

SCARD_READERSTATE *CardTerminalList::buildReaderStates(void) {
	static std::vector<SCARD_READERSTATE> ret;
	ret.clear();
	auto it = this->_list.begin();
	for (it = it; it != this->_list.end(); ++it) {
		ret.push_back((*it)->getState());
	}
	ret.push_back(rgReaderStates);
	return (&ret[0]);
}

void CardTerminalList::loop(void) {
	DWORD timeout;

	this->waitForReader();
	this->setupReaders();
	if (this->_options.getPnp()) timeout = INFINITE;
	else timeout = TIMEOUT;
	SCARD_READERSTATE *tmp = this->buildReaderStates();
	this->rv = SCardGetStatusChange(this->hContext, timeout, tmp, this->_list.size() + 1);
	while ((this->rv == SCARD_S_SUCCESS) || (this->rv == SCARD_E_TIMEOUT)) {
		if ((this->_options.getPnp() && tmp[this->_list.size()].dwEventState & SCARD_STATE_CHANGED)
			|| ((SCardListReaders(this->hContext, NULL, NULL, &this->dwReaders) == SCARD_S_SUCCESS) && (this->dwReaders != this->dwReadersOld)) )
			this->list();
		for (unsigned int i = 0; i < this->_list.size(); i++) {
			if (tmp[i].dwEventState & SCARD_STATE_CHANGED) {
				tmp[i].dwCurrentState = tmp[i].dwEventState;
				this->_list[i]->setCurrentState(tmp[i]);
			} else continue;
			this->_list[i]->displayState(this->_term, this->_options);
		}
		tmp = this->buildReaderStates();
		this->rv = SCardGetStatusChange(this->hContext, timeout, tmp, this->_list.size() + 1);
	}
}
