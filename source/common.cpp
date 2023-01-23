//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "common.h"
#include "math.h"

#include <sstream>
#include <random>

// random generator
std::mt19937& getRandomGenerator()
{
	static std::random_device rd;
	static std::mt19937 generator(rd());
	return generator;
}

int32_t uniform_random(int32_t minNumber, int32_t maxNumber)
{
	static std::uniform_int_distribution<int32_t> uniformRand;
	if(minNumber == maxNumber) {
		return minNumber;
	} else if(minNumber > maxNumber) {
		std::swap(minNumber, maxNumber);
	}
	return uniformRand(getRandomGenerator(), std::uniform_int_distribution<int32_t>::param_type(minNumber, maxNumber));
}

int32_t uniform_random(int32_t maxNumber)
{
	return uniform_random(0, maxNumber);
}

//
std::string i2s(const int _i)
{
	static std::stringstream ss;
	ss.str("");
	ss << _i;
	return ss.str();
}

std::string f2s(const double _d)
{
	static std::stringstream ss;
	ss.str("");
	ss << _d;
	return ss.str();
}

int s2i(const std::string s)
{
	return atoi(s.c_str());
}

double s2f(const std::string s)
{
	return atof(s.c_str());
}

wxString i2ws(const int _i)
{
	wxString str;
	str << _i;
	return str;
}

wxString f2ws(const double _d)
{
	wxString str;
	str << _d;
	return str;
}

int ws2i(const wxString s)
{
	long _i;
	if(s.ToLong(&_i))
		return int(_i);
	return 0;
}

double ws2f(const wxString s)
{
	double _d;
	if(s.ToDouble(&_d))
		return _d;
	return 0.0;
}

void replaceString(std::string& str, const std::string sought, const std::string replacement)
{
	size_t pos = 0;
	size_t start = 0;
	size_t soughtLen = sought.length();
	size_t replaceLen = replacement.length();
	while((pos = str.find(sought, start)) != std::string::npos) {
		str = str.substr(0, pos) + replacement + str.substr(pos + soughtLen);
		start = pos + replaceLen;
	}
}

void trim_right(std::string& source, const std::string& t)
{
	source.erase(source.find_last_not_of(t)+1);
}

void trim_left(std::string& source, const std::string& t)
{
	source.erase(0, source.find_first_not_of(t));
}

void to_lower_str(std::string& source)
{
	std::transform(source.begin(), source.end(), source.begin(), tolower);
}

void to_upper_str(std::string& source)
{
	std::transform(source.begin(), source.end(), source.begin(), toupper);
}

std::string as_lower_str(const std::string& other)
{
	std::string ret = other;
	to_lower_str(ret);
	return ret;
}

std::string as_upper_str(const std::string& other)
{
	std::string ret = other;
	to_upper_str(ret);
	return ret;
}

bool isFalseString(std::string& str)
{
	if(str == "false" || str == "0" || str == "" || str == "no" || str == "not") {
		return true;
	}
	return false;
}

bool isTrueString(std::string& str)
{
	return !isFalseString(str);
}

int random(int low, int high)
{
	if(low == high) {
		return low;
	}

	if(low > high) {
		return low;
	}

	int range = high - low;

	double dist = double(mt_randi()) / 0xFFFFFFFF;
	return low + std::min(range, int((1 + range) * dist));
}

int random(int high)
{
	return random(0,high);
}

std::wstring string2wstring(const std::string& utf8string)
{
	wxString s(utf8string.c_str(), wxConvUTF8);
	return std::wstring((const wchar_t*)s.c_str());
}

std::string wstring2string(const std::wstring& widestring)
{
	wxString s(widestring.c_str());
	return std::string((const char*)s.mb_str(wxConvUTF8));
}

bool posFromClipboard(int& x, int& y, int& z)
{
	bool done = false;

	if(wxTheClipboard->Open()) {
		if(wxTheClipboard->IsSupported(wxDF_TEXT)) {
			std::vector<int> values;
			wxTextDataObject data;
			wxTheClipboard->GetData(data);
			wxString text = data.GetText();

			if(text.size() < 50) {
				bool r = false;
				wxString sv;

				for(size_t s = 0; s < text.size(); ++s) {
					if(text[s] >= '0' && text[s] <= '9') {
						sv << text[s];
						r = true;
					} else if(r) {
						values.push_back(ws2i(sv));
						sv.Clear();
						r = false;

						if(values.size() >= 3)
							break;
					}
				}
			}

			if(values.size() == 3) {
				x = values[0];
				y = values[1];
				z = values[2];
				done = true;
			}
		}
		wxTheClipboard->Close();
	}
	return done;
}

bool posToClipboard(int x, int y, int z, int format)
{
	if (!wxTheClipboard->Open())
		return false;

	wxTextDataObject* data = new wxTextDataObject();

	switch (format) {
		case 0:
			data->SetText(wxString::Format("{x = %d, y = %d, z = %d}", x, y, z));
			break;
		case 1:
			data->SetText(wxString::Format("{\"x\":%d, \"y\":%d, \"z\":%d}", x, y, z));
			break;
		case 2:
			data->SetText(wxString::Format("%d, %d, %d", x, y, z));
			break;
		case 3:
			data->SetText(wxString::Format("(%d, %d, %d)", x, y, z));
			break;
		case 4:
			data->SetText(wxString::Format("Position(%d, %d, %d)", x, y, z));
			break;
		default:
			wxTheClipboard->Close();
			return false;
	}

	wxTheClipboard->SetData(data);
	wxTheClipboard->Close();
	return true;
}

bool posToClipboard(int fromx, int fromy, int fromz, int tox, int toy, int toz)
{
	if (!wxTheClipboard->Open())
		return false;

	std::ostringstream clip;
	clip << "{";
	clip << "fromx = " << fromx << ", ";
	clip << "tox = " << tox << ", ";
	clip << "fromy = " << fromy << ", ";
	clip << "toy = " << toy << ", ";
	if (fromz != toz) {
		clip << "fromz = " << fromz << ", ";
		clip << "toz = " << toz;
	}
	else
		clip << "z = " << fromz;
	clip << "}";

	wxTheClipboard->SetData(new wxTextDataObject(clip.str()));
	wxTheClipboard->Close();
	return true;
}

wxString b2yn(bool value)
{
	return value ? "Yes" : "No";
}
