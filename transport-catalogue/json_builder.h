#pragma once

#include <string>
#include <stack>

#include "json.h"

namespace json {

	class ItemContext;
	class DictItemContext;
	class ArrayItemContext;
	class KeyItemContext;
	class ValueItemContext;

	class Builder {
	public:
		
		Builder();
		KeyItemContext Key(std::string value);
		Builder& Value(json::Node value);

		DictItemContext StartDict();
		Builder& EndDict();

		ArrayItemContext StartArray();
		Builder& EndArray();

		json::Node Build() const;

	private:
		std::stack<Node*> node_stack_;
		json::Node value_;
	}; // builder

	class ItemContext {
	public:
		ItemContext(Builder& builder) :builder_(builder) {}

		KeyItemContext Key(std::string value);
		Builder& Value(json::Node node);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
	protected:
		Builder& builder_;
	};

	class DictItemContext : public ItemContext {
	public:
		DictItemContext(Builder& builder) :ItemContext(builder) {}

		// KeyItemContext& Key(std::string value);
		// Builder& EndDict();
		
		Builder& Value(json::Node node) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};

	class ArrayItemContext :public ItemContext {
	public:
		ArrayItemContext(Builder& builder) :ItemContext(builder) {}

		// Builder& Value(json::Node& node);
		// DictItemContext StartDict();
		// ArrayItemContext StartArray();
		// Builder& EndArray();

		ArrayItemContext Value(json::Node node);

		KeyItemContext Key(std::string value) = delete;
		Builder& EndDict() = delete;
	};

	class KeyItemContext :public ItemContext {
	public:
		KeyItemContext(Builder& builder) :ItemContext(builder) {}
		
		// DictItemContext StartDict();
		// ArrayItemContext StartArray();

		ValueItemContext Value(json::Node node);

		KeyItemContext Key(std::string value) = delete;
		Builder& EndDict() = delete;
		Builder& EndArray() = delete;
	};

	class ValueItemContext :public ItemContext {
	public:
		ValueItemContext(Builder& builder) :ItemContext(builder) {}

		// KeyItemContext& Key(std::string value);
		// Builder& EndDict();

		Builder& Value(json::Node node) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		Builder& EndArray() = delete;
	};
} // ------------ namespace json ----------