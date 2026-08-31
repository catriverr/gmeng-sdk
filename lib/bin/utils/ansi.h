#pragma once

#include <string>
#include <vector>

/// splits a text into characters but includes colorcodes in chars.
static std::vector<std::string> split_with_ansi(const std::string& input)
{
    std::vector<std::string> result;
    std::string pending_ansi;

    auto read_ansi = [&](size_t& i) {
        size_t start = i++;
        if (i >= input.size()) return;

        if (input[i] == '[') // CSI
        {
            ++i;
            while (i < input.size() &&
                   (input[i] < '@' || input[i] > '~'))
                ++i;
            if (i < input.size()) ++i;
        }
        else if (input[i] == ']') // OSC
        {
            ++i;
            while (i < input.size() &&
                   !(input[i] == '\x07' ||
                     (input[i] == '\x1B' && i + 1 < input.size() && input[i + 1] == '\\')))
                ++i;
            if (i < input.size())
            {
                if (input[i] == '\x1B') i += 2;
                else ++i;
            }
        }
        else // single-char ESC
        {
            ++i;
        }

        pending_ansi.append(input, start, i - start);
    };

    for (size_t i = 0; i < input.size();)
    {
        if (input[i] == '\x1B')
        {
            read_ansi(i);
        }
        else
        {
            std::string s;
            s.reserve(pending_ansi.size() + 1);
            s += pending_ansi;
            pending_ansi.clear();

            s += input[i++];
            result.push_back(std::move(s));
        }
    }

    // Trailing ANSI → attach to last character
    if (!pending_ansi.empty() && !result.empty())
    {
        result.back() += pending_ansi;
    }

    return result;
}

/// Returns whether the character is
/// the start to an ANSI escape code sequence (`\x1B`)
inline bool is_ansi_start(char c) noexcept
{
    return c == '\x1B';
}

/// Returns an index to a logical index (index in a string EXCLUDING ansi escape codes)
/// as a physical index (index in a string INCLUDING ansi escape codes).
/// If the index cannot be found, `std::string::npos` is returned.
static size_t logical_to_physical_index(const std::string& s, size_t logical_index)
{
    size_t logical = 0;
    size_t i = 0;
    const size_t n = s.size();

    while (i < n)
    {
        if (is_ansi_start(s[i]))
        {
            ++i;
            if (i >= n) break;

            // CSI: ESC [
            if (s[i] == '[')
            {
                ++i;
                while (i < n && (s[i] < '@' || s[i] > '~'))
                    ++i;
                if (i < n) ++i;
            }
            // OSC: ESC ]
            else if (s[i] == ']')
            {
                ++i;
                while (i < n &&
                       !(s[i] == '\x07' ||
                        (s[i] == '\x1B' && i + 1 < n && s[i + 1] == '\\')))
                    ++i;
                if (i < n)
                {
                    if (s[i] == '\x1B') i += 2;
                    else ++i;
                }
            }
            // Single-character ESC
            else
            {
                ++i;
            }
        }
        else
        {
            if (logical == logical_index)
                return i;

            ++logical;
            ++i;
        }
    }

    return std::string::npos;
}


/// strips all ANSI escape codes / color codes from a text.
/// Used for getting the actual length of strings without accounting
/// for colorcodes.
static std::string strip_ansi(const std::string& input)
{
    std::string out;
    out.reserve(input.size());

    for (size_t i = 0; i < input.size();)
    {
        if (input[i] == '\x1B') // ESC
        {
            ++i;
            if (i >= input.size()) break;

            // CSI: ESC [
            if (input[i] == '[')
            {
                ++i;
                while (i < input.size() &&
                       (input[i] < '@' || input[i] > '~'))
                {
                    ++i;
                }
                if (i < input.size()) ++i;
            }
            // OSC: ESC ]
            else if (input[i] == ']')
            {
                ++i;
                while (i < input.size() &&
                       !(input[i] == '\x07' ||
                        (input[i] == '\x1B' && i + 1 < input.size() && input[i + 1] == '\\')))
                {
                    ++i;
                }
                if (i < input.size())
                {
                    if (input[i] == '\x1B') i += 2;
                    else ++i;
                }
            }
            // Other ESC sequences
            else
            {
                ++i;
            }
        }
        else
        {
            out += input[i++];
        }
    }

    return out;
}
