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

#ifndef OTML_H
#define OTML_H

#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <exception>
#include <memory>
#include <algorithm>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

class OTMLNode;
class OTMLDocument;
class OTMLParser;
class OTMLEmitter;

#ifdef __GXX_EXPERIMENTAL_CXX0X__
typedef std::shared_ptr<OTMLNode> OTMLNodePtr;
typedef std::enable_shared_from_this<OTMLNode> OTMLNodeEnableSharedFromThis;
typedef std::shared_ptr<OTMLDocument> OTMLDocumentPtr;
typedef std::weak_ptr<OTMLNode> OTMLNodeWeakPtr;
#else
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
typedef boost::shared_ptr<OTMLNode> OTMLNodePtr;
typedef boost::enable_shared_from_this<OTMLNode> OTMLNodeEnableSharedFromThis;
typedef boost::shared_ptr<OTMLDocument> OTMLDocumentPtr;
typedef boost::weak_ptr<OTMLNode> OTMLNodeWeakPtr;
#endif

typedef std::vector<OTMLNodePtr> OTMLNodeList;

namespace otml_util {
	template<typename T, typename R>
	bool cast(const T& in, R& out) {
		std::stringstream ss;
		ss << in;
		ss >> out;
		return !!ss && ss.eof();
	}

	template<typename T>
	bool cast(const T& in, std::string& out) {
		std::stringstream ss;
		ss << in;
		out = ss.str();
		return true;
	}

	template<>
	inline bool cast(const std::string& in, std::string& out) {
		out = in;
		return true;
	}

	template<>
	inline bool cast(const std::string& in, bool& b) {
		if(in == "true")
			b = true;
		else if(in == "false")
			b = false;
		else
			return false;
		return true;
	}

	template<>
	inline bool cast(const std::string& in, char& c) {
		if(in.length() != 1)
			return false;
		c = in[0];
		return true;
	}

	template<>
	inline bool cast(const std::string& in, long& l) {
		if(in.find_first_not_of("-0123456789") != std::string::npos)
			return false;
		std::size_t t = in.find_last_of('-');
		if(t != std::string::npos && t != 0)
			return false;
		l = atol(in.c_str());
		return true;
	}

	template<>
	inline bool cast(const std::string& in, int& i) {
		long l;
		if(cast(in, l)) {
			i = l;
			return true;
		}
		return false;
	}

	template<>
	inline bool cast(const std::string& in, double& d) {
		if(in.find_first_not_of("-0123456789.") != std::string::npos)
			return false;
		std::size_t t = in.find_last_of('-');
		if(t != std::string::npos &&  t != 0)
			return false;
		t = in.find_first_of('.');
		if(t != std::string::npos && (t == 0 || t == in.length() - 1 || in.find_first_of('.', t + 1) != std::string::npos))
			return false;
		d = atof(in.c_str());
		return true;
	}

	template<>
	inline bool cast(const bool& in, std::string& out) {
		out = (in ? "true" : "false");
		return true;
	}

	class BadCast : public std::bad_cast {
	public:
		virtual ~BadCast() throw() { }
		virtual const char* what() { return "failed to cast value"; }
	};

	template<typename R, typename T>
	R safeCast(const T& t) {
		R r;
		if(!cast(t, r))
			throw BadCast();
		return r;
	}
};


class OTMLException : public std::exception {
public:
	OTMLException(const std::string& error) : m_what(error) { }
	OTMLException(const OTMLNodePtr& node, const std::string& error);
	OTMLException(const OTMLDocumentPtr& doc, const std::string& error, int line = -1);
	virtual ~OTMLException() throw() { };

	virtual const char* what() const throw() { return m_what.c_str(); }

protected:
	std::string m_what;
};

class OTMLNode : public OTMLNodeEnableSharedFromThis {
public:
	virtual ~OTMLNode() { }

	static OTMLNodePtr create(std::string tag = "", bool unique = false);
	static OTMLNodePtr create(std::string tag, std::string value);

	std::string tag() const { return m_tag; }
	int size() const { return m_children.size(); }
	OTMLNodePtr parent() const { return m_parent.lock(); }
	std::string source() const { return m_source; }
	std::string rawValue() const { return m_value; }

	bool isUnique() const { return m_unique; }
	bool isNull() const { return m_null; }

