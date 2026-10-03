#include "robotsreservation.hh"
#include "reservationbase.hh"
#include "record.hh"
#include "util.hh"
#include <boost/log/trivial.hpp>

namespace warc2text {

    bool RobotsReservation::isAllowedByHeader(const Record& record) const {
        if (!record.HTTPheaderExists("x-robots-tag"))
            return true;
        std::string directives = util::toLowerCopy(record.getHTTPheaderProperty("x-robots-tag"));
        std::string rejecting = disallowReason(directives);
        if (rejecting.empty())
            return true;
        BOOST_LOG_TRIVIAL(info) << "Record " << record.getURL()
                                << " discarded: x-robots-tag HTTP header contains \"" << rejecting
                                << "\" directive";
        return false;
    }

    bool RobotsReservation::isAllowedByPayload(const Record& record) const {
        if (!record.isTextFormat())
            return true;
        for (std::string val : getMetaTagContents(record, "robots")) {
            std::string rejecting = disallowReason(util::toLowerCopy(val));
            if (!rejecting.empty()) {
                BOOST_LOG_TRIVIAL(info) << "Record " << record.getURL()
                                        << " discarded: <meta name=\"robots\" content=\""
                                        << val << "\"> contains \"" << rejecting << "\" directive";
                return false;
            }
        }
        return true;
    }

    // Returns the directive that rejects extraction (empty string when a page is
    // allowed): any directive starting with "no", except "nofollow" and "noodp",
    // which only affect link/ODP handling and are harmless here.
    std::string RobotsReservation::disallowReason(std::string directives) const {
        std::vector<std::string> parts = util::split(directives, ",");
        for (std::string& d : parts) {
            util::trim(d);
            if (d.rfind("no", 0) == 0 && d != "nofollow" && d != "noodp")
                return d;
        }
        return "";
    }
}