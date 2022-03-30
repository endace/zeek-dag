// See the file "COPYING" in the main distribution directory for copyright.

extern "C" {
#include <dagapi.h>
#include <dagerf.h>
#include <dagutil.h>
#include <pcap.h>
}
#include <errno.h>

#include <set>
#include <string>

#include "PktDagSrc.h"
#include "util.h"

#if ZEEK_VERSION_NUMBER >= 40100
using namespace zeek;
using zeek::util::fmt;
#endif

using namespace Endace_DAG;

#ifndef ERF_TYPE_META
#define ERF_TYPE_META 27
#endif

PktDagSrc::PktDagSrc(const std::string& path, bool is_live)
: PktSrc()
	{
	if ( ! is_live )
		Error("endace dag source does not support offline input");

	fd = -1;
	current_filter = -1;
	stream_num = 0;
	props.path = path;
	props.is_live = is_live;
	}

void PktDagSrc::Open()
	{
	char interface[DAGNAME_BUFSIZE];
	uint8_t erfs[ERF_TYPE_MAX];
	int types = 0;
	int i = 0;

	dag_parse_name(props.path.c_str(), interface, DAGNAME_BUFSIZE, &stream_num);

	// We only support Ethernet.
	props.link_type = DLT_EN10MB;
	props.netmask = NETMASK_UNKNOWN;

	fd = -1;
	current_filter = -1;

	if ((dag_ref = dag_config_init(interface)) == NULL) {
	    Error(fmt("dag_attach_stream: %s",
						strerror(errno)));
	    return;
	}
	fd = dag_config_get_card_fd(dag_ref);

	if ( fd < 0 )
		{
		Error(fmt("dag_config_get_card_fd: %s",
						strerror(errno)));
		return;
		}

	// XXX Currently, the DAG fd is not selectable :-(.
	// However, pretend that it is for now, as select() will always return true as the driver doesn't implement poll.
	// Otherwise in some cases Bro will never call us again after we go idle.
	// (e.g. where fd 0 that is added as a hack in PktMgr::GetFds() is not writeable, such as a debugger).
	// Zeek 3.1+ use a new event loop that does not appear to have this problem and does not support this hack.
#if ZEEK_VERSION_NUMBER >= 30100
	props.selectable_fd = -1;
#else
	props.selectable_fd = fd;
#endif

#if 0
	// long= is needed to prevent the DAG card from truncating jumbo frames.
	char* dag_configure_string =
		copy_string(fmt("slen=%d varlen long=%d",
				snaplen, snaplen > 1500 ? snaplen : 1500));

	fprintf(stderr, "Configuring %s with options \"%s\"...\n",
		interface, dag_configure_string);

	if ( dag_configure(fd, dag_configure_string) < 0 )
		{
		Error("dag_configure");
		delete [] dag_configure_string;
		return;
		}

	delete [] dag_configure_string;
#endif

	if ( dag_attach_stream64(fd, stream_num, 0, EXTRA_WINDOW_SIZE) < 0 )
		{
		Error(fmt("dag_attach_stream: %s",
						strerror(errno)));
		Close();
		return;
		}

	/* Try to find Stream Drop attribute */
	drop_attr = kNullAttributeUuid;
	dag_root = dag_config_get_root_component(dag_ref);
	if ( dag_component_get_subcomponent(dag_root, kComponentStreamFeatures, 0) )
	{
		drop_attr = dag_config_get_indexed_attribute_uuid(dag_ref, kUint32AttributeStreamDropCount, stream_num/2);
	}

	types = dag_get_stream_erf_types(fd, stream_num, erfs, ERF_TYPE_MAX);
	for (i = 0; i < types; i++)
		{
			dag_record_t dummy;
			dummy.type = erfs[i];
			dummy.rlen = dag_record_size;

			// Annoyingly there isn't a version of this that just takes a type, and dag_get_stream_erf_class_types doesn't currently appear to work with vDAGs.
			if (dagerf_is_ethernet_type((uint8_t*)&dummy))
				{
					break;
				}
		}

	if ( types > 0 && i == types )
		{
			// Don't give an error if we can't find any ERF types (e.g. uninitialized vDAG)
			Error("unsupported non-Ethernet DAG link type");
			Close();
			return;
		}

	if ( dag_start_stream(fd, stream_num) < 0 )
		{
		Error(fmt("dag_start_stream: %s",
						strerror(errno)));
		Close();
		return;
		}

	struct timeval maxwait, poll;

	// Wait for 100 microseconds for data to arrive if none available to reduce high idle CPU load.
	// Zeek poll sleeping every 10 or 100 dag stream polls isn't sufficient.
	maxwait.tv_sec = 0;
	maxwait.tv_usec = 100;
	poll.tv_sec = 0;
	poll.tv_usec = 100;

	// mindata == 0 for non-blocking. Using dag_record_size (16) so poll timeout works.
	if ( dag_set_stream_poll64(fd, stream_num, dag_record_size, &maxwait, &poll) < 0 )
		{
		Error(fmt("dag_set_stream_poll: %s",
						strerror(errno)));
		Close();
		return;
		}

	Info(fmt("Listening on DAG card on %s stream %d\n", interface, stream_num));

	stats.link = stats.received = stats.dropped = stats.bytes_received = 0;

	Opened(props);
	}

PktDagSrc::~PktDagSrc()
	{
	}


void PktDagSrc::Close()
	{
	if ( fd >= 0 )
		{
		dag_stop_stream(fd, stream_num);
		dag_detach_stream(fd, stream_num);
		dag_config_dispose(dag_ref);
		dag_ref = NULL;
		fd = -1;
		}

	Closed();
	}

