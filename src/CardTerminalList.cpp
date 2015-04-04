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
	rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	test_rv("SCardEstablishContext", rv, hContext, term);
	rgReaderStates.szReader = "\\\\?PnP?\\Notification";
	rgReaderStates.dwCurrentState = SCARD_STATE_UNAWARE;
	rv = SCardGetStatusChange(hContext, 0, &rgReaderStates, 1);
	if (rgReaderStates.dwEventState && SCARD_STATE_UNKNOWN) {
		std::cout << "PnP reader name not supported. Using polling." << std::endl;
		opts.setPnp(false);
	}
}

CardTerminalList::~CardTerminalList() {
}

void CardTerminalList::list() {
	char *ptr;
	this->_list.clear();
	std::cout << this->_term.getRed() << "Scanning present readers..." << this->_term.getColorEnd() << std::endl;
	rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
	if (rv != SCARD_E_NO_READERS_AVAILABLE) test_rv("SCardListReaders", rv, hContext, this->_term);
	dwReadersOld = dwReaders;
	if (mszReaders) {
		free(mszReaders);
		mszReaders = NULL;
	}
	mszReaders = reinterpret_cast<LPSTR> (malloc(sizeof(char)*dwReaders));
	if (mszReaders == NULL) {
		std::cout << "malloc: not enough memory" << std::endl;
		exit(1);
	}
	*mszReaders = 0;
	rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);
	ptr = mszReaders;
	while (*ptr) {
		this->_list.push_back(new CardTerminal(ptr));
		ptr += strlen(ptr) + 1;
	}
}

void CardTerminalList::waitForReader(void) {
	if (SCARD_E_NO_READERS_AVAILABLE == rv || this->_list.size() == 0) {
		std::cout << this->_term.getRed() << "Waiting for the first reader..." << this->_term.getColorEnd() << std::endl;
		if (this->_options.getPnp()) {
			rv = SCardGetStatusChange(hContext, INFINITE, &rgReaderStates, 1);
			test_rv("SCardGetStatusChange", rv, hContext, this->_term);
		} else {
			rv = SCARD_S_SUCCESS;
			while ((SCARD_S_SUCCESS == rv) && (dwReaders == dwReadersOld)) {
				rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
				if (SCARD_E_NO_READERS_AVAILABLE == rv) rv = SCARD_S_SUCCESS;
				sleep(1);
			}
		}
		this->list();
	} else test_rv("SCardListReader", rv, hContext, this->_term);
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
	rv = SCardGetStatusChange(hContext, timeout, tmp, this->_list.size() + 1);
	while ((rv == SCARD_S_SUCCESS) || (rv == SCARD_E_TIMEOUT)) {
		if ((this->_options.getPnp() && tmp[this->_list.size()].dwEventState & SCARD_STATE_CHANGED)
			|| ((SCardListReaders(hContext, NULL, NULL, &dwReaders) == SCARD_S_SUCCESS) && (dwReaders != dwReadersOld)) )
			this->list();
		for (int i = 0; i < this->_list.size(); i++) {
			const SCARD_READERSTATE &current = this->_list[i]->getState();
			if (tmp[i].dwEventState & SCARD_STATE_CHANGED) {
				this->_list[i]->setCurrentState(tmp[i]);
			} else continue;
			this->_list[i]->displayState(this->_term, this->_options);
		}
		tmp = this->buildReaderStates();
		rv = SCardGetStatusChange(hContext, timeout, tmp, this->_list.size() + 1);
	}
}
