#pragma once

#include "../html/document.hpp"
#include "../html/element.hpp"
#include "../core/string_utils.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <iostream>
#include <sstream>

namespace nexus::js {

struct JSValue {
    enum Type { UNDEFINED, NULL_VAL, BOOLEAN, NUMBER, STRING, OBJECT, FUNCTION } type{UNDEFINED};
    bool bool_val{false};
    double num_val{0.0};
    std::string str_val{""};
    std::shared_ptr<html::Element> element_ref{nullptr};

    static JSValue make_undefined() { JSValue v; v.type = UNDEFINED; return v; }
    static JSValue make_null() { JSValue v; v.type = NULL_VAL; return v; }
    static JSValue make_bool(bool b) { JSValue v; v.type = BOOLEAN; v.bool_val = b; return v; }
    static JSValue make_number(double n) { JSValue v; v.type = NUMBER; v.num_val = n; return v; }
    static JSValue make_string(std::string s) { JSValue v; v.type = STRING; v.str_val = std::move(s); return v; }
    static JSValue make_element(std::shared_ptr<html::Element> el) { JSValue v; v.type = OBJECT; v.element_ref = el; return v; }

    std::string to_string() const {
        switch (type) {
            case UNDEFINED: return "undefined";
            case NULL_VAL: return "null";
            case BOOLEAN: return bool_val ? "true" : "false";
            case NUMBER: {
                std::ostringstream ss;
                ss << num_val;
                return ss.str();
            }
            case STRING: return str_val;
            case OBJECT:
                if (element_ref) return "[Element <" + element_ref->get_tag_name() + ">]";
                return "[Object]";
            case FUNCTION: return "[Function]";
            default: return "";
        }
    }
};

class JSEngine {
private:
    std::shared_ptr<html::Document> document_;
    std::unordered_map<std::string, JSValue> global_scope_;
    std::vector<std::string> console_logs_;
    std::unordered_map<std::string, std::function<void()>> event_listeners_;
    bool dom_mutated_{false};

public:
    explicit JSEngine(std::shared_ptr<html::Document> doc) : document_(doc) {
        setup_globals();
    }

    void setup_globals() {
        global_scope_["window"] = JSValue::make_undefined();
        global_scope_["document"] = JSValue::make_undefined();
    }

    const std::vector<std::string>& get_console_logs() const { return console_logs_; }
    bool has_dom_mutated() const { return dom_mutated_; }
    void clear_mutation_flag() { dom_mutated_ = false; }

    void add_event_listener(const std::string& element_id, const std::string& event, std::function<void()> handler) {
        event_listeners_[element_id + ":" + event] = handler;
    }

    void dispatch_click(const std::string& element_id) {
        auto it = event_listeners_.find(element_id + ":click");
        if (it != event_listeners_.end()) {
            console_logs_.push_back("[Event] Click dispatched on #" + element_id);
            it->second();
            dom_mutated_ = true;
        }
    }

