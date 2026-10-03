#ifndef WARC2TEXT_TDMRESERVATION_HH
#define WARC2TEXT_TDMRESERVATION_HH

#include "reservationbase.hh"

namespace warc2text {
    // Text and Data Mining reservation policy.
    class TDMReservation : public ReservationBase {
    public:
        bool isAllowedByHeader(const Record& record) const override;
        bool isAllowedByPayload(const Record& record) const override;

    private:
        bool allows(const std::string& val) const;
    };
}

#endif //WARC2TEXT_TDMRESERVATION_HH