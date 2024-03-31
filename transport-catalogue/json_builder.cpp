#include "json_builder.h"

namespace json {

	// Builder

	Builder::Builder() {
		node_stack_.push(&value_);
	}

	KeyItemContext Builder::Key(std::string value) {
		if (node_stack_.empty()) {
			throw std::logic_error("Incorrect place for Value.");
		}
		if (node_stack_.top()->IsDict()) {
			node_stack_.push(&const_cast<Dict&>(node_stack_.top()->AsDict())[value]);
			return KeyItemContext{ *this };
		}
		throw std::logic_error("Incorrect place for Key.");
	}

	Builder& Builder::Value(json::Node value) {
		if (node_stack_.empty()) {
			throw std::logic_error("Incorrect place for Value.");
		}
		if (node_stack_.top()->IsNull()) {
			*node_stack_.top() = value;
			node_stack_.pop();
			return *this;
		}
		else if (node_stack_.top()->IsArray()) {
			const_cast<Array&>(node_stack_.top()->AsArray()).push_back(value);
			return *this;
		}
		throw std::logic_error("Incorrect place for Value.");
	}

	DictItemContext Builder::StartDict() {
		if (node_stack_.empty()) {
			throw std::logic_error("Incorrect place for Value.");
		}
		if (node_stack_.top()->IsNull()) {
			*node_stack_.top() = Dict();
			return DictItemContext{ *this };
		}
		else if (node_stack_.top()->IsArray()) {
			const_cast<Array&>(node_stack_.top()->AsArray()).emplace_back(Dict());
			node_stack_.push(&const_cast<Array&>(node_stack_.top()->AsArray()).back());
			return DictItemContext{ *this };
		}
		throw std::logic_error("Incorrect place for Dict.");
	}

	Builder& Builder::EndDict() {
		if (node_stack_.empty()) {
			throw std::logic_error("Incorrect place for Value.");
		}
		if (!node_stack_.top()->IsDict()) {
			throw std::logic_error("The Map object has not yet been created or there are unfinished Nodes.");
		}
		node_stack_.pop();
		return *this;
	}

	ArrayItemContext Builder::StartArray() {
		if (node_stack_.empty()) {
			throw std::logic_error("Incorrect place for Value.");
		}
		if (node_stack_.top()->IsNull()) {
			*node_stack_.top() = Array();
			return ArrayItemContext{ *this };
		}
		else if (node_stack_.top()->IsArray()) {
			const_cast<Array&>(node_stack_.top()->AsArray()).emplace_back(Array());
			node_stack_.push(&const_cast<Array&>(node_stack_.top()->AsArray()).back());
			return ArrayItemContext{ *this };
		}
		throw std::logic_error("Incorrect place for Array.");
	}

	Builder& Builder::EndArray() {
		if (node_stack_.empty()) {
			throw std::logic_error("Incorrect place for Value.");
		}
		if (!node_stack_.top()->IsArray()) {
			throw std::logic_error("The Array object has not yet been created or there are unfinished Nodes.");
		}
		node_stack_.pop();
		return *this;
	}

	json::Node Builder::Build() const {
		if (!node_stack_.empty()) {
			throw std::logic_error("Some nodes are unfinished.");
		}
		return value_;
	}

	// ItemContext


	KeyItemContext ItemContext::Key(std::string value) {
		return builder_.Key(value);
	}

	Builder& ItemContext::Value(json::Node node) {
		return builder_.Value(node);
	}

	DictItemContext ItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ItemContext::EndDict() {
		return builder_.EndDict();
	}

	Builder& ItemContext::EndArray() {
		return builder_.EndArray();
	}

	// KeyItemContext

	ValueItemContext KeyItemContext::Value(json::Node node) {
		return ValueItemContext{ builder_.Value(node) };
	}

	// ArrayItemContext

	ArrayItemContext ArrayItemContext::Value(json::Node node) {
		return ArrayItemContext{ builder_.Value(node) };
	}

} // namespace json