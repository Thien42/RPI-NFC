#include <stdlib.h>
#include <string.h>
#include "Term.hh"

Term::Term() {
	const char *terms[] = { "linux", "xterm", "xterm-color", "Eterm", "rxvt", "rxvt-unicode" };

	char *term = getenv("TERM");
	if (term) {
		for (int i = 0; i < sizeof(terms) / sizeof(terms[0]); i++)
		{
			if (!strcmp(terms[i], term))
			{
				this->_blue = "\33[34m";
				this->_red = "\33[31m";
				this->_magenta = "\33[35m";
				this->_color_end = "\33[0m";
				break;
			}
		}
	}
}

Term::~Term() {
}

const char *Term::getRed(void) const {
	return (this->_red.c_str());
}

const char *Term::getBlue(void) const {
	return (this->_blue.c_str());
}

const char *Term::getMagenta(void) const {
	return (this->_magenta.c_str());
}

const char *Term::getColorEnd(void) const {
	return (this->_color_end.c_str());
}