	bool hasTag() const { return !m_tag.empty(); }
	bool hasValue() const { return !m_value.empty(); }
	bool hasChildren() const;
	bool hasChildAt(const std::string& childTag) { return !!get(childTag); }
	bool hasChildAtIndex(int childIndex) { return !!getIndex(childIndex); }

	void setTag(std::string tag) { m_tag = tag; }
	void setValue(const std::string& value) { m_value = value; }
	void setNull(bool null) { m_null = null; }
	void setUnique(bool unique) { m_unique = unique; }
	void setParent(const OTMLNodePtr& parent) { m_parent = parent; }
	void setSource(const std::string& source) { m_source = source; }

	OTMLNodePtr get(const std::string& childTag) const;
	OTMLNodePtr getIndex(int childIndex) const;

	OTMLNodePtr at(const std::string& childTag);
	OTMLNodePtr atIndex(int childIndex);

	void addChild(const OTMLNodePtr& newChild);
	bool removeChild(const OTMLNodePtr& oldChild);
	bool replaceChild(const OTMLNodePtr& oldChild, const OTMLNodePtr& newChild);
	void merge(const OTMLNodePtr& node);
	void copy(const OTMLNodePtr& node);
	void clear();

	OTMLNodeList children() const;
	OTMLNodePtr clone() const;

	template<typename T>
	T value();
	template<typename T>
	T valueAt(const std::string& childTag);
	template<typename T>
	T valueAtIndex(int childIndex);
	template<typename T>
	T valueAt(const std::string& childTag, const T& def);
	template<typename T>
	T valueAtIndex(int childIndex, const T& def);

	template<typename T>
	void write(const T& v);
	template<typename T>
	void writeAt(const std::string& childTag, const T& v);
	template<typename T>
	void writeIn(const T& v);

	virtual std::string emit();

protected:
	OTMLNode() : m_unique(false), m_null(false) { }

	OTMLNodeList m_children;
	OTMLNodeWeakPtr m_parent;
	std::string m_tag;
	std::string m_value;
	std::string m_source;
	bool m_unique;
	bool m_null;
};

class OTMLDocument : public OTMLNode {
public:
	virtual ~OTMLDocument() { }
	static OTMLDocumentPtr create();
	static OTMLDocumentPtr parse(const std::string& fileName);
	static OTMLDocumentPtr parse(std::istream& in, const std::string& source);
	std::string emit();
	bool save(const std::string& fileName);

private:
	OTMLDocument() { }
};

class OTMLParser {
public:
	OTMLParser(OTMLDocumentPtr doc, std::istream& in) :
		currentDepth(0), currentLine(0),
		doc(doc), currentParent(doc),
		in(in) { }
	void parse();

private:
	std::string getNextLine();
	int getLineDepth(const std::string& line, bool multilining = false);
	void parseLine(std::string line);
	void parseNode(const std::string& data);

	int currentDepth;
	int currentLine;
	OTMLDocumentPtr doc;
	OTMLNodePtr currentParent;
	OTMLNodePtr previousNode;
	std::istream& in;
};

class OTMLEmitter {
public:
	static std::string emitNode(const OTMLNodePtr& node, int currentDepth = -1);
};

inline OTMLException::OTMLException(const OTMLNodePtr& node, const std::string& error) {
	std::stringstream ss;
	ss << "OTML error";
	if(!node->source().empty())
		ss << " in '" << node->source() << "'";
	ss << ": " << error;
	m_what = ss.str();
}

inline OTMLException::OTMLException(const OTMLDocumentPtr& doc, const std::string& error, int line) {
	std::stringstream ss;
	ss << "OTML error";
	if(doc && !doc->source().empty()) {
		ss << " in '" << doc->source() << "'";
		if(line >= 0)
			ss << " at line " << line;
	}
	ss << ": " << error;
	m_what = ss.str();
}

inline OTMLNodePtr OTMLNode::create(std::string tag, bool unique) {
	OTMLNodePtr node(new OTMLNode);
	node->setTag(tag);
	node->setUnique(unique);
	return node;
}

inline OTMLNodePtr OTMLNode::create(std::string tag, std::string value) {
	OTMLNodePtr node(new OTMLNode);
	node->setTag(tag);
	node->setValue(value);
	node->setUnique(true);
	return node;
}

inline bool OTMLNode::hasChildren() const {
	int count = 0;
	for(OTMLNodeList::const_iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
		const OTMLNodePtr& child = *it;
		if(!child->isNull())
			count++;
	}
	return count > 0;
}

