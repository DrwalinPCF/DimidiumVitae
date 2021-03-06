
//	This file is a wrapper for RapidJSON and stl
// Copyright (C) 2020 Marek Zalewski aka Drwalin aka DrwalinPCF

#ifndef JSON_WRAPPER_FOR_RAPIDJSON_CPP
#define JSON_WRAPPER_FOR_RAPIDJSON_CPP

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

#include <cmath>

using namespace rapidjson;

#include "JSON.h"

MemoryPoolAllocator<>& __GetJSONPoolAllocator() {
	static MemoryPoolAllocator<> allocator;
	return allocator;
}

CrtAllocator& __GetJSONStackAllocator() {
	static CrtAllocator allocator;
	return allocator;
}


template<>
JSON::Iterator<JSON>::Iterator(JSON &json) : id(0), json(json) {}
template<>
JSON::Iterator<JSON>::Iterator(JSON &json, uint64_t id) : id(id), json(json) {}
template<>
JSON::Iterator<JSON>::~Iterator() {}

template<>
void JSON::Iterator<JSON>::operator++(void) {++id;}
template<>
void JSON::Iterator<JSON>::operator++(int) {id++;}

template<>
bool JSON::Iterator<JSON>::operator==(const Iterator &other) const {return id==other.id && json.ref==other.json.ref;}
template<>
bool JSON::Iterator<JSON>::operator!=(const Iterator &other) const {return id!=other.id || json.ref!=other.json.ref;}

template<>
JSON JSON::Iterator<JSON>::Value() {
	if(json.IsArray()) {
		return JSON(&(*(json.ref->Begin()+id)), REFERENCE);
	} else if(json.IsObject()) {
		return JSON(&((json.ref->MemberBegin()+id)->value), REFERENCE);
	} else {
		throw std::string(std::string("JSON is not an array or object " __FILE__ ":")+std::to_string(__LINE__));
	}
	return JSON();
}
template<>
std::string JSON::Iterator<JSON>::Name() const {
	if(json.IsObject()) {
		return (json.ref->MemberBegin()+id)->name.GetString();
	} else {
		throw std::string(std::string("JSON is not an object " __FILE__ ":")+std::to_string(__LINE__));
	}
	return "";
}
template<>
JSON::Iterator<JSON> JSON::Iterator<JSON>::operator*() {
	return *this;
}











template<>
JSON::ConstIterator<JSON>::ConstIterator(const JSON &json) : id(0), json(json) {}
template<>
JSON::ConstIterator<JSON>::ConstIterator(const JSON &json, uint64_t id) : id(id), json(json) {}
template<>
JSON::ConstIterator<JSON>::~ConstIterator() {}

template<>
void JSON::ConstIterator<JSON>::operator++(void) {++id;}
template<>
void JSON::ConstIterator<JSON>::operator++(int) {id++;}

template<>
bool JSON::ConstIterator<JSON>::operator==(const ConstIterator &other) const {return id==other.id && json.ref==other.json.ref;}
template<>
bool JSON::ConstIterator<JSON>::operator!=(const ConstIterator &other) const {return id!=other.id || json.ref!=other.json.ref;}

template<>
const JSON JSON::ConstIterator<JSON>::Value() const {
	if(json.IsArray()) {
		return JSON(&(*(json.ref->Begin()+id)), REFERENCE);
	} else if(json.IsObject()) {
		return JSON(&((json.ref->MemberBegin()+id)->value), REFERENCE);
	} else {
		throw std::string(std::string("JSON is not an array or object " __FILE__ ":")+std::to_string(__LINE__));
	}
	return JSON();
}
template<>
std::string JSON::ConstIterator<JSON>::Name() const {
	if(json.IsObject()) {
		return (json.ref->MemberBegin()+id)->name.GetString();
	} else {
		throw std::string(std::string("JSON is not an object " __FILE__ ":")+std::to_string(__LINE__));
	}
	return "";
}
template<>
JSON::ConstIterator<JSON> JSON::ConstIterator<JSON>::operator*() {
	return *this;
}












