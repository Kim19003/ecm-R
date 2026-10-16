#pragma once

#include <string>
#include <cwctype>
#include <cctype>
#include <Windows.h>

#include "fs/fs.hpp"

namespace korean
{
	// Choseong (initial consonants): 19
	inline constexpr const char* kChoseong[] = {
		"g", "kk", "n", "d", "tt", "r", "m", "b", "pp",
		"s", "ss", "", "j", "jj", "ch", "k", "t", "p", "h"
	};

	// Jungseong (medial vowels): 21
	inline constexpr const char* kJungseong[] = {
		"a", "ae", "ya", "yae", "eo", "e", "yeo", "ye", "o",
		"wa", "wae", "oe", "yo", "u", "wo", "we", "wi", "yu",
		"eu", "ui", "i"
	};

	// Jongseong (final consonants): 28 (0 = none)
	inline constexpr const char* kJongseong[] = {
		"", "k", "k", "k", "n", "n", "n", "t", "l", "k",
		"m", "l", "l", "t", "p", "l", "m", "p", "p", "t",
		"t", "ng", "t", "t", "k", "t", "p", "h"
	};

	// Compatibility Jamo (U+3131..U+3163)
	// 0x3131 (ㄱ) to 0x314E (ㅎ): 30 consonants
	inline constexpr const char* kCompatConsonants[] = {
		"g", "kk", "gs", "n", "nj", "nh", "d", "dd", "r", "lg",
		"lm", "lb", "ls", "lt", "lp", "lh", "m", "b", "bb", "bs",
		"s", "ss", "ng", "j", "jj", "ch", "k", "t", "p", "h"
	};

	// 0x314F (ㅏ) to 0x3163 (ㅣ): 21 vowels
	inline constexpr const char* kCompatVowels[] = {
		"a", "ae", "ya", "yae", "eo", "e", "yeo", "ye", "o",
		"wa", "wae", "oe", "yo", "u", "wo", "we", "wi", "yu",
		"eu", "ui", "i"
	};

	struct Syllable
	{
		int initial; // 0..18
		int vowel;   // 0..20
		int final_c; // 0..27
	};

	inline bool is_hangul_syllable(wchar_t c)
	{
		return c >= 0xAC00 && c <= 0xD7A3;
	}

	inline bool is_compat_jamo(wchar_t c)
	{
		return c >= 0x3131 && c <= 0x3163;
	}

	inline Syllable decompose(wchar_t c)
	{
		const int sindex = static_cast<int>(c) - 0xAC00;
		return {
			sindex / (21 * 28),
			(sindex % (21 * 28)) / 28,
			sindex % 28
		};
	}

	inline bool is_word_boundary_char(wchar_t c)
	{
		return iswspace(c) || iswpunct(c) || c == L'(' || c == L')' ||
		       c == L'[' || c == L']' || c == L'{' || c == L'}' ||
		       c == L'<' || c == L'>' || c == L'"' || c == L'\'' ||
		       c == L'-' || c == L'_' || c == L'/' || c == L'\\';
	}