inline OTMLNodePtr OTMLNode::get(const std::string& childTag) const {
	for(OTMLNodeList::const_iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
		const OTMLNodePtr& child = *it;
		if(child->tag() == childTag && !child->isNull())
			return child;
	}
	return OTMLNodePtr();
}

inline OTMLNodePtr OTMLNode::getIndex(int childIndex) const {
	if(childIndex < size() && childIndex >= 0)
		return m_children[childIndex];
	return OTMLNodePtr();
}

inline OTMLNodePtr OTMLNode::at(const std::string& childTag) {
	OTMLNodePtr res;
	for(OTMLNodeList::iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
		const OTMLNodePtr& child = *it;
		if(child->tag() == childTag && !child->isNull()) {
			res = child;
			break;
		}
	}
	if(!res) {
		std::stringstream ss;
		ss << "child node with tag '" << childTag << "' not found";
		throw OTMLException(shared_from_this(), ss.str());
	}
	return res;
}

inline OTMLNodePtr OTMLNode::atIndex(int childIndex) {
	if(childIndex >= size() || childIndex < 0) {
		std::stringstream ss;
		ss << "child node with index '" << childIndex << "' not found";
		throw OTMLException(shared_from_this(), ss.str());
	}
	return m_children[childIndex];
}

inline void OTMLNode::addChild(const OTMLNodePtr& newChild) {
	if(newChild->hasTag()) {
		for(OTMLNodeList::iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
			const OTMLNodePtr& node = *it;
			if(node->tag() == newChild->tag() && (node->isUnique() || newChild->isUnique())) {
				newChild->setUnique(true);

				if(node->hasChildren() && newChild->hasChildren()) {
					OTMLNodePtr tmpNode = node->clone();
					tmpNode->merge(newChild);
					newChild->copy(tmpNode);
				}

				replaceChild(node, newChild);
				OTMLNodeList::iterator it = m_children.begin();
				while(it != m_children.end()) {
					OTMLNodePtr node = (*it);
					if(node != newChild && node->tag() == newChild->tag()) {
						node->setParent(OTMLNodePtr());
						it = m_children.erase(it);
					}
					else
						++it;
				}
				return;
			}
		}
	}
	m_children.push_back(newChild);
	newChild->setParent(shared_from_this());
}

inline bool OTMLNode::removeChild(const OTMLNodePtr& oldChild) {
	OTMLNodeList::iterator it = std::find(m_children.begin(), m_children.end(), oldChild);
	if(it != m_children.end()) {
		m_children.erase(it);
		oldChild->setParent(OTMLNodePtr());
		return true;
	}
	return false;
}

inline bool OTMLNode::replaceChild(const OTMLNodePtr& oldChild, const OTMLNodePtr& newChild) {
	OTMLNodeList::iterator it = std::find(m_children.begin(), m_children.end(), oldChild);
	if(it != m_children.end()) {
		oldChild->setParent(OTMLNodePtr());
		newChild->setParent(shared_from_this());
		it = m_children.erase(it);
		m_children.insert(it, newChild);
		return true;
	}
	return false;
}

inline void OTMLNode::copy(const OTMLNodePtr& node)
{
	setTag(node->tag());
	setValue(node->rawValue());
	setUnique(node->isUnique());
	setNull(node->isNull());
	setSource(node->source());
	clear();
	for(OTMLNodeList::iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
		const OTMLNodePtr& child = *it;
		addChild(child->clone());
	}
}

inline void OTMLNode::merge(const OTMLNodePtr& node) {
	for(OTMLNodeList::iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
		const OTMLNodePtr& child = *it;
		addChild(child->clone());
	}
	setTag(node->tag());
	setSource(node->source());
}

inline void OTMLNode::clear() {
	for(OTMLNodeList::iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
		const OTMLNodePtr& child = *it;
		child->setParent(OTMLNodePtr());
	}
	m_children.clear();
}

inline OTMLNodeList OTMLNode::children() const {
	OTMLNodeList children;
	for(OTMLNodeList::const_iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
		const OTMLNodePtr& child = *it;
		if(!child->isNull())
			children.push_back(child);
	}
	return children;
}

