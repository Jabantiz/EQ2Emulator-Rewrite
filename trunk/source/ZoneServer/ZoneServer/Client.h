#pragma once

#include "../../common/EQ2Stream.h"

class Client : public EQ2Stream {
public:
	Client(unsigned int ip, unsigned short port);

	void Process();

private:

};