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
        struct HeadMeta {
            std::vector<std::string> tdm;
            std::vector<std::string> robots;
        };

        virtual ~ReservationBase() = default;

        // True if the record's HTTP headers do not reserve the content.
        virtual bool isAllowedByHeader(const Record& record) const = 0;

        // True if the record's payload (HTML <meta> tags) does not reserve the
        // content. The meta tags are pre-extracted and passed as an argument
        // to avoid many passes over HTML.
        virtual bool isAllowedByPayload(const Record& record, const HeadMeta& headMeta) const = 0;

        // Extract opt-out meta tags from the HTML head.
        static HeadMeta getHeadMetaReservations(const Record& record);
    };
}

#endif //WARC2TEXT_RESERVATIONBASE_HH