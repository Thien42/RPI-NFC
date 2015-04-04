#include <iostream>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "CardTerminal.hh"

CardTerminalList::CardTerminalList(const Term &term, Options &opts) :
	_term(term),
	_options(opts),
	_readers(NULL),
	rgReaderStates_t(NULL),
	mszReaders(NULL),
	dwReaders(0),
	ptr(NULL)
{
	rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	test_rv("SCardEstablishContext", rv, hContext, term);
	rgReaderStates[0].szReader = "\\\\?PnP?\\Notification";
	rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;
	rv = SCardGetStatusChange(hContext, 0, rgReaderStates, 1);
	if (rgReaderStates[0].dwEventState && SCARD_STATE_UNKNOWN) {
		std::cout << "PnP reader name not supported. Using polling." << std::endl;
		opts.setPnp(false);
	}
}

CardTerminalList::~CardTerminalList() {
}

void CardTerminalList::list() {
	free(this->_readers);
	this->_readers = NULL;
	free(rgReaderStates_t);
	rgReaderStates_t = NULL;

	/* Retrieve the available readers list.
	 *
	 * 1. Call with a null buffer to get the number of bytes to allocate
	 * 2. malloc the necessary storage
	 * 3. call with the real allocated buffer
	 */
	std::cout << this->_term.getRed() << "Scanning present readers..." << this->_term.getColorEnd() << std::endl;
	rv = SCardListReaders(hContext, NULL, NULL, &dwReaders);
	if (rv != SCARD_E_NO_READERS_AVAILABLE) test_rv("SCardListReaders", rv, hContext, this->_term);
	dwReadersOld = dwReaders;

/* if non NULL we came back so free first */
	if (mszReaders) {
		free(mszReaders);
		mszReaders = NULL;
	}

	mszReaders = reinterpret_cast<LPSTR> (malloc(sizeof(char)*dwReaders));
	if (mszReaders == NULL) {
		std::cout << "malloc: not enough memory" << std::endl;
		exit(1);
	}

	*mszReaders = '\0';
	rv = SCardListReaders(hContext, NULL, mszReaders, &dwReaders);

	/* Extract readers from the null separated string and get the total
	 * number of readers */
	nbReaders = 0;
	ptr = mszReaders;
	while (*ptr != '\0') {
		ptr += strlen(ptr)+1;
		nbReaders++;
	}
}

