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

#include "filehandle.h"

#include <stdio.h>
#include <assert.h>

uint8_t NodeFileWriteHandle::NODE_START = ::NODE_START;
uint8_t NodeFileWriteHandle::NODE_END = ::NODE_END;
uint8_t NodeFileWriteHandle::ESCAPE_CHAR = ::ESCAPE_CHAR;

void FileHandle::close()
{
	if(file) {
		fclose(file);
		file = nullptr;
	}
}

std::string FileHandle::getErrorMessage()
{
	switch(error_code) {
		case FILE_NO_ERROR: return "No error";
		case FILE_COULD_NOT_OPEN: return "Could not open file";
		case FILE_INVALID_IDENTIFIER: return "File magic number not recognized";
		case FILE_STRING_TOO_LONG: return "Too long string encountered";
		case FILE_READ_ERROR: return "Failed to read from file";
		case FILE_WRITE_ERROR: return "Failed to write to file";
		case FILE_SYNTAX_ERROR: return "Node file syntax error";
		case FILE_PREMATURE_END: return "File end encountered unexpectedly";
	}
	if(file == nullptr) {
		return "Could not open file (2)";
	}
	if(ferror(file)) {
		return "Internal file error #" + i2s(ferror(file));
	}
	return "No error";
}

//=============================================================================
// File read handle

FileReadHandle::FileReadHandle(const std::string& name) : file_size(0)
{
#if defined __VISUALC__ && defined _UNICODE
	file = _wfopen(string2wstring(name).c_str(), L"rb");
#else
	file = fopen(name.c_str(), "rb");
#endif
	if(!file || ferror(file)) {
		error_code = FILE_COULD_NOT_OPEN;
	} else {
		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, 0, SEEK_SET);
	}
}

FileReadHandle::~FileReadHandle()
{
	////
}

void FileReadHandle::close()
{
	file_size = 0;
	FileHandle::close();
}

bool FileReadHandle::getRAW(uint8_t* ptr, size_t sz)
{
	size_t o = fread(ptr, 1, sz, file);
	if(o != sz) {
		error_code = FILE_READ_ERROR;
		return false;
	}
	return true;
}

bool FileReadHandle::getRAW(std::string& str, size_t sz)
{
	str.resize(sz);
	size_t o = fread(const_cast<char*>(str.data()), 1, sz, file);
	if(o != sz) {
		error_code = FILE_READ_ERROR;
		return false;
	}
	return true;
}

bool FileReadHandle::getString(std::string& str)
{
	uint16_t sz;
	if(!getU16(sz)) {
		error_code = FILE_READ_ERROR;
		return false;
	}
	return getRAW(str, sz);
}

bool FileReadHandle::getLongString(std::string& str)
{
	uint32_t sz;
	if(!getU32(sz)) {
		error_code = FILE_READ_ERROR;
		return false;
	}
	return getRAW(str, sz);
}

bool FileReadHandle::seek(size_t offset)
{
	return fseek(file, long(offset), SEEK_SET) == 0;
}

bool FileReadHandle::seekRelative(size_t offset)
{
	return fseek(file, long(offset), SEEK_CUR) == 0;
}

//=============================================================================
// Node file read handle

NodeFileReadHandle::NodeFileReadHandle() :
	last_was_start(false),
	cache(nullptr),
	cache_size(32768),
	cache_length(0),
	local_read_index(0),
	root_node(nullptr)
{
	////
}

NodeFileReadHandle::~NodeFileReadHandle()
{
	while(!unused.empty()) {
		free(unused.top());
		unused.pop();
	}
}

BinaryNode* NodeFileReadHandle::getNode(BinaryNode* parent)
{
	void* mem;
	if(unused.empty()) {
		mem = malloc(sizeof(BinaryNode));
	} else {
		mem = unused.top();
		unused.pop();
	}
	return new(mem) BinaryNode(this, parent);
}

void NodeFileReadHandle::freeNode(BinaryNode* node)
{
	if(node) {
		node->~BinaryNode();
		unused.push(node);
	}
}

//=============================================================================
// Memory based node file read handle

MemoryNodeFileReadHandle::MemoryNodeFileReadHandle(const uint8_t* data, size_t size)
{
	assign(data, size);
}

void MemoryNodeFileReadHandle::assign(const uint8_t* data, size_t size)
{
	freeNode(root_node);
	root_node = nullptr;
	// Highly volatile, but we know we're not gonna modify
	cache = const_cast<uint8_t*>(data);
	cache_size = cache_length = size;
	local_read_index = 0;
}

