// $Id: PktDagSrc.h 6219 2008-10-01 05:39:07Z vern $
//
// See the file "COPYING" in the main distribution directory for copyright.
//
// Support for Endace's DAG interface card.
//
// Caveats:
//    - No support for hardware-side filtering yet.
//    - No support for secondary filter path yet.
//    - No support for other media than Ethernet.
//    - Mutex should be per interface
//    - No support for multiple rx streams

#ifndef PKTDAGSRC_H
#define PKTDAGSRC_H

extern int snaplen;

#include "iosource/PktSrc.h"

namespace iosource {
namespace pktsrc {

class PktDagSrc : public iosource::PktSrc {
public:
	PktDagSrc(const std::string& path, bool is_live);
	virtual ~PktDagSrc();
	static PktSrc* InstantiatePktDagSrc(const std::string& path, bool is_live);

	// PktSrc interface:
	virtual void Statistics(Stats* stats);
	virtual bool PrecompileFilter(int index, const std::string& filter);
	virtual bool SetFilter(int index);

protected:
	virtual void Open();
	virtual bool ExtractNextPacket(Packet* pkt);
	virtual void DoneWithPacket();
	//virtual void GetFds(int* read, int* write, int* except);
	virtual void Close();

	void Error(const char* str);

private:
	static const unsigned int EXTRA_WINDOW_SIZE = 4 * 1024 * 1024;
	int stream_num;

	// Unfortunaly the DAG API has some problems with locking streams,
	// so we do our own checks to ensure we don't use more than one
	// stream.   In particular, the secondary filter won't work.
	static int mutex;

	int fd;
	int current_filter;

	Properties props;
	Stats stats;
};

}
}

#endif
