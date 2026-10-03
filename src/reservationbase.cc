#include "reservationbase.hh"
#include "record.hh"
#include <cctype>
#include <regex.h>
#include <string>
#include <utility>
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

    // Position of the next match of `pat` (a "<tag\>" pattern compiled with
    // REG_ICASE) that is not inside an HTML comment. regexec works on
    // NUL-terminated C strings, so the search is split into NUL-free segments;
    // a tag can never span a NUL byte.
    std::size_t findTag(const std::string& s, std::size_t from, const regex_t& pat) {
        while (from < s.size()) {
            std::size_t nul = s.find('\0', from);
            if (nul == std::string::npos)
                nul = s.size();
            regmatch_t m;
            if (regexec(&pat, s.c_str() + from, 1, &m, 0) != 0) {
                from = nul + 1;
                continue;
            }
            std::size_t hit = from + static_cast<std::size_t>(m.rm_so);
            std::size_t cmt = s.find("<!--", from);
            if (cmt != std::string::npos && cmt < hit) {
                std::size_t end = s.find("-->", cmt);
                from = end == std::string::npos ? s.size() : end + 3;
                continue;
            }
            return hit;
        }
        return std::string::npos;
    }

    // Extract the quoted value captured by the first group of `pat` from `tag`.
    bool attrValue(const std::string& tag, const regex_t& pat, std::string& out) {
        regmatch_t m[2];
        if (regexec(&pat, tag.c_str(), 2, m, 0) != 0)
            return false;
        out.assign(tag, m[1].rm_so, m[1].rm_eo - m[1].rm_so);
        return true;
    }

    struct TagRegexes {
        regex_t meta;
        regex_t head;
        regex_t close_head;
        regex_t name_attr;
        regex_t content_attr;
        bool ok;
        TagRegexes() {
            ok = (regcomp(&meta, "<meta\\>", REG_EXTENDED | REG_ICASE) == 0) &&
                 (regcomp(&head, "<head\\>", REG_EXTENDED | REG_ICASE) == 0) &&
                 (regcomp(&close_head, "</head\\>", REG_EXTENDED | REG_ICASE) == 0) &&
                 (regcomp(&name_attr, "\\<name[[:space:]]*=[[:space:]]*[\"']([^\"']*)[\"']",
                          REG_EXTENDED | REG_ICASE) == 0) &&
                 (regcomp(&content_attr, "\\<content[[:space:]]*=[[:space:]]*[\"']([^\"']*)[\"']",
                          REG_EXTENDED | REG_ICASE) == 0);
        }
    };
}

namespace warc2text {

    std::vector<std::string> ReservationBase::getMetaTagContents(const Record& record, const std::string& name) {
        static const TagRegexes rx;
        std::vector<std::string> contents;
        if (!rx.ok)
            return contents;

        const std::string& s = record.getPayload();

        // 1. All <meta name="..." content="..."> tags in the document (comments skipped).
        std::vector<std::pair<std::size_t, std::string>> found;
        std::size_t pos = 0;
        while (true) {
            std::size_t lt = findTag(s, pos, rx.meta);
            if (lt == std::string::npos)
                break;
            std::size_t gt = s.find('>', lt);
            if (gt == std::string::npos)
                break;
            pos = gt + 1;

            std::string tag(s, lt, gt - lt);
            std::string name_val;
            if (!attrValue(tag, rx.name_attr, name_val))
                continue;
            if (!ciEquals(name_val, name))
                continue;
            std::string content_val;
            if (attrValue(tag, rx.content_attr, content_val))
                found.push_back({lt, content_val});
        }

        // Nothing found: no restriction expressed.
        if (found.empty())
            return contents;

        // 2. The real (non-commented) <head> ... </head> span.
        std::size_t head_start = findTag(s, 0, rx.head);
        if (head_start == std::string::npos)
            return contents;
        std::size_t head_end = findTag(s, head_start, rx.close_head);
        if (head_end == std::string::npos)
            return contents;

        // 3. Only meta tags located inside the head span count.
        for (const auto& candidate : found) {
            if (head_start <= candidate.first && candidate.first < head_end)
                contents.push_back(candidate.second);
        }
        return contents;
    }
}