MemoryNodeFileReadHandle::~MemoryNodeFileReadHandle()
{
	freeNode(root_node);
}

void MemoryNodeFileReadHandle::close()
{
	assign(nullptr, 0);
}

bool MemoryNodeFileReadHandle::renewCache()
{
	return false;
}

BinaryNode* MemoryNodeFileReadHandle::getRootNode()
{
	assert(root_node == nullptr); // You should never do this twice

	local_read_index++; // Skip first NODE_START
	last_was_start = true;
	root_node = getNode(nullptr);
	root_node->load();
	return root_node;
}

//=============================================================================
// File based node file read handle

DiskNodeFileReadHandle::DiskNodeFileReadHandle(const std::string& name, const std::vector<std::string>& acceptable_identifiers) :
	file_size(0)
{
#if defined __VISUALC__ && defined _UNICODE
	file = _wfopen(string2wstring(name).c_str(), L"rb");
#else
	file = fopen(name.c_str(), "rb");
#endif
	if(!file || ferror(file)) {
		error_code = FILE_COULD_NOT_OPEN;
	} else {
		char ver[4];
		if(fread(ver, 1, 4, file) != 4) {
			fclose(file);
			error_code = FILE_SYNTAX_ERROR;
			return;
		}

		// 0x00 00 00 00 is accepted as a wildcard version

		if(ver[0] != 0 || ver[1] != 0 || ver[2] != 0 || ver[3] != 0) {
			bool accepted = false;
			for(std::vector<std::string>::const_iterator id_iter = acceptable_identifiers.begin(); id_iter != acceptable_identifiers.end(); ++id_iter) {
				if(memcmp(ver, id_iter->c_str(), 4) == 0) {
					accepted = true;
					break;
				}
			}

			if(!accepted) {
				fclose(file);
				error_code = FILE_SYNTAX_ERROR;
				return;
			}
		}

		fseek(file, 0, SEEK_END);
		file_size = ftell(file);
		fseek(file, 4, SEEK_SET);
	}
}

DiskNodeFileReadHandle::~DiskNodeFileReadHandle()
{
	close();
}

void DiskNodeFileReadHandle::close()
{
	freeNode(root_node);
	file_size = 0;
	FileHandle::close();
	free(cache);
}

bool DiskNodeFileReadHandle::renewCache()
{
	if(!cache) {
		cache = (uint8_t*)malloc(cache_size);
	}
	cache_length = fread(cache, 1, cache_size, file);

	if(cache_length == 0 || ferror(file)) {
		return false;
	}
	local_read_index = 0;
	return true;
}

BinaryNode* DiskNodeFileReadHandle::getRootNode()
{
	assert(root_node == nullptr); // You should never do this twice
	uint8_t first;
	fread(&first, 1, 1, file);
	if(first == NODE_START) {
		root_node = getNode(nullptr);
		root_node->load();
		return root_node;
	} else {
		error_code = FILE_SYNTAX_ERROR;
		return nullptr;
	}
}

//=============================================================================
// Binary file node

BinaryNode::BinaryNode(NodeFileReadHandle* file, BinaryNode* parent) :
	read_offset(0),
	file(file),
	parent(parent),
	child(nullptr)
{
	////
}

BinaryNode::~BinaryNode()
{
	file->freeNode(child);
}

BinaryNode* BinaryNode::getChild()
{
	ASSERT(file);
	ASSERT(child == nullptr);

	if(file->last_was_start) {
		child = file->getNode(this);
		child->load();
		return child;
	}
	return nullptr;
}

bool BinaryNode::getRAW(uint8_t* ptr, size_t sz)
{
	if(read_offset + sz > data.size()) {
		read_offset = data.size();
		return false;
	}
	memcpy(ptr, data.data() + read_offset, sz);
	read_offset += sz;
	return true;
}

bool BinaryNode::getRAW(std::string& str, size_t sz)
{
	if(read_offset + sz > data.size()) {
		read_offset = data.size();
		return false;
	}
	str.assign(data.data() + read_offset, sz);
	read_offset += sz;
	return true;
}

bool BinaryNode::getString(std::string& str)
{
	uint16_t len;
	if(!getU16(len)) {
		return false;
	}
	return getRAW(str, len);
}

