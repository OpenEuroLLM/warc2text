#include "tdmreservation.hh"
#include "reservationbase.hh"
#include "record.hh"
#include "util.hh"
#include <boost/log/trivial.hpp>

namespace warc2text {

    bool TDMReservation::isAllowedByHeader(const Record& record) const {
        if (!record.HTTPheaderExists("tdm-reservation"))
            return true; // no header - no reservation
        std::string val = record.getHTTPheaderProperty("tdm-reservation");
        util::trim(val);
        if (allows(val))
            return true;
        BOOST_LOG_TRIVIAL(info) << "Record " << record.getURL()
                                << " discarded: tdm-reservation HTTP header value \""
                                << val << "\" (only \"0\" allows extraction)";
        return false;
    }

    bool TDMReservation::isAllowedByPayload(const Record& record) const {
        if (!record.isTextFormat())
            return true;
        for (std::string val : getMetaTagContents(record, "tdm-reservation")) {
            util::trim(val);
            if (!allows(val)) {
                BOOST_LOG_TRIVIAL(info) << "Record " << record.getURL()
                                        << " discarded: <meta name=\"tdm-reservation\" content=\""
                                        << val << "\"> (only \"0\" allows extraction)";
                return false;
            }
        }
        return true;  // all corresponding meta tags explicitly allow
    }

    bool TDMReservation::allows(const std::string& val) const {
        // if a tdm-reservation header/meta tag is present, the page is reserved
        // unless the value is "0" (no reservation)
        return val == "0";
    }
}