	// Resolves inter-syllable assimilation between the current syllable's final consonant
	// and the next syllable's initial consonant according to Revised Romanization rules.
	inline void resolve_assimilation(
		const Syllable& cur,
		const Syllable& next,
		std::string& cur_final_out,
		std::string& next_initial_out)
	{
		cur_final_out = kJongseong[cur.final_c];
		next_initial_out = kChoseong[next.initial];

		if (cur.final_c == 0)
		{
			return;
		}

		// Case 1: Next syllable starts with a vowel (initial 11 = ㅇ, silent onset)
		// Liaison / Resyllabification: final consonant carries over to next syllable onset.
		if (next.initial == 11)
		{
			switch (cur.final_c)
			{
			case 1:  cur_final_out = ""; next_initial_out = "g"; break;  // ㄱ -> g
			case 2:  cur_final_out = ""; next_initial_out = "kk"; break; // ㄲ -> kk
			case 3:  cur_final_out = "k"; next_initial_out = "s"; break; // ㄳ -> k + s
			case 4:  cur_final_out = ""; next_initial_out = "n"; break;  // ㄴ -> n
			case 5:  cur_final_out = "n"; next_initial_out = "j"; break; // ㄵ -> n + j
			case 6:  cur_final_out = ""; next_initial_out = "n"; break;  // ㄶ -> n (h drops)
			case 7:  cur_final_out = ""; next_initial_out = "d"; break;  // ㄷ -> d
			case 8:  cur_final_out = ""; next_initial_out = "r"; break;  // ㄹ -> r (between vowels)
			case 9:  cur_final_out = "l"; next_initial_out = "g"; break; // ㄺ -> l + g
			case 10: cur_final_out = "l"; next_initial_out = "m"; break; // ㄻ -> l + m
			case 11: cur_final_out = "l"; next_initial_out = "b"; break; // ㄼ -> l + b
			case 12: cur_final_out = "l"; next_initial_out = "s"; break; // ㄽ -> l + s
			case 13: cur_final_out = "l"; next_initial_out = "t"; break; // ㄾ -> l + t
			case 14: cur_final_out = "l"; next_initial_out = "p"; break; // ㄿ -> l + p
			case 15: cur_final_out = ""; next_initial_out = "r"; break;  // ㅀ -> r (h drops)
			case 16: cur_final_out = ""; next_initial_out = "m"; break;  // ㅁ -> m
			case 17: cur_final_out = ""; next_initial_out = "b"; break;  // ㅂ -> b
			case 18: cur_final_out = "p"; next_initial_out = "s"; break; // ㅄ -> p + s
			case 19: cur_final_out = ""; next_initial_out = "s"; break;  // ㅅ -> s
			case 20: cur_final_out = ""; next_initial_out = "ss"; break; // ㅆ -> ss
			case 21: cur_final_out = "ng"; next_initial_out = ""; break; // ㅇ -> stays ng
			case 22: cur_final_out = ""; next_initial_out = "j"; break;  // ㅈ -> j
			case 23: cur_final_out = ""; next_initial_out = "ch"; break; // ㅊ -> ch
			case 24: cur_final_out = ""; next_initial_out = "k"; break;  // ㅋ -> k
			case 25: cur_final_out = ""; next_initial_out = "t"; break;  // ㅌ -> t
			case 26: cur_final_out = ""; next_initial_out = "p"; break;  // ㅍ -> p
			case 27: cur_final_out = ""; next_initial_out = ""; break;   // ㅎ -> silent before vowel
			default: break;
			}
			return;
		}

		// Case 2: Liquidization (ㄴ + ㄹ -> ll, ㄹ + ㄴ -> ll)
		if (cur.final_c == 4 && next.initial == 5) // ㄴ + ㄹ
		{
			cur_final_out = "l";
			next_initial_out = "l";
			return;
		}
		if (cur.final_c == 8 && next.initial == 2) // ㄹ + ㄴ
		{
			cur_final_out = "l";
			next_initial_out = "l";
			return;
		}

		// Case 2b: Nasal / stop + ㄹ (ㄹ -> n)
		if (next.initial == 5) // next is ㄹ
		{
			if (cur.final_c == 21 || cur.final_c == 16) // ㅇ or ㅁ + ㄹ -> ng/m + n (e.g. Jongno)
			{
				next_initial_out = "n";
				return;
			}
			if (cur.final_c == 1 || cur.final_c == 2 || cur.final_c == 3 ||
			    cur.final_c == 9 || cur.final_c == 24) // ㄱ + ㄹ -> ng + n
			{
				cur_final_out = "ng";
				next_initial_out = "n";
				return;
			}
			if (cur.final_c == 17 || cur.final_c == 26 || cur.final_c == 18 || cur.final_c == 14) // ㅂ + ㄹ -> m + n
			{
				cur_final_out = "m";
				next_initial_out = "n";
				return;
			}
		}

		// Case 3: Nasalization (stops before ㄴ (2) or ㅁ (6))
		const bool next_is_nasal = (next.initial == 2 || next.initial == 6);
		if (next_is_nasal)
		{
			// ㄱ-family (ㄱ 1, ㄲ 2, ㄳ 3, ㄺ 9, ㅋ 24) -> ng
			if (cur.final_c == 1 || cur.final_c == 2 || cur.final_c == 3 ||
			    cur.final_c == 9 || cur.final_c == 24)
			{
				cur_final_out = "ng";
				return;
			}
			// ㄷ-family (ㄷ 7, ㅅ 19, ㅆ 20, ㅈ 22, ㅊ 23, ㅌ 25, ㅎ 27) -> n
			if (cur.final_c == 7 || cur.final_c == 19 || cur.final_c == 20 ||
			    cur.final_c == 22 || cur.final_c == 23 || cur.final_c == 25 ||
			    cur.final_c == 27)
			{
				cur_final_out = "n";
				return;
			}
			// ㅂ-family (ㅂ 17, ㅍ 26, ㅄ 18, ㄿ 14) -> m
			if (cur.final_c == 17 || cur.final_c == 26 || cur.final_c == 18 || cur.final_c == 14)
			{
				cur_final_out = "m";
				return;
			}
		}

		// Case 4: Aspiration with ㅎ (initial 18)
		if (next.initial == 18) // next is ㅎ
		{
			if (cur.final_c == 1 || cur.final_c == 9 || cur.final_c == 24) // ㄱ/ㄺ/ㅋ + ㅎ -> k
			{
				cur_final_out = "";
				next_initial_out = "k";
				return;
			}
			if (cur.final_c == 7 || cur.final_c == 19 || cur.final_c == 20 ||
			    cur.final_c == 22 || cur.final_c == 23 || cur.final_c == 25) // ㄷ/ㅅ/ㅈ/ㅊ/ㅌ + ㅎ -> t
			{
				cur_final_out = "";
				next_initial_out = "t";
				return;
			}
			if (cur.final_c == 17 || cur.final_c == 26 || cur.final_c == 18) // ㅂ/ㅍ/ㅄ + ㅎ -> p
			{
				cur_final_out = "";
				next_initial_out = "p";
				return;
			}
		}
	}