inline OTMLNodePtr OTMLNode::clone() const {
	OTMLNodePtr myClone(new OTMLNode);
	myClone->setTag(m_tag);
	myClone->setValue(m_value);
	myClone->setUnique(m_unique);
	myClone->setNull(m_null);
	myClone->setSource(m_source);
	for(OTMLNodeList::const_iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
		const OTMLNodePtr& child = *it;
		myClone->addChild(child->clone());
	}
	return myClone;
}

inline std::string OTMLNode::emit() {
	return OTMLEmitter::emitNode(shared_from_this(), 0);
}

template<>
inline std::string OTMLNode::value() {
	std::string value = m_value;
	if(boost::starts_with(value, "\"") && boost::ends_with(value, "\"")) {
		value = value.substr(1, value.length() - 2);
		boost::replace_all(value, "\\\\", "\\");
		boost::replace_all(value, "\\\"", "\"");
		boost::replace_all(value, "\\t", "\t");
		boost::replace_all(value, "\\n", "\n");
		boost::replace_all(value, "\\'", "\'");
	}
	return value;
}

template<typename T>
T OTMLNode::value() {
	T ret;
	if(!otml_util::cast(m_value, ret))
		throw OTMLException(shared_from_this(), "failed to cast node value");
	return ret;
}

template<typename T>
T OTMLNode::valueAt(const std::string& childTag) {
	OTMLNodePtr node = at(childTag);
	return node->value<T>();
}

template<typename T>
T OTMLNode::valueAtIndex(int childIndex) {
	OTMLNodePtr node = atIndex(childIndex);
	return node->value<T>();
}

template<typename T>
T OTMLNode::valueAt(const std::string& childTag, const T& def) {
	if(OTMLNodePtr node = get(childTag))
		if(!node->isNull())
			return node->value<T>();
	return def;
}

template<typename T>
T OTMLNode::valueAtIndex(int childIndex, const T& def) {
	if(OTMLNodePtr node = getIndex(childIndex))
		return node->value<T>();
	return def;
}

template<typename T>
void OTMLNode::write(const T& v) {
	m_value = otml_util::safeCast<std::string>(v);
}

template<typename T>
void OTMLNode::writeAt(const std::string& childTag, const T& v) {
	OTMLNodePtr child = OTMLNode::create(childTag);
	child->setUnique(true);
	child->write<T>(v);
	addChild(child);
}

template<typename T>
void OTMLNode::writeIn(const T& v) {
	OTMLNodePtr child = OTMLNode::create();
	child->write<T>(v);
	addChild(child);
}

inline OTMLDocumentPtr OTMLDocument::create() {
	OTMLDocumentPtr doc(new OTMLDocument);
	doc->setTag("doc");
	return doc;
}

inline OTMLDocumentPtr OTMLDocument::parse(const std::string& fileName) {
	std::ifstream fin(fileName.c_str());
	if(!fin.good()) {
		std::stringstream ss;
		ss << "failed to open file " << fileName;
		throw OTMLException(ss.str());
	}
	return parse(fin, fileName);
}

inline OTMLDocumentPtr OTMLDocument::parse(std::istream& in, const std::string& source) {
	OTMLDocumentPtr doc(new OTMLDocument);
	doc->setSource(source);
	OTMLParser parser(doc, in);
	parser.parse();
	return doc;
}

inline std::string OTMLDocument::emit() {
	return OTMLEmitter::emitNode(shared_from_this()) + "\n";
}

inline bool OTMLDocument::save(const std::string& fileName) {
	m_source = fileName;
	std::ofstream fout(fileName.c_str());
	if(fout.good()) {
		fout << emit();
		fout.close();
		return true;
	}
	return false;
}

inline std::string OTMLEmitter::emitNode(const OTMLNodePtr& node, int currentDepth) {
	std::stringstream ss;
	if(currentDepth >= 0) {
		for(int i = 0; i<currentDepth; ++i)
			ss << "  ";
		if(node->hasTag()) {
			ss << node->tag();
			if(node->hasValue() || node->isUnique() || node->isNull())
				ss << ":";
		}
		else
			ss << "-";
		if(node->isNull())
			ss << " ~";
		else if(node->hasValue()) {
			ss << " ";
			std::string value = node->rawValue();
			if(value.find("\n") != std::string::npos) {
				if(value[value.length() - 1] == '\n' && value[value.length() - 2] == '\n')
					ss << "|+";
				else if(value[value.length() - 1] == '\n')
					ss << "|";
				else
					ss << "|-";
				for(std::size_t pos = 0; pos < value.length(); ++pos) {
					ss << "\n";
					for(int i = 0; i<currentDepth + 1; ++i)
						ss << "  ";
					while(pos < value.length()) {
						if(value[pos] == '\n')
							break;
						ss << value[pos++];
					}
				}
			}
			else
				ss << value;
		}
	}
	for(int i = 0; i<node->size(); ++i) {
		if(currentDepth >= 0 || i != 0)
			ss << "\n";
		ss << emitNode(node->atIndex(i), currentDepth + 1);
	}
	return ss.str();
}