bool PktDagSrc::ExtractNextPacket(Packet* pkt)
	{
	unsigned link_count = 0;	// # packets on link for this call
	const u_char *data = 0;
	pcap_pkthdr hdr;

	// As we can't use select() on the fd, we always have to pretend
	// we're busy (in fact this is probably even true; otherwise
	// we shouldn't be using such expensive monitoring hardware!).
	//idle = false;

	uint8_t *erf_ptr = 0;
	dag_record_t* r = 0;

	while ( true )
		{
		erf_ptr = dag_rx_stream_next_record(fd, stream_num);
		r = (dag_record_t*) erf_ptr;

		if ( ! r )
			{
			data = 0;	// make dataptr invalid

			if ( errno != EAGAIN )
				{
				// FIXME: Bro doesn't print Error() on close, only if net_init fails. Using InternalError() instead.
				InternalError(fmt("dag_rx_stream_next_record: %s",
						strerror(errno)));
				Close();
				return false;
				}

			else
				{ // gone dry
				return false;
				}
			}

		// Return after 20 unwanted packets on the link.
		if ( ++link_count > 20 )
			{
			data = 0;
			return false;
			}

		// Locate start of the Ethernet header.

		unsigned int erf_hdr_len = dag_record_size;
		uint8_t erf_type = r->type & 0x7f;

		if ((drop_attr == kNullAttributeUuid) && (!dagerf_is_color_type(erf_ptr)))
			{
			stats.dropped += ntohs(r->lctr);
			}

		if (erf_type == ERF_TYPE_PAD || erf_type == ERF_TYPE_META)
			{
			// Silently skip Provenance and pad records
			continue;
			}

		++stats.link;

		if (dagerf_is_ethernet_type(erf_ptr))
			{
			erf_hdr_len += 2; //eth pad
			}
		else
			{
				Weird(fmt("ERF record with unsupported type: %s (%d)", dagerf_type_to_string(r->type, TT_ERROR), erf_type), 0);
				continue;
			}

		if (r->type & 0x80)
			{
			unsigned int num_ext_hdrs = dagerf_ext_header_count(erf_ptr, ntohs(r->rlen));
			if (num_ext_hdrs == 0)
				{
				// TODO: re-arrange so we can supply pkt argument?
				Weird("ERF record with invalid extension header stack", 0);
				continue;
				}

			erf_hdr_len += 8 * num_ext_hdrs;
			}

		if (dagerf_is_multichannel_type(erf_ptr))
			erf_hdr_len += 4; //mc_hdr length

		if (erf_hdr_len > ntohs(r->rlen))
			{
			Weird("ERF record with insufficient length for header", 0);
			continue;
			}

		data = (const u_char*) erf_ptr + erf_hdr_len;

		hdr.len = ntohs(r->wlen);
		hdr.caplen = ntohs(r->rlen) - erf_hdr_len;

		// Records have trailing padding (usually 64-bit alignment), don't include this in caplen
		if (hdr.len < hdr.caplen)
			hdr.caplen = hdr.len;

		// Timestamp conversion taken from DAG programming manual.
		unsigned long long lts = r->ts;
		hdr.ts.tv_sec = lts >> 32;
		lts = ((lts & 0xffffffffULL) * 1000 * 1000);
		lts += (lts & 0x80000000ULL) << 1;
		hdr.ts.tv_usec = lts >> 32;
		if ( hdr.ts.tv_usec >= 1000000 )
			{
			hdr.ts.tv_usec -= 1000000;
			hdr.ts.tv_sec += 1;
			}

		// Finally, make sure the packet matches our filter (which may be none)
		if ( ApplyBPFFilter(current_filter, &hdr, data) )
			{
				break;
			}

		}

	++stats.received;
	stats.bytes_received += hdr.len; /* XXX: wlen, may not be if support non EN10MB DLTs later! */
	pkt->Init(props.link_type, &hdr.ts, hdr.caplen, hdr.len, data);

	return true;
	}

void PktDagSrc::DoneWithPacket()
	{
	// Nothing to do.
	}

// TODO: What is needed in modern Bro for this?
#if 0
void PktDagSrc::GetFds(int* read, int* write, int* except)
	{
	// We don't have a selectable fd, but we take the opportunity to
	// reset our idle flag if we have data now.
	if ( ! data )
		ExtractNextPacket();
	}
#endif

void PktDagSrc::Statistics(Stats* s)
	{
	dag_err_t dag_error;
	uint32_t drops;

	if(drop_attr != kNullAttributeUuid) {
		/* Note this counter is cleared at start of capture and will wrap at UINT_MAX.
		 * The application is responsible for polling s.dropped frequently enough
		 * to detect each wrap and integrate total drop with a wider counter */
		if ((dag_error = dag_config_get_uint32_attribute_ex(dag_ref, drop_attr, &drops)) != kDagErrNone) {
			Error(fmt("reading stream drop attribute: %s",
				  dag_config_strerror(dag_error)));
			return;
		}
		stats.dropped = drops;
	}

	s->received = stats.received;
	s->dropped = stats.dropped;
	s->link = stats.link + stats.dropped;
	s->bytes_received = stats.bytes_received;
	}

bool PktDagSrc::PrecompileFilter(int index, const std::string& filter)
	{
	return PktSrc::PrecompileBPFFilter(index, filter);
	}

bool PktDagSrc::SetFilter(int index)
	{
	current_filter = index;

	// Reset counters.
	stats.received = stats.dropped = stats.link = stats.bytes_received = 0;

	return true;
	}

iosource::PktSrc* PktDagSrc::InstantiatePktDagSrc(const std::string& path, bool is_live)
	{
	return new PktDagSrc(path, is_live);
	}
