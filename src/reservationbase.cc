#include "reservationbase.hh"
#include "record.hh"
#include <boost/regex.hpp>
#include <cctype>
#include <string>
#include <string_view>
#include <vector>

namespace {
    // Case-insensitive equality against a lowercase constant.
    bool ciEquals(const std::string& value, const std::string& lower) {
        if (value.size() != lower.size())
            return false;
        for (std::size_t i = 0; i < value.size(); ++i) {
            if (std::tolower(static_cast<unsigned char>(value[i])) != lower[i])
                return false;
        }
        return true;
    }

    // Find `needle` inside s[from, end), return npos if absent.
    std::size_t boundedFind(const std::string& s, std::size_t from, std::size_t end,
                            std::string_view needle) {
        if (from >= end || end > s.size())
            return std::string::npos;
        std::string_view hay(s.data() + from, end - from);
        std::size_t p = hay.find(needle);
        return p == std::string_view::npos ? std::string::npos : from + p;
    }


    // Extract the quoted value captured by the first group of `re` from `tag`.
    bool attrValue(const std::string& tag, const boost::regex& re, std::string& out) {
        boost::match_results<std::string::const_iterator> m;
        if (!boost::regex_search(tag.begin(), tag.end(), m, re))
            return false;
        out.assign(m[1].first, m[1].second);
        return true;
    }

    // Position of the first match of `re` inside s[from, end) that is not
    // inside an HTML comment.
    // `re` is expected to be of the form `(<pat>)|<!--`.
    std::size_t findNonCommented(const std::string& s, std::size_t from, std::size_t end,
                                 const boost::regex& re) {
        while (from < end) {
            boost::match_results<std::string::const_iterator> m;
            if (!boost::regex_search(s.begin() + from, s.begin() + end, m, re))
                return std::string::npos;
            // If the real pattern matched (group 1), it's the leftmost non-commented
            // match. If group 1 didn't match, then '<!--' matched.
            if (m[1].matched) {
                std::size_t hit = from + static_cast<std::size_t>(m.position(1));
                return hit;
            }
            std::size_t cmt = from + static_cast<std::size_t>(m.position());
            std::size_t ce = boundedFind(s, cmt, end, "-->");
            if (ce == std::string::npos)
                return std::string::npos; // unterminated comment swallows the region
            from = ce + 3;
        }
        return std::string::npos;
    }

    // Precompiled patterns. \b matches word boundaries.
    struct Regexes {
        boost::regex head;
        boost::regex close_head;
        boost::regex meta;
        boost::regex name_attr;
        boost::regex content_attr;

        Regexes()
            : head("(<head\\b)|<!--", boost::regex_constants::icase),
              close_head("(</head\\b)|<!--", boost::regex_constants::icase),
              meta("(<meta\\b)|<!--", boost::regex_constants::icase),
              name_attr("\\bname\\s*=\\s*[\"']([^\"']*)[\"']",
                        boost::regex_constants::icase),
              content_attr("\\bcontent\\s*=\\s*[\"']([^\"']*)[\"']",
                           boost::regex_constants::icase) {}
    };
}

namespace warc2text {

    ReservationBase::HeadMeta ReservationBase::getHeadMetaReservations(const Record& record) {
        static const Regexes rx;
        HeadMeta out;

        const std::string& s = record.getPayload();
        const std::size_t end = s.size();

        // Locate the real (non-commented) <head> ... </head> span.
        std::size_t head_start = findNonCommented(s, 0, end, rx.head);
        if (head_start == std::string::npos)
            return out;
        std::size_t head_end = findNonCommented(s, head_start, end, rx.close_head);
        if (head_end == std::string::npos)
            return out;

        // collect the reservation meta tags inside the head span only
        std::size_t pos = head_start;
        while (pos < head_end) {
            std::size_t lt = findNonCommented(s, pos, head_end, rx.meta);
            if (lt == std::string::npos)
                break;
            std::size_t gt = boundedFind(s, lt, head_end, ">");
            if (gt == std::string::npos)
                break; // incorrect tag without a closing '>'
            std::string tag(s, lt, gt - lt);
            std::string name_val;
            if (attrValue(tag, rx.name_attr, name_val)) {
                if (ciEquals(name_val, "tdm-reservation")) {
                    std::string content_val;
                    if (attrValue(tag, rx.content_attr, content_val))
                        out.tdm.push_back(content_val);
                } else if (ciEquals(name_val, "robots")) {
                    std::string content_val;
                    if (attrValue(tag, rx.content_attr, content_val))
                        out.robots.push_back(content_val);
                }
            }
            pos = gt + 1;
        }
        return out;
    }
}