    JSValue eval(const std::string& script) {
        std::vector<std::string> lines = core::StringUtils::split(script, ';');
        JSValue last_result = JSValue::make_undefined();

        for (auto& raw_line : lines) {
            std::string line = core::StringUtils::trim(raw_line);
            if (line.empty() || core::StringUtils::starts_with(line, "//")) continue;

            // console.log(...)
            if (core::StringUtils::starts_with(line, "console.log(")) {
                size_t start = line.find('(');
                size_t end = line.rfind(')');
                if (start != std::string::npos && end != std::string::npos && end > start) {
                    std::string arg = core::StringUtils::trim(line.substr(start + 1, end - start - 1));
                    if (!arg.empty() && ((arg.front() == '"' && arg.back() == '"') || (arg.front() == '\'' && arg.back() == '\''))) {
                        std::string log_msg = arg.substr(1, arg.length() - 2);
                        console_logs_.push_back(log_msg);
                    } else if (global_scope_.find(arg) != global_scope_.end()) {
                        console_logs_.push_back(global_scope_[arg].to_string());
                    } else {
                        console_logs_.push_back(arg);
                    }
                }
                continue;
            }

            // document.getElementById("...").innerHTML = "..."
            if (line.find("document.getElementById(") != std::string::npos) {
                size_t id_start = line.find("document.getElementById(\"");
                size_t quote_type = 1; // double quote
                if (id_start == std::string::npos) {
                    id_start = line.find("document.getElementById('");
                    quote_type = 2; // single quote
                }

                if (id_start != std::string::npos) {
                    size_t quote_start = (quote_type == 1) ? id_start + 25 : id_start + 25;
                    char q_char = (quote_type == 1) ? '"' : '\'';
                    size_t quote_end = line.find(q_char, quote_start);

                    if (quote_end != std::string::npos) {
                        std::string id = line.substr(quote_start, quote_end - quote_start);
                        auto elem = document_->get_element_by_id(id);

                        if (elem) {
                            // Check for .textContent or .innerHTML assignment
                            size_t eq_pos = line.find('=', quote_end);
                            if (eq_pos != std::string::npos) {
                                std::string val_part = core::StringUtils::trim(line.substr(eq_pos + 1));
                                if (!val_part.empty() && (val_part.front() == '"' || val_part.front() == '\'')) {
                                    val_part = val_part.substr(1, val_part.length() - 2);
                                }
                                elem->set_text_content(val_part);
                                dom_mutated_ = true;
                                console_logs_.push_back("[DOM Mutation] Updated #" + id + " textContent = \"" + val_part + "\"");
                            }

                            // Check for .style.color = "..."
                            if (line.find(".style.color") != std::string::npos && eq_pos != std::string::npos) {
                                std::string color_val = core::StringUtils::trim(line.substr(eq_pos + 1));
                                if (!color_val.empty() && (color_val.front() == '"' || color_val.front() == '\'')) {
                                    color_val = color_val.substr(1, color_val.length() - 2);
                                }
                                std::string cur_style = elem->get_attribute("style");
                                cur_style += "; color: " + color_val + ";";
                                elem->set_attribute("style", cur_style);
                                dom_mutated_ = true;
                                console_logs_.push_back("[Style Mutation] Updated #" + id + " color = " + color_val);
                            }

                            // Check for .classList.add("...")
                            if (line.find(".classList.add(") != std::string::npos) {
                                size_t c_start = line.find(".classList.add(");
                                size_t c_end = line.find(')', c_start);
                                if (c_start != std::string::npos && c_end != std::string::npos) {
                                    std::string cls = core::StringUtils::trim(line.substr(c_start + 15, c_end - c_start - 15));
                                    if (!cls.empty() && (cls.front() == '"' || cls.front() == '\'')) {
                                        cls = cls.substr(1, cls.length() - 2);
                                    }
                                    elem->add_class(cls);
                                    dom_mutated_ = true;
                                    console_logs_.push_back("[DOM Mutation] Added class '" + cls + "' to #" + id);
                                }
                            }
                        }
                    }
                }
            }

            // Variable assignment `let x = 10`
            if (core::StringUtils::starts_with(line, "let ") ||
                core::StringUtils::starts_with(line, "var ") ||
                core::StringUtils::starts_with(line, "const ")) {
                size_t space_pos = line.find(' ');
                size_t eq_pos = line.find('=', space_pos);
                if (eq_pos != std::string::npos) {
                    std::string var_name = core::StringUtils::trim(line.substr(space_pos + 1, eq_pos - space_pos - 1));
                    std::string var_val = core::StringUtils::trim(line.substr(eq_pos + 1));
                    try {
                        double num = std::stod(var_val);
                        global_scope_[var_name] = JSValue::make_number(num);
                    } catch (...) {
                        global_scope_[var_name] = JSValue::make_string(var_val);
                    }
                }
            }
        }

        return last_result;
    }
};

} // namespace nexus::js