inline void OTMLParser::parse() {
	if(!in.good())
		throw OTMLException(doc, "cannot read from input stream");
	while(!in.eof())
		parseLine(getNextLine());
}

inline std::string OTMLParser::getNextLine() {
	currentLine++;
	std::string line;
	std::getline(in, line);
	return line;
}

inline int OTMLParser::getLineDepth(const std::string& line, bool multilining) {
	std::size_t spaces = 0;
	while(line[spaces] == ' ')
		spaces++;

	int depth = spaces / 2;
	if(!multilining || depth <= currentDepth) {
		if(line[spaces] == '\t')
			throw OTMLException(doc, "indentation with tabs are not allowed", currentLine);
		if(spaces % 2 != 0)
			throw OTMLException(doc, "must indent every 2 spaces", currentLine);
	}
	return depth;
}

inline void OTMLParser::parseLine(std::string line) {
	int depth = getLineDepth(line);
	if(depth == -1)
		return;
	boost::trim(line);
	if(line.empty())
		return;
	if(line.substr(0, 2) == "//")
		return;
	if(depth == currentDepth + 1) {
		currentParent = previousNode;
	}
	else if(depth < currentDepth) {
		for(int i = 0; i<currentDepth - depth; ++i)
			currentParent = currentParent->parent();
	}
	else if(depth != currentDepth)
		throw OTMLException(doc, "invalid indentation depth, are you indenting correctly?", currentLine);
	currentDepth = depth;
	parseNode(line);
}

inline void OTMLParser::parseNode(const std::string& data) {
	std::string tag;
	std::string value;
	std::size_t dotsPos = data.find_first_of(':');
	int nodeLine = currentLine;
	if(!data.empty() && data[0] == '-') {
		value = data.substr(1);
		boost::trim(value);
	}
	else if(dotsPos != std::string::npos) {
		tag = data.substr(0, dotsPos);
		if(data.size() > dotsPos + 1)
			value = data.substr(dotsPos + 1);
	}
	else {
		tag = data;
	}
	boost::trim(tag);
	boost::trim(value);
	if(value == "|" || value == "|-" || value == "|+") {
		std::string multiLineData;
		do {
			size_t lastPos = in.tellg();
			std::string line = getNextLine();
			int depth = getLineDepth(line, true);
			if(depth > currentDepth) {
				multiLineData += line.substr((currentDepth + 1) * 2);
			}
			else {
				boost::trim(line);
				if(!line.empty()) {
					in.seekg(lastPos, std::ios::beg);
					currentLine--;
					break;
				}
			}
			multiLineData += "\n";
		} while(!in.eof());
		if(value == "|" || value == "|-") {
			int lastPos = multiLineData.length();
			while(multiLineData[--lastPos] == '\n')
				multiLineData.erase(lastPos, 1);

			if(value == "|")
				multiLineData.append("\n");
		}
		value = multiLineData;
	}
	OTMLNodePtr node = OTMLNode::create(tag);
	node->setUnique(dotsPos != std::string::npos);
	node->setTag(tag);
	node->setSource(doc->source() + ":" + otml_util::safeCast<std::string>(nodeLine));
	if(value == "~")
		node->setNull(true);
	else {
		if(boost::starts_with(value, "[") && boost::ends_with(value, "]")) {
			typedef boost::tokenizer<boost::escaped_list_separator<char> > Tokenizer;
			std::string tmp = value.substr(1, value.length() - 2);
			Tokenizer tok(tmp);
			for(Tokenizer::iterator it = tok.begin(), end = tok.end(); it != end; ++it) {
				std::string v = *it;
				boost::trim(v);
				node->writeIn(v);
			}
		}
		else
			node->setValue(value);
	}

	currentParent->addChild(node);
	previousNode = node;
}

#endif
