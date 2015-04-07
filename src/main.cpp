#include "Term.hh"
#include "Options.hh"
#include "CardTerminalList.hh"

int main(int argc, char *argv[])
{
	Term term;
	Options opts;
	CardTerminalList list(term, opts);

	if (opts.init(argc, argv) == false) return (1);
	list.list();
	list.loop();
	return 0;
}
