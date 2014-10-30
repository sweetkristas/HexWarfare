/*
   Copyright 2014 Kristina Simpson <sweet.kristas@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>

#include <deque>
#include <cstdint>

#include "filesystem.hpp"
#include "formatter.hpp"
#include "json.hpp"
#include "unit_test.hpp"

namespace json
{
	bool is_space(int c)
	{
		if(c == ' ' || c == '\t' || c == '\v' || c == '\r' || c == '\n' || c == '\f') {
			return true;
		}
		return false;
	}

	bool is_digit(int c)
	{
		if(c == '0' || c == '1' || c == '2' || c == '3' 
			|| c == '4' || c == '5' || c == '6' || c == '7' 
			|| c == '8' || c == '9') {
			return true;
		}
		return false;
	}

	class lexer
	{
	public:
		enum json_token
		{
			LEFT_BRACE,
			RIGHT_BRACE,
			LEFT_BRACKET,
			RIGHT_BRACKET,
			COLON,
			COMMA,
			STRING_LITERAL,
			INTEGER,
			FLOAT,
			LIT_TRUE,
			LIT_FALSE,
			LIT_NULL,
			LITERAL,
			DOCUMENT_END,
		};

		lexer(const std::string& s)
			: lex_str_(s)
		{
			it_ = lex_str_.begin();
		}

		static bool is_simple_value(json_token tok)
		{
			if(tok == STRING_LITERAL || tok == INTEGER || tok == FLOAT
				|| tok == LIT_TRUE || tok == LIT_FALSE || tok == LIT_NULL) {
				return true;
			}
			return false;
		}

		static std::string token_as_string(json_token tok)
		{
			switch(tok) {
			case LEFT_BRACE:
				return "LEFT BRACE";
			case RIGHT_BRACE:
				return "RIGHT BRACE";
			case LEFT_BRACKET:
				return "LEFT BRACKET";
			case RIGHT_BRACKET:
				return "RIGHT BRACKET";
			case COLON:
				return "COLON";
			case COMMA:
				return "COMMA";
			case STRING_LITERAL:
				return "STRING LITERAL";
			case INTEGER:
				return "INTEGER";
			case FLOAT:
				return "FLOAT";
			case LIT_TRUE:
				return "LITERAL TRUE";
			case LIT_FALSE:
				return "LITERAL FALSE";
			case LIT_NULL:
				return "LITERAL NULL";
			case LITERAL:
				return "LITERAL";
			case DOCUMENT_END:
				return "DOCUMENT END";
			default:
				throw parse_error(formatter() << "unknown token type in token_as_string: " << tok);
			}
			return "";
		}
		
		uint16_t decode_hex_nibble(char c)
		{
			if(c >= '0' && c <= '9') {
				return c - '0';
			} else if(c >= 'a' && c <= 'f') {
				return c - 'a' + 10;
			} else if(c >= 'A' && c <= 'F') {
				return c - 'A' + 10;
			}
			throw parse_error(formatter() << "Invalid character in decode: " << c);
		}

		void push_back(json_token tok, node value)
		{
			pushed_back_tokens.push_back(std::make_pair(tok, value));
		}
		
		boost::tuple<json_token, node> get_next_token()
		{
			if(pushed_back_tokens.empty() == false) {
				std::pair<json_token, node> pr = pushed_back_tokens.front();
				pushed_back_tokens.pop_front();
				return boost::make_tuple(pr.first, pr.second);
			}

			bool running = true;
			bool in_string = false;
			std::string::iterator start_it = it_;
			std::string new_string;
			while(running) {
				if(it_ == lex_str_.end()) {
					if(in_string) {
						throw parse_error(formatter() << "End of data inside string");
					}
					return boost::make_tuple(DOCUMENT_END, node());
				}
				if(in_string) {
					if(*it_ == '"') {
						++it_;
						node string_node(new_string);
						//if(new_string == "true") {
						//	return boost::make_tuple(LIT_TRUE, node::from_bool(true));
						//} else if(new_string == "false") {
						//	return boost::make_tuple(LIT_FALSE, node::from_bool(false));
						//} else if(new_string == "null") {
						//	return boost::make_tuple(LIT_NULL, node());
						//}
						return boost::make_tuple(STRING_LITERAL, string_node);
					} else if(*it_ == '\\') {
						++it_;
						if(it_ == lex_str_.end()) {
							throw parse_error(formatter() << "End of data in quoted token");
						}
						if(*it_ == '"') {
							new_string += '"';
							++it_;
						} else if(*it_ == '\\') {
							new_string += '\\';
							++it_;
						} else if(*it_ == '/') {
							new_string += '/';
							++it_;
						} else if(*it_ == 'b') {
							new_string += '\b';
							++it_;
						} else if(*it_ == 'f') {
							new_string += '\f';
							++it_;
						} else if(*it_ == 'n') {
							new_string += '\n';
							++it_;
						} else if(*it_ == 'r') {
							new_string += '\r';
							++it_;
						} else if(*it_ == 't') {
							new_string += '\t';
							++it_;
						} else if(*it_ == 'u') {
							++it_;
							if((lex_str_.end() - it_) >= 4) {
								uint16_t value = 0;
								for(int n = 0; n != 4; ++n) {
									value = (value << 4) | decode_hex_nibble(*it_++);
								}
								// Quick and dirty conversion from \uXXXX -> UTF-8
								if(value >= 0 && value <= 127U) {
									new_string += char(value);
								} else if(value >= 128U && value <= 2047U) {
									new_string += char(0xc0 | (value >> 6));
									new_string += char(0x80 | (value & 0x3f));
								} else if(value >= 2048U) {
									new_string += char(0xe0 | (value >> 12));
									new_string += char(0x80 | ((value >> 6) & 0x3f));
									new_string += char(0x80 | (value & 0x3f));
								}
							} else {
								throw parse_error(formatter() << "Expected 4 hexadecimal characters after \\u token");
							}
						} else {
							throw parse_error(formatter() << "Unrecognised quoted token: " << *it_);
						}
					} else {
						new_string += *it_++;
					}
				} else {
					if(*it_ == '{' || *it_ == '}' || *it_ == '[' || *it_ == ']' || *it_ == ',' || *it_ == ':' || *it_ == '"' || is_space(*it_)) {
						if(new_string.empty() == false) {
							if(new_string == "true") {
								return boost::make_tuple(LIT_TRUE, node::from_bool(true));
							} else if(new_string == "false") {
								return boost::make_tuple(LIT_FALSE, node::from_bool(false));
							} else if(new_string == "null") {
								return boost::make_tuple(LIT_NULL, node());
							} else {
								return boost::make_tuple(LITERAL, node(new_string));
							}
						}
					}
					if(*it_ == '{') {
						++it_;
						return boost::make_tuple(LEFT_BRACE, node());
					} else if(*it_ == '}') {
						++it_;
						return boost::make_tuple(RIGHT_BRACE, node());
					} else if(*it_ == '[') {
						++it_;
						return boost::make_tuple(LEFT_BRACKET, node());
					} else if(*it_ == ']') {
						++it_;
						return boost::make_tuple(RIGHT_BRACKET, node());
					} else if(*it_ == ',') {
						++it_;
						return boost::make_tuple(COMMA, node());
					} else if(*it_ == ':') {
						++it_;
						return boost::make_tuple(COLON, node());
					} else if(*it_ == '/' && *(it_+1) == '/') {
						// is single line comment
						while(*it_ != '\n') {
							++it_;
						}
					} else if(*it_ == '"') {
						in_string = true;
						++it_;
					} else if(is_digit(*it_) || *it_ == '-') {
						bool is_float = false;
						std::string num;
						if(*it_ == '-') {
							num += *it_;
							++it_;
						}
						while(is_digit(*it_)) {
							num += *it_;
							++it_;
						}
						if(*it_ == '.') {
							num += *it_;
							++it_;
							while(is_digit(*it_)) {
								num += *it_;
								++it_;
							}
							is_float = true;
						} 
						if(*it_ == 'e' || *it_ == 'E') {
							is_float = true;
							num += *it_;
							++it_;
							if(*it_ == '+') {
								num += *it_;
								++it_;
							} else if(*it_ == '-') {
								num += *it_;
								++it_;
							}
							while(is_digit(*it_)) {
								num += *it_;
								++it_;
							}
						}
						if(is_float) {
							//std::cerr << "Convert \"" << num << "\" to float" << std::endl;
							try {
								return boost::make_tuple(FLOAT, node(boost::lexical_cast<float>(num)));
							} catch (boost::bad_lexical_cast&) {
								throw parse_error(formatter() << "error converting value to float: " << num);
							}
						} else {
							//std::cerr << "Convert \"" << num << "\" to int" << std::endl;
							try {
								return boost::make_tuple(INTEGER, node(boost::lexical_cast<int64_t>(num)));
							} catch (boost::bad_lexical_cast&) {
								throw parse_error(formatter() << "error converting value to integer: " << num);
							}
						}
					} else if(is_space(*it_)) {
						++it_;
					} else {
						new_string += *it_;
						++it_;
					}
				}
			}
			return boost::make_tuple(DOCUMENT_END, node());
		}

	private:
		std::string lex_str_;
		std::string::iterator it_;
		std::deque<std::pair<json_token, node> > pushed_back_tokens;
	};

	namespace 
	{
		node read_array(lexer& lex);
		node read_object(lexer& lex);

		node read_array(lexer& lex)
		{
			std::vector<node> res;
			bool running = true;
			while(running) {
				lexer::json_token tok;
				node token_value;
				boost::tie(tok, token_value) = lex.get_next_token();
				if(lexer::is_simple_value(tok)) {
					res.push_back(token_value);				
				} else if(tok == lexer::LEFT_BRACE) {
					res.push_back(read_object(lex));
				} else if(tok == lexer::LEFT_BRACKET) {
					res.push_back(read_array(lex));
				} else if(tok == lexer::RIGHT_BRACKET) {
					return node(&res);
				} else {
					throw parse_error(formatter() << "Expected colon ':' found " << lexer::token_as_string(tok));
				}
				boost::tie(tok, token_value) = lex.get_next_token();
				if(tok == lexer::RIGHT_BRACKET) {
					running = false;
				} else if(tok == lexer::COMMA) {
					boost::tie(tok, token_value) = lex.get_next_token();
					if(tok == lexer::RIGHT_BRACKET) {
						running = false;
					} else {
						lex.push_back(tok, token_value);
					}
				}
			}
			return node(&res);
		}

		node read_object(lexer& lex)
		{
			std::map<node, node> res;
			bool running = true;
			while(running) {
				lexer::json_token tok;
				node token_value;
				boost::tie(tok, token_value) = lex.get_next_token();
				node key;
				if(tok == lexer::RIGHT_BRACE) {
					// empty map
					return node(&res);
				} else if(tok == lexer::LITERAL || tok == lexer::STRING_LITERAL) {
					key = token_value;
				} else {
					throw parse_error(formatter() << "Unexpected token type: " << lexer::token_as_string(tok) << " expected string or literal");
				}
				boost::tie(tok, token_value) = lex.get_next_token();
				if(tok != lexer::COLON) {
					throw parse_error(formatter() << "Expected colon ':' found " << lexer::token_as_string(tok));
				}
				boost::tie(tok, token_value) = lex.get_next_token();
				if(lexer::is_simple_value(tok)) {
					res[key] = token_value;
				} else if(tok == lexer::LEFT_BRACE) {
					res[key] = read_object(lex);
				} else if(tok == lexer::LEFT_BRACKET) {
					res[key] = read_array(lex);
				} else {
					throw parse_error(formatter() << "Expected colon ':' found " << lexer::token_as_string(tok));
				}
				boost::tie(tok, token_value) = lex.get_next_token();
				if(tok == lexer::RIGHT_BRACE) {
					running = false;
				} else if(tok == lexer::COMMA) {
					boost::tie(tok, token_value) = lex.get_next_token();
					if(tok == lexer::RIGHT_BRACE) {
						running = false;
					} else {
						lex.push_back(tok, token_value);
					}
				}
			}
			return node(&res);
		}
	}

	node parse(const std::string& s)
	{
		lexer lex(s);
		lexer::json_token tok;
		node token_value;
		boost::tie(tok, token_value) = lex.get_next_token();
		if(tok == lexer::LEFT_BRACE) {
			return read_object(lex);
		} else if(tok == lexer::LEFT_BRACKET) {
			return read_array(lex);
		} else {
			throw parse_error(formatter() << "Expecting array or object, found " << lexer::token_as_string(tok));
		}
	}

	node parse_from_file(const std::string& fname)
	{
		if(sys::file_exists(fname)) {
			return parse(sys::read_file(fname));
		} else {
			throw parse_error(formatter() << "File \"" <<  fname << "\" doesn't exist");
		}
	}
}

UNIT_TEST(json_parse_test)
{
	node n = json::parse("{\n"
		"literal: \"a\",\n"
		"\"simple\": \"test\",\n"
		"\"012\": 1234,\n"
		"\"float\": 1e-0,\n"
		"\"int\": 999999999999,\n"
		"\"true\": true,\n"
		"// comment 1\n"
		"\"false\": false, // comment 2\n"
		"\"null\": null,\n"
		"\"list\": [1,2,3,4],\n"
		"\"map\": {\"a\":\"a\", b: \"b\", c:\"c\"},\n"
		"}");
	CHECK_EQ(n["literal"].as_string(), "a");
	CHECK_EQ(n["simple"].as_string(), "test");
	CHECK_EQ(n["012"].as_int(), 1234);
	CHECK_EQ(n["float"].as_float(), 1.0f);
	CHECK_EQ(n["int"].as_int(), 999999999999);
	CHECK_EQ(n["true"].as_bool(), true);
	CHECK_EQ(n["false"].as_bool(), false);
	CHECK_EQ(n["list"][0].as_int(), 1);
	CHECK_EQ(n["list"][1].as_int(), 2);
	CHECK_EQ(n["list"][2].as_int(), 3);
	CHECK_EQ(n["list"][3].as_int(), 4);
	CHECK_EQ(n["map"]["a"].as_string(), "a");
	CHECK_EQ(n["map"]["b"].as_string(), "b");
	CHECK_EQ(n["map"]["c"].as_string(), "c");

	CHECK_EQ(json::parse("[1,2,3,4]"), json::parse("[1,\n2,\n3,\n4]"));
	CHECK_EQ(json::parse("{\"a\":\"a\", b: \"b\", c:\"c\"}"), json::parse("{\"a\":\"a\", b:\"b\",c:\"c\"}"));
}