void CardTerminalList::waitForReader(void) {
	if (SCARD_E_NO_READERS_AVAILABLE == rv || !nbReaders) {
		std::cout << this->_term.getRed() << "Waiting for the first reader..." << this->_term.getColorEnd() << std::endl;
		fflush(stdout);
		if (this->_options.getPnp()) {
			rgReaderStates[0].szReader = "\\\\?PnP?\\Notification";
			rgReaderStates[0].dwCurrentState = SCARD_STATE_UNAWARE;
			rv = SCardGetStatusChange(hContext, INFINITE, rgReaderStates, 1);
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
	/* allocate the readers table */
	this->_readers = reinterpret_cast<char**> (calloc(nbReaders+1, sizeof(char *)));
	if (!this->_readers) {
		std::cerr << "Not enough memory for readers table" << std::endl;
		exit(1);
	}

	/* fill the readers table */
	nbReaders = 0;
	ptr = mszReaders;
	while (*ptr) {
		printf("%s%d: %s%s\n", this->_term.getBlue(), nbReaders, ptr, this->_term.getColorEnd());
		_readers[nbReaders] = ptr;
		ptr += strlen(ptr)+1;
		nbReaders++;
	}

	/* allocate the ReaderStates table */
	rgReaderStates_t = reinterpret_cast<SCARD_READERSTATE*>(calloc(nbReaders+1, sizeof(* rgReaderStates_t)));
	if (NULL == rgReaderStates_t) {
		std::cerr << "Not enough memory for readers states" << std::endl;
		exit(1);
	}
	/* Set the initial states to something we do not know
	 * The loop below will include this state to the dwCurrentState
	 */
	for (int i = 0; i < nbReaders; i++) {
		rgReaderStates_t[i].szReader = _readers[i];
		rgReaderStates_t[i].dwCurrentState = SCARD_STATE_UNAWARE;
	}
	rgReaderStates_t[nbReaders].szReader = "\\\\?PnP?\\Notification";
	rgReaderStates_t[nbReaders].dwCurrentState = SCARD_STATE_UNAWARE;
}

void CardTerminalList::loop(void) {
	int current_reader;
	DWORD timeout;
	char atr[MAX_ATR_SIZE*3+1];	/* ATR in ASCII */
	char atr_command[sizeof(atr)+sizeof(ATR_PARSER)+2+1];

	this->waitForReader();
	this->setupReaders();

	/* Wait endlessly for all events in the list of readers
	 * We only stop in case of an error
	 */
	if (this->_options.getPnp()) {
		timeout = INFINITE;
		nbReaders++;
	} else timeout = TIMEOUT;
	rv = SCardGetStatusChange(hContext, timeout, rgReaderStates_t, nbReaders);
	while ((rv == SCARD_S_SUCCESS) || (rv == SCARD_E_TIMEOUT)) {
		if ((this->_options.getPnp() && rgReaderStates_t[nbReaders - 1].dwEventState & SCARD_STATE_CHANGED)
		|| ((SCardListReaders(hContext, NULL, NULL, &dwReaders) == SCARD_S_SUCCESS) && (dwReaders != dwReadersOld)) )
			this->list();

		/* Now we have an event, check all the readers in the list to see what
		 * happened */
		for (current_reader = 0; current_reader < nbReaders; current_reader++) {
			time_t t;
			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_CHANGED) {
				/* If something has changed the new state is now the current
				 * state */
				rgReaderStates_t[current_reader].dwCurrentState = rgReaderStates_t[current_reader].dwEventState;
			} else continue;

			/* From here we know that the state for the current reader has
			 * changed because we did not pass through the continue statement
			 * above.
			 */

			/* Timestamp the event as we get notified */
			t = time(NULL);

			/* Specify the current reader's number and name */
			printf("\n%s Reader %d: %s%s%s\n", ctime(&t), current_reader, this->_term.getMagenta(), rgReaderStates_t[current_reader].szReader, this->_term.getColorEnd());

			/* Dump the full current state */
			std::cout << "  Card state: " << this->_term.getRed();

			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_IGNORE)
				std::cout << "Ignore this reader, ";

			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_UNKNOWN) {
				std::cout << "Reader unknown" << std::endl;
				// goto get_readers;
			}

			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_UNAVAILABLE)
				std::cout << "Status unavailable, ";

			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_EMPTY)
				std::cout << "Card removed, ";

			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_PRESENT)
				std::cout << "Card inserted, ";

			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_ATRMATCH)
				std::cout << "ATR matches card, ";

			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_EXCLUSIVE)
				std::cout << "Exclusive Mode, ";

			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_INUSE)
				std::cout << "Shared Mode, ";
			if (rgReaderStates_t[current_reader].dwEventState & SCARD_STATE_MUTE)
				std::cout << "Unresponsive card - more than one card on reader?, ";
			std::cout << this->_term.getColorEnd() << std::endl;
			/* force display */
			fflush(stdout);
			/* Also dump the ATR if available */
			if (rgReaderStates_t[current_reader].cbAtr > 0) {
				printf("  ATR: ");
				if (rgReaderStates_t[current_reader].cbAtr) {
					int i;
					for (i = 0; i < rgReaderStates_t[current_reader].cbAtr; i++)
						sprintf(&atr[i*3], "%02X ", rgReaderStates_t[current_reader].rgbAtr[i]);
					atr[i*3-1] = '\0';
				} else atr[0] = '\0';
				printf("%s%s%s\n\n", this->_term.getMagenta(), atr, this->_term.getColorEnd());
				/* force display */
				fflush(stdout);
				if (this->_options.getAnalyseATR()) {
					sprintf(atr_command, ATR_PARSER " '%s'", atr);
					if (system(atr_command))
						perror(atr_command);
				}
			}
		} /* for */

		rv = SCardGetStatusChange(hContext, timeout, rgReaderStates_t, nbReaders);
	} /* while */

	/* If we get out the loop, GetStatusChange() was unsuccessful */
	test_rv("SCardGetStatusChange", rv, hContext, this->_term);
	
	/* We try to leave things as clean as possible */
	rv = SCardReleaseContext(hContext);
	test_rv("SCardReleaseContext", rv, hContext, this->_term);

	/* free memory possibly allocated */
	if (NULL != _readers) free(_readers);
	if (NULL != rgReaderStates_t) free(rgReaderStates_t);
}
