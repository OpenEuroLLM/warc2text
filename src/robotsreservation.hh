#ifndef WARC2TEXT_ROBOTSRESERVATION_HH
#define WARC2TEXT_ROBOTSRESERVATION_HH

#include "reservationbase.hh"

namespace warc2text {
    // X-Robots-Tag / robots meta tag reservation policy.
    class RobotsReservation : public ReservationBase {
    public:
        bool isAllowedByHeader(const Record& record) const override;
        bool isAllowedByPayload(const Record& record) const override;

    private:
        std::string disallowReason(std::string directives) const;
    };
}

#endif //WARC2TEXT_ROBOTSRESERVATION_HH