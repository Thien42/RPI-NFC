#include <iostream>
#include <unistd.h>
#include "Options.hh"
#ifdef __APPLE__
	#include <PCSC/wintypes.h>
	#include <PCSC/winscard.h>
#else
	#include <winscard.h>
#endif

Options::Options() : _analyse_atr(true), _pnp(true)
{
}

Options::~Options() {
}

void Options::usage(void) const {
	std::cout << "usage: pcsc_scan [-n] [-V] [-h] [-t]" << std::endl;
	std::cout << "  -n : no ATR analysis" << std::endl;
	std::cout << "  -h : this help" << std::endl;
	std::cout << "  -t : test" << std::endl;
}

bool Options::init(int ac, char **av) {
	int opt;
	std::cout << "PC/SC device scanner" << std::endl;
	std::cout << "Compiled with PC/SC lite version: " << PCSCLITE_VERSION_NUMBER << std::endl;

	while ((opt = getopt(ac, av, "hnt")) != -1)
	{
		switch (opt)
		{
			case 'n':
				this->_analyse_atr = false;
				break;

			case 'h':
				default:
				this->usage();
				return (false);
				break;

			case 't':
				std::cout << "test" << std::endl;
				return (false);
				break;
		}
	}

	if (ac - optind != 0)
	{
		this->usage();
		return (false);
	}
	return (true);
}

void Options::setAnalyseATR(bool analyse_atr) {
	this->_analyse_atr = analyse_atr;
}

bool Options::getAnalyseATR(void) const {
	return (this->_analyse_atr);
}

void Options::setPnp(bool pnp) {
	this->_pnp = pnp;
}

bool Options::getPnp(void) const {
	return (this->_pnp);
}