JSON::Iterator<JSON> JSON::begin() {
	return JSON::Iterator<JSON>(*this, 0);
}
JSON::Iterator<JSON> JSON::end() {
	return JSON::Iterator<JSON>(*this, Size());
}
JSON::ConstIterator<JSON> JSON::begin() const {
	return JSON::ConstIterator<JSON>(*this, 0);
}
JSON::ConstIterator<JSON> JSON::end() const {
	return JSON::ConstIterator<JSON>(*this, Size());
}


JSON::JSON() {
	type = NONE;
	ref = NULL;
}

JSON::JSON(const JSON &other) {
	ref = other.ref;
	if(ref)
		type = REFERENCE;
	else
		type = NONE;
}

JSON::JSON(JSON &&other) {
	ref = other.ref;
	type = other.type;
	other.ref = NULL;
	other.type = NONE;
}

JSON::JSON(void *ptr, ReferenceType referenceType) {
	ref = (RAPIDJSON_TYPE*)ptr;
	if(ref)
		type = referenceType;
	else
		type = NONE;
}

JSON::JSON(const char *jsonString) {
	type = PRIMARY;
	Document *document = new Document(kNullType, &(__GetJSONPoolAllocator()), 256, &(__GetJSONStackAllocator()));
	ref = document;
	document->Parse(jsonString);
}

JSON::JSON(const std::string &jsonString) {
	type = PRIMARY;
	Document *document = new Document(kNullType, &(__GetJSONPoolAllocator()), 256, &(__GetJSONStackAllocator()));
	ref = document;
	document->Parse(jsonString.c_str());
}

JSON::JSON(std::istream &input) {
	type = PRIMARY;
	Document *document = new Document(kNullType, &(__GetJSONPoolAllocator()), 256, &(__GetJSONStackAllocator()));
	ref = document;
	BasicIStreamWrapper istream(input);
	document->ParseStream(istream);
}

JSON::~JSON() {
	Destroy();
}

void JSON::Destroy() {
	if(type == PRIMARY)
		if(ref)
			delete ref;
	ref = NULL;
	type = NONE;
}

JSON& JSON::operator=(uint64_t value) {
	SetUint(value);
	return *this;
}
JSON& JSON::operator=(int64_t value) {
	SetInt(value);
	return *this;
}
JSON& JSON::operator=(double value) {
	SetFloat(value);
	return *this;
}
JSON& JSON::operator=(bool value) {
	SetBool(value);
	return *this;
}
JSON& JSON::operator=(const char *value) {
	SetString(value);
	return *this;
}
JSON& JSON::operator=(const std::string &value) {
	SetString(value);
	return *this;
}
JSON& JSON::operator=(const JSON &value) {	// copy value to current pointer, or make PRIMARY pointer
	if(ref == NULL) {
		ref = new RAPIDJSON_TYPE(*value.ref, __GetJSONPoolAllocator());
		type = PRIMARY;
	} else {
		ref->CopyFrom(*value.ref, __GetJSONPoolAllocator());
	}
	return *this;
}
JSON& JSON::operator<<=(const JSON &value) {	// make reference to value.ref
	Destroy();
	if(ref = value.ref)
		type = REFERENCE;
	else
		type = NONE;
	return *this;
}

void JSON::Parse(const char *jsonString) {
	if(ref == NULL) {
		type = PRIMARY;
		ref = new RAPIDJSON_TYPE();
	}
	Document document(kNullType, &(__GetJSONPoolAllocator()), 256, &(__GetJSONStackAllocator()));
	document.Parse(jsonString);
	ref->Swap(document);
}
void JSON::Parse(const std::string &jsonString) {
	Parse(jsonString.c_str());
}