	// Checks whether a string contains any Hangul syllables or jamo characters.
	inline bool contains_hangul(const std::wstring& wstr)
	{
		for (wchar_t c : wstr)
		{
			if (is_hangul_syllable(c) || is_compat_jamo(c) || (c >= 0x1100 && c <= 0x11FF))
			{
				return true;
			}
		}
		return false;
	}

	// Attempts to demangle a UTF-8 string that was created by misinterpreting raw UTF-8 as ISO-8859-1.
	inline bool try_demangle_utf8_as_latin1(const std::string& in, std::string& out)
	{
		std::string candidate_raw;
		candidate_raw.reserve(in.size());

		std::size_t i = 0;
		bool had_high_latin1 = false;

		while (i < in.size())
		{
			const unsigned char c = static_cast<unsigned char>(in[i]);
			if (c < 0x80)
			{
				candidate_raw.push_back(static_cast<char>(c));
				++i;
			}
			else if (c == 0xC2 && i + 1 < in.size())
			{
				const unsigned char c1 = static_cast<unsigned char>(in[i + 1]);
				if ((c1 & 0xC0) != 0x80) return false;
				candidate_raw.push_back(static_cast<char>(c1));
				had_high_latin1 = true;
				i += 2;
			}
			else if (c == 0xC3 && i + 1 < in.size())
			{
				const unsigned char c1 = static_cast<unsigned char>(in[i + 1]);
				if ((c1 & 0xC0) != 0x80) return false;
				candidate_raw.push_back(static_cast<char>(c1 + 0x40));
				had_high_latin1 = true;
				i += 2;
			}
			else
			{
				return false;
			}
		}

		if (!had_high_latin1) return false;

		const std::wstring w = fs::utf8_to_wstring(candidate_raw);
		if (contains_hangul(w))
		{
			out = std::move(candidate_raw);
			return true;
		}

		return false;
	}

	// Romanizes Hangul characters in a UTF-8 string to Latin script using Revised Romanization.
	// Non-Hangul characters (Latin, numbers, punctuation) are preserved as-is.
	inline std::string romanize(const std::string& utf8_str)
	{
		if (utf8_str.empty())
		{
			return {};
		}

		std::wstring wstr = fs::utf8_to_wstring(utf8_str);
		if (!contains_hangul(wstr))
		{
			std::string demangled;
			if (try_demangle_utf8_as_latin1(utf8_str, demangled))
			{
				wstr = fs::utf8_to_wstring(demangled);
			}
			else
			{
				return utf8_str;
			}
		}

		std::string result;
		result.reserve(utf8_str.size() * 2);

		bool new_word = true;
		std::string pending_next_initial;

		for (size_t i = 0; i < wstr.size(); ++i)
		{
			const wchar_t c = wstr[i];

			if (is_hangul_syllable(c))
			{
				const Syllable cur = decompose(c);

				std::string initial_str;
				if (!pending_next_initial.empty())
				{
					initial_str = pending_next_initial;
					pending_next_initial.clear();
				}
				else
				{
					initial_str = kChoseong[cur.initial];
				}

				std::string vowel_str = kJungseong[cur.vowel];
				std::string final_str;

				// Check lookahead for assimilation with next syllable
				if (cur.final_c > 0)
				{
					if (i + 1 < wstr.size() && is_hangul_syllable(wstr[i + 1]))
					{
						const Syllable next = decompose(wstr[i + 1]);
						resolve_assimilation(cur, next, final_str, pending_next_initial);
					}
					else
					{
						final_str = kJongseong[cur.final_c];
					}
				}

				// Assemble syllable
				std::string syl = initial_str + vowel_str + final_str;

				// Capitalize first letter of new word
				if (new_word && !syl.empty())
				{
					syl[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(syl[0])));
					new_word = false;
				}

				result += syl;
			}
			else if (is_compat_jamo(c))
			{
				pending_next_initial.clear();
				std::string jamo_str;
				if (c >= 0x3131 && c <= 0x314E)
				{
					jamo_str = kCompatConsonants[c - 0x3131];
				}
				else if (c >= 0x314F && c <= 0x3163)
				{
					jamo_str = kCompatVowels[c - 0x314F];
				}
				if (new_word && !jamo_str.empty())
				{
					jamo_str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(jamo_str[0])));
					new_word = false;
				}
				result += jamo_str;
			}
			else
			{
				pending_next_initial.clear();
				// Non-Hangul wide char to ASCII/Latin UTF-8 pass-through
				if (c < 0x80)
				{
					result += static_cast<char>(c);
				}
				else
				{
					std::wstring w1(1, c);
					result += fs::wstring_to_utf8(w1);
				}

				if (is_word_boundary_char(c))
				{
					new_word = true;
				}
			}
		}

		return result;
	}
}
