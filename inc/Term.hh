#ifndef __TERM_HH__
#define __TERM_HH__

#include <string>

class Term {
public:
	Term();
	~Term();
	const char *getRed(void) const;
	const char *getBlue(void) const;
	const char *getMagenta(void) const;
	const char *getColorEnd(void) const;
private:
	std::string _blue;
	std::string _red;
	std::string _magenta;
	std::string _color_end;
};

#endif /* __TERM_HH__ */
