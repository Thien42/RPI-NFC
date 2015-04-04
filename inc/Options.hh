#ifndef __OPTIONS_HH_
#define __OPTIONS_HH_

class Options {
public:
	Options();
	~Options();
	void usage(void) const;
	bool init(int, char**);

	void setAnalyseATR(bool);
	bool getAnalyseATR(void) const;

	void setPnp(bool);
	bool getPnp(void) const;
private:
	bool _analyse_atr;
	bool _pnp;
};

#endif /* __OPTIONS_HH_ */