void JSON::Parse(std::istream &input) {
	if(ref == NULL) {
		type = PRIMARY;
		ref = new RAPIDJSON_TYPE();
	}
	Document document(kNullType, &(__GetJSONPoolAllocator()), 256, &(__GetJSONStackAllocator()));
	BasicIStreamWrapper istream(input);
	document.ParseStream(istream);
	ref->Swap(document);
}

std::string JSON::Write() const {
	if(ref) {
		StringBuffer ostream;
		Writer writer(ostream);
		writer.SetMaxDecimalPlaces(3);
		ref->Accept(writer);
		return std::string(ostream.GetString());
	}
	return "";
}

void JSON::Write(std::ostream &output) const {
	if(ref) {
		BasicOStreamWrapper ostream(output);
		Writer writer(ostream);
		writer.SetMaxDecimalPlaces(3);
		ref->Accept(writer);
	}
}
/*
std::string JSON::PrettyWrite() const {
	if(ref) {
		StringBuffer ostream;
		PrettyWriter writer(ostream);
		ref->Accept(writer);
		return std::string(ostream.GetString());
	}
	return "";
}

void JSON::PrettyWrite(std::ostream &output) const {
	if(ref) {
		BasicOStreamWrapper ostream(output);
		PrettyWriter writer(ostream);
		ref->Accept(writer);
	}
}
*/

JSON JSON::operator[](uint64_t id) {
	if(IsArray()) {
		if(ref->Size() <= id) {
			Resize(id+1);
		}
		return JSON(&(ref->operator[](id)), REFERENCE);
	}
	throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
}
const JSON JSON::operator[](uint64_t id) const {
	if(IsArray()) {
		if(ref->Size() <= id) {
			throw std::string(std::string("JSON trying to access const ARRAY index over it's size. " __FILE__ ":")+std::to_string(__LINE__));
		}
		return JSON(&(ref->operator[](id)), REFERENCE);
	}
	throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
}
void JSON::PushBack(const JSON &json) {
	if(IsArray()) {
		if(ref && json.ref) {
			ref->PushBack(*json.ref, __GetJSONPoolAllocator());
		} else {
			ref->PushBack(RAPIDJSON_TYPE(kNullType), __GetJSONPoolAllocator());
		}
	} else {
		throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
	}
}
void JSON::PopBack() {
	if(IsArray()) {
		ref->PopBack();
	} else {
		throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
	}
}
JSON JSON::Back() {
	if(IsArray()) {
		if(ref->Size()>0) {
			return JSON(&ref->operator[](ref->Size()-1), REFERENCE);
		}
	} else {
		throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
	}
	return JSON();
}
const JSON JSON::Back() const {
	if(IsArray()) {
		if(ref->Size()>0) {
			return JSON(&ref->operator[](ref->Size()-1), REFERENCE);
		}
	} else {
		throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
	}
	return JSON();
}
JSON JSON::Front() {
	if(IsArray()) {
		if(ref->Size()>0) {
			return JSON(&ref->operator[](0), REFERENCE);
		}
	} else {
		throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
	}
	return JSON();
}
const JSON JSON::Front() const {
	if(IsArray()) {
		if(ref->Size()>0) {
			return JSON(&ref->operator[](0), REFERENCE);
		}
	} else {
		throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
	}
	return JSON();
}
void JSON::Resize(uint64_t size) {
	if(IsArray()) {
		ref->Reserve(size, __GetJSONPoolAllocator());
		while(ref->Size() < size) {
			ref->PushBack(RAPIDJSON_TYPE(kNullType), __GetJSONPoolAllocator());
		}
		while(ref->Size() > size) {
			ref->PopBack();
		}
	} else {
		throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
	}
}
void JSON::Erase(uint64_t id) {
	if(IsArray()) {
		
	} else {
		throw std::string(std::string("JSON is not an array " __FILE__ ":")+std::to_string(__LINE__));
	}
}
void JSON::InitArray() {
	if(ref == NULL) {
		ref = new RAPIDJSON_TYPE();
		type = PRIMARY;
	}
	if(ref->IsArray()) {
		ref->Clear();
	} else {
		ref->SetArray();
	}
	ref->Reserve(32, __GetJSONPoolAllocator());
}