bool BinaryNode::getLongString(std::string& str)
{
	uint32_t len;
	if(!getU32(len)) {
		return false;
	}
	return getRAW(str, len);
}

BinaryNode* BinaryNode::advance()
{
	// Advance this to the next position
	ASSERT(file);

	if(file->error_code != FILE_NO_ERROR)
		return nullptr;

	if(child == nullptr) {
		getChild();
	}
	// We need to move the cursor to the next node, since we're still iterating our child node!
	while(child) {
		// both functions modify ourselves and sets child to nullptr, so loop will be aborted
		// possibly change to assignment ?
		child->getChild();
		child->advance();
	}

	if(file->last_was_start) {
		return nullptr;
	} else {
		// Last was end (0xff)
		// Read next byte to decide if there is another node following this
		uint8_t*& cache = file->cache;
		size_t& cache_length = file->cache_length;
		size_t& local_read_index = file->local_read_index;

		if(local_read_index >= cache_length) {
			if(!file->renewCache()) {
				// Failed to renew, exit
				parent->child = nullptr;
				file->freeNode(this);
				return nullptr;
			}
		}

		uint8_t op = cache[local_read_index];
		++local_read_index;

		if(op == NODE_START) {
			// Another node follows this.
			// Load this node as the next one
			read_offset = 0;
			data.clear();
			load();
			return this;
		} else if(op == NODE_END) {
			// End of this child-tree
			parent->child = nullptr;
			file->last_was_start = false;
			file->freeNode(this);
			return nullptr;
		} else {
			file->error_code = FILE_SYNTAX_ERROR;
			return nullptr;
		}
	}
}

void BinaryNode::load()
{
	ASSERT(file);
	// Read until next node starts
	uint8_t*& cache = file->cache;
	size_t& cache_length = file->cache_length;
	size_t& local_read_index = file->local_read_index;
	while(true) {
		if(local_read_index >= cache_length) {
			if(!file->renewCache()) {
				// Failed to renew, exit
				file->error_code = FILE_PREMATURE_END;
				return;
			}
		}

		uint8_t op = cache[local_read_index];
		++local_read_index;

		switch(op) {
			case NODE_START: {
				file->last_was_start = true;
				return;
			}

			case NODE_END: {
				file->last_was_start = false;
				return;
			}

			case ESCAPE_CHAR: {
				if(local_read_index >= cache_length) {
					if(!file->renewCache()) {
						// Failed to renew, exit
						file->error_code = FILE_PREMATURE_END;
						return;
					}
				}

				op = cache[local_read_index];
				++local_read_index;
				break;
			}

			default:
				break;
		}
		//std::cout << "Appending..." << std::endl;
		data.append(1, op);
	}
}

//=============================================================================
// node file binary write handle

FileWriteHandle::FileWriteHandle(const std::string& name)
{
#if defined __VISUALC__ && defined _UNICODE
	file = _wfopen(string2wstring(name).c_str(), L"wb");
#else
	file = fopen(name.c_str(), "wb");
#endif
	if(file == nullptr || ferror(file)) {
		error_code = FILE_COULD_NOT_OPEN;
	}
}

FileWriteHandle::~FileWriteHandle()
{
	////
}

bool FileWriteHandle::addString(const std::string& str)
{
	if(str.size() > 0xFFFF) {
		error_code = FILE_STRING_TOO_LONG;
		return false;
	}
	addU16(uint16_t(str.size()));
	addRAW(str);
	return true;
}

bool FileWriteHandle::addString(const char* str)
{
	size_t len = strlen(str);
	if(len > 0xFFFF) {
		error_code = FILE_STRING_TOO_LONG;
		return false;
	}
	addU16(uint16_t(len));
	fwrite(str, 1, len, file);
	return true;
}

bool FileWriteHandle::addLongString(const std::string& str)
{
	addU32(uint32_t(str.size()));
	addRAW(str);
	return true;
}

bool FileWriteHandle::addRAW(const std::string& str)
{
	fwrite(str.c_str(), 1, str.size(), file);
	return ferror(file) == 0;
}

bool FileWriteHandle::addRAW(const uint8_t* ptr, size_t sz)
{
	fwrite(ptr, 1, sz, file);
	return ferror(file) == 0;
}

//=============================================================================
// Disk based node file write handle

