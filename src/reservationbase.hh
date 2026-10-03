#ifndef WARC2TEXT_RESERVATIONBASE_HH
#define WARC2TEXT_RESERVATIONBASE_HH

#include "record.hh"
#include <string>
#include <vector>

namespace warc2text {
    // Base class for content usage reservations that are expressed both via
    // HTTP headers and via HTML <meta> tags.
    class ReservationBase {
    public:
        virtual ~ReservationBase() = default;

        // True if the record's HTTP headers do not reserve the content.
        virtual bool isAllowedByHeader(const Record& record) const = 0;

        // True if the record's payload (HTML <meta> tags) does not reserve the content.
        virtual bool isAllowedByPayload(const Record& record) const = 0;

    protected:
        // Content values of <meta name="..." content="..."> tags with the given
        // lowercase tag name, scanning the payload and skipping comments.
        static std::vector<std::string> getMetaTagContents(const Record& record, const std::string& name);
    };
}

#endif //WARC2TEXT_RESERVATIONBASE_HH