uint64_t JSON::Size() const {
	if(IsArray()) {
		return ref->Size();
	} else if(IsObject()) {
		return ref->MemberCount();
	} else {
		throw std::string(std::string("JSON is not an array or object " __FILE__ ":")+std::to_string(__LINE__));
	}
}
void JSON::Clear() {
	if(IsArray()) {
		return ref->Clear();
	} else if(IsObject()) {
		return ref->RemoveAllMembers();
	} else {
		throw std::string(std::string("JSON is not an array or object " __FILE__ ":")+std::to_string(__LINE__));
	}
}

JSON JSON::operator[](const char *name) {
	if(IsObject()) {
		if(!ref->HasMember(name)) {
			ref->AddMember(RAPIDJSON_TYPE(name, __GetJSONPoolAllocator()), RAPIDJSON_TYPE(kNullType), __GetJSONPoolAllocator());
		}
		JSON ret(&((*ref)[name]), REFERENCE);
		return ret;
	}
	throw std::string(std::string("JSON is not an object " __FILE__ ":")+std::to_string(__LINE__));
}
JSON JSON::operator[](const std::string &name) {
	return this->operator[](name.c_str());
}
const JSON JSON::operator[](const char *name) const {
	if(IsObject()) {
		if(!ref->HasMember(name)) {
			throw std::string(std::string("JSON trying to access const OBJECT key which does not exist. " __FILE__ ":")+std::to_string(__LINE__));
		}
		JSON ret(&((*ref)[name]), REFERENCE);
		return ret;
	}
	throw std::string(std::string("JSON is not an object " __FILE__ ":")+std::to_string(__LINE__));
}
const JSON JSON::operator[](const std::string &name) const {
	return this->operator[](name.c_str());
}
bool JSON::HasKey(const char *name) const {
	if(IsObject()) {
		return ref->HasMember(name);
	}
	throw std::string(std::string("JSON is not an object " __FILE__ ":")+std::to_string(__LINE__));
}
bool JSON::HasKey(const std::string &name) const {
	return HasKey(name.c_str());
}
void JSON::Erase(const char *name) {
	if(IsObject()) {
		ref->RemoveMember(name);
	} else {
		throw std::string(std::string("JSON is not an object " __FILE__ ":")+std::to_string(__LINE__));
	}
}
void JSON::Erase(const std::string &name) {
	Erase(name.c_str());
}
void JSON::InitObject() {
	if(ref == NULL) {
		ref = new RAPIDJSON_TYPE();
		type = PRIMARY;
	}
	if(ref->IsObject()) {
		ref->RemoveAllMembers();
	} else {
		ref->SetObject();
	}
}




JSON::operator int64_t() const {
	return GetInt();
}
JSON::operator uint64_t() const {
	return GetUint();
}
JSON::operator int32_t() const {
	return GetInt();
}
JSON::operator uint32_t() const {
	return GetUint();
}
JSON::operator int16_t() const {
	return GetInt();
}
JSON::operator uint16_t() const {
	return GetUint();
}
JSON::operator float() const {
	return GetFloat();
}
JSON::operator double() const {
	return GetFloat();
}
JSON::operator std::string() const {
	return GetString();
}
JSON::operator bool() const {
	return GetBool();
}

int64_t JSON::GetInt() const {
	if(IsNumber()) {
		if(!ref->IsInt64())
			return (int64_t)ref->GetFloat();
		else
			return ref->GetInt64();
	} else {
		throw std::string(std::string("JSON is not a boolean " __FILE__ ":")+std::to_string(__LINE__));
	}
}
uint64_t JSON::GetUint() const {
	if(IsNumber()) {
		if(!ref->IsUint64())
			return (uint64_t)ref->GetFloat();
		else
			return ref->GetInt64();
	} else {
		throw std::string(std::string("JSON is not a boolean " __FILE__ ":")+std::to_string(__LINE__));
	}
}
double JSON::GetFloat() const {
	if(IsNumber()) {
		return ref->GetFloat();
	} else {
		throw std::string(std::string("JSON is not a boolean " __FILE__ ":")+std::to_string(__LINE__));
	}
}
std::string JSON::GetString() const {
	if(IsString()) {
		return ref->GetString();
	} else {
		throw std::string(std::string("JSON is not a boolean " __FILE__ ":")+std::to_string(__LINE__));
	}
}
bool JSON::GetBool() const {
	if(IsBool()) {
		return IsTrue();
	} else {
		throw std::string(std::string("JSON is not a boolean " __FILE__ ":")+std::to_string(__LINE__));
	}
}

void JSON::SetInt(int64_t value) {
	if(!ref) {
		type = PRIMARY;
		ref = new RAPIDJSON_TYPE(kNullType);
	}
	ref->SetInt64(value);
}
void JSON::SetUint(uint64_t value) {
	if(!ref) {
		type = PRIMARY;
		ref = new RAPIDJSON_TYPE(kNullType);
	}
	ref->SetUint64(value);
}
void JSON::SetFloat(double value) {
	if(!ref) {
		type = PRIMARY;
		ref = new RAPIDJSON_TYPE(kNullType);
	}
	const double quant = 0.001;
	const double maxval = 1e21;
	if(value < -maxval)
		value = -maxval;
	else if(value > maxval)
		value = maxval;
	else if(fabs(value) < quant)
		value = 0.0;
	else {
		value = round(value/quant)*quant;
	}
	
	if(fabs(round(value)-value)<quant && fabs(value)<1000000)
		ref->SetInt64(round(value));
	else
		ref->SetDouble(value);
}
void JSON::SetString(const char *value) {
	if(!ref) {
		type = PRIMARY;
		ref = new RAPIDJSON_TYPE(kNullType);
	}
	ref->SetString(value, __GetJSONPoolAllocator());
}
void JSON::SetString(const std::string& value) {
	if(!ref) {
		type = PRIMARY;
		ref = new RAPIDJSON_TYPE(kNullType);
	}
	ref->SetString(value.c_str(), value.length(), __GetJSONPoolAllocator());
}
void JSON::SetBool(bool value) {
	if(!ref) {
		type = PRIMARY;
		ref = new RAPIDJSON_TYPE(kNullType);
	}
	ref->SetBool(value);
}

bool JSON::IsBool() const {
	return IsFalse() || IsTrue();
}
bool JSON::IsNumber() const {
	if(ref)
		return ref->IsNumber();
	return false;
}
bool JSON::IsInt() const {
	if(ref)
		return ref->IsInt64();
	return false;
}
bool JSON::IsUint() const {
	if(ref)
		return ref->IsUint64();
	return false;
}
bool JSON::IsFloat() const {
	if(ref) {
		return ref->IsDouble();
	}
	return false;
}
bool JSON::IsString() const {
	if(ref)
		return ref->IsString();
	return false;
}
bool JSON::IsTrue() const {
	if(ref)
		return ref->IsTrue();
	return false;
}
bool JSON::IsFalse() const {
	if(ref)
		return ref->IsFalse();
	return false;
}
bool JSON::IsNull() const {
	if(ref)
		return ref->IsNull();
	return true;
}
bool JSON::IsArray() const {
	if(ref)
		return ref->IsArray();
	return false;
}
bool JSON::IsObject() const {
	if(ref)
		return ref->IsObject();
	return false;
}

std::ostream& operator<<(std::ostream &output, const JSON &json) {
	json.Write(output);
	return output;
}

std::istream& operator>>(std::istream &input, JSON &json) {
	json.Parse(input);
	return input;
}

#endif