DiskNodeFileWriteHandle::DiskNodeFileWriteHandle(const std::string& name, const std::string& identifier)
{
#if defined __VISUALC__ && defined _UNICODE
	file = _wfopen(string2wstring(name).c_str(), L"wb");
#else
	file = fopen(name.c_str(), "wb");
#endif
	if(!file || ferror(file)) {
		error_code = FILE_COULD_NOT_OPEN;
		return;
	}
	if(identifier.length() != 4) {
		error_code = FILE_INVALID_IDENTIFIER;
		return;
	}

	fwrite(identifier.c_str(), 1, 4, file);
	if(!cache) {
		cache = (uint8_t*)malloc(cache_size+1);
	}
	local_write_index = 0;
}

DiskNodeFileWriteHandle::~DiskNodeFileWriteHandle()
{
	close();
}

void DiskNodeFileWriteHandle::close()
{
	if(file) {
		renewCache();
		fclose(file);
		file = nullptr;
		error_code = FILE_NO_ERROR;
	}
}

void DiskNodeFileWriteHandle::renewCache()
{
	if(cache) {
		fwrite(cache, local_write_index, 1, file);
		if(ferror(file) != 0) {
			error_code = FILE_WRITE_ERROR;
		}
	} else {
		cache = (uint8_t*)malloc(cache_size+1);
	}
	local_write_index = 0;
}

//=============================================================================
// Memory based node file write handle

MemoryNodeFileWriteHandle::MemoryNodeFileWriteHandle()
{
	if(!cache) {
		cache = (uint8_t*)malloc(cache_size+1);
	}
	local_write_index = 0;
}

MemoryNodeFileWriteHandle::~MemoryNodeFileWriteHandle()
{
	close();
}

void MemoryNodeFileWriteHandle::reset()
{
	memset(cache, 0xAA, cache_size);
	local_write_index = 0;
}

void MemoryNodeFileWriteHandle::close()
{
	free(cache);
	cache = nullptr;
}

uint8_t* MemoryNodeFileWriteHandle::getMemory()
{
	return cache;
}

size_t MemoryNodeFileWriteHandle::getSize()
{
	return local_write_index;
}

void MemoryNodeFileWriteHandle::renewCache()
{
	if(cache) {
		cache_size = cache_size * 2;
		cache = (uint8_t*)realloc(cache, cache_size);
		if(!cache) {
			exit(1);
		}
	} else {
		cache = (uint8_t*)malloc(cache_size+1);
	}
}

//=============================================================================
// Node file write handle

NodeFileWriteHandle::NodeFileWriteHandle() :
	cache(nullptr),
	cache_size(0x7FFF),
	local_write_index(0)
{
	////
}

NodeFileWriteHandle::~NodeFileWriteHandle()
{
	free(cache);
}

bool NodeFileWriteHandle::addNode(uint8_t nodetype)
{
	cache[local_write_index++] = NODE_START;
	if(local_write_index >= cache_size) {
		renewCache();
	}

	cache[local_write_index++] = nodetype;
	if(local_write_index >= cache_size) {
		renewCache();
	}

	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::endNode()
{
	cache[local_write_index++] = NODE_END;
	if(local_write_index >= cache_size) {
		renewCache();
	}

	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addU8(uint8_t u8)
{
	writeBytes(&u8, sizeof(u8));
	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addByte(uint8_t u8)
{
	writeBytes(&u8, sizeof(u8));
	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addU16(uint16_t u16)
{
	writeBytes(reinterpret_cast<uint8_t*>(&u16), sizeof(u16));
	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addU32(uint32_t u32)
{
	writeBytes(reinterpret_cast<uint8_t*>(&u32), sizeof(u32));
	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addU64(uint64_t u64)
{
	writeBytes(reinterpret_cast<uint8_t*>(&u64), sizeof(u64));
	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addString(const std::string& str)
{
	if(str.size() > 0xFFFF) {
		error_code = FILE_STRING_TOO_LONG;
		return false;
	}
	addU16(uint16_t(str.size()));
	addRAW((const uint8_t*)str.c_str(), str.size());
	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addLongString(const std::string& str)
{
	addU32(uint32_t(str.size()));
	addRAW((const uint8_t*)str.c_str(), str.size());
	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addRAW(std::string& str)
{
	writeBytes(reinterpret_cast<uint8_t*>(const_cast<char*>(str.data())), str.size());
	return error_code == FILE_NO_ERROR;
}

bool NodeFileWriteHandle::addRAW(const uint8_t* ptr, size_t sz)
{
	writeBytes(ptr, sz);
	return error_code == FILE_NO_ERROR